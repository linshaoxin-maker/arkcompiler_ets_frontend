/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "util/importPathResolver.h"
#include "util/error_handler.h"

#if defined PANDA_TARGET_MOBILE
#define USE_UNIX_SYSCALL
#endif

#ifdef USE_UNIX_SYSCALL
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#else
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#endif

namespace panda::es2panda::util {

std::string ImportPathResolver::ResolveFullPathFromRelative(const std::string &path, const parser::Program *program)
{
    const auto resolvedFp = program->ResolvedFilePath().Mutf8();
    const auto sourceFp = program->SourceFileFolder().Mutf8();
    if (resolvedFp.empty()) {
        auto fp = JoinPaths(sourceFp, path);
        return util::Helpers::IsRealPath(fp) ? fp : path;
    }
    auto fp = JoinPaths(resolvedFp, path);
    if (util::Helpers::IsRealPath(fp)) {
        return fp;
    }
    if (path.find(sourceFp) == 0) {
        return JoinPaths(resolvedFp, path.substr(sourceFp.size()));
    }
    return path;
}

ImportPathResolver::ResultPath ImportPathResolver::GetImportPath(const PathT &path,
                                                                 const parser::Program *program) const
{
    ResultPath resolvedPath;
    if (util::Helpers::IsRelativePath(path)) {
        resolvedPath = ResultPath::Ok(util::Helpers::GetAbsPath(ResolveFullPathFromRelative(path, program)));
    } else if (path.find(PATH_DELIM) == 0) {
        resolvedPath = ResultPath::Ok(config_->BaseUrl());
        resolvedPath->append(path, 0, path.length());
    } else {
        auto &dynamicPaths = config_->DynamicPaths();
        auto it = dynamicPaths.find(path);
        if (it != dynamicPaths.cend() && !it->second.HasDecl()) {
            resolvedPath = ResultPath::Ok(path);
        } else {
            resolvedPath = ResolveRootPath(path);
        }
    }
    return resolvedPath;
}

ImportPathResolver::ResultPath ImportPathResolver::ResolveImportPath(const PathT &path, const parser::Program *program)
{
    auto importPath = GetImportPath(path, program);
    if (importPath) {
        AddResolved(path, *importPath);
    }
    return importPath;
}

std::pair<ImportPathResolver::ResultPath, bool> ImportPathResolver::GetSourceRegularPath(
    const PathT &path, const PathT &resolvedPath) const
{
    if (!panda::os::file::File::IsRegularFile(resolvedPath)) {
        for (const auto &ext : allowedExts_) {
            if (panda::os::file::File::IsRegularFile(resolvedPath + ext)) {
                return std::make_pair(ResultPath::Ok(path + ext), true);
            }
        }
        return std::make_pair(ResultPath::Error(ErrorCode::INCORRECT_PATH, resolvedPath), false);
    }
    return std::make_pair(ResultPath::Ok(path), false);
}

ImportPathResolver::ImportData ImportPathResolver::GetImportData(const PathT &path) const
{
    auto &dynamicPaths = config_->DynamicPaths();
    auto key = panda::os::NormalizePath(path);

    auto it = dynamicPaths.find(key);
    if (it == dynamicPaths.cend()) {
        key = panda::os::RemoveExtension(key);
    }

    while (it == dynamicPaths.cend() && !key.empty()) {
        it = dynamicPaths.find(key);
        if (it != dynamicPaths.cend()) {
            break;
        }
        key = panda::os::GetParentDir(key);
    }

    if (it != dynamicPaths.cend()) {
        return {it->second.GetLanguage(), key, it->second.HasDecl()};
    }
    return {ToLanguage(extension_), path, true};
}

ImportPathResolver::ResultSources ImportPathResolver::CollectUserSources(const PathT &path,
                                                                         const parser::Program *program)
{
    const auto maybeResolvedPath = ResolveImportPath(path, program);
    if (!maybeResolvedPath) {
        return ResultSources::Error(maybeResolvedPath);
    }
    const auto &resolvedPath = *maybeResolvedPath;
    const auto data = GetImportData(resolvedPath);
    if (!data.hasDecl) {
        return ResultSources::Ok(std::vector<PathT> {}, false);
    }

    if (!panda::os::file::File::IsDirectory(resolvedPath)) {
        auto [regular_path, is_module] = GetSourceRegularPath(path, resolvedPath);
        return std::move(regular_path).Transform<UserSources>([module = is_module](PathT &&resPath) {
            return UserSources {std::vector<PathT> {resPath}, module};
        });
    }

    auto newPaths = CollectUserSourcesFromIndex(path, resolvedPath);
    return std::move(newPaths).Transform<UserSources>([](std::vector<PathT> &&paths) {
        return UserSources {std::move(paths), false};
    });
}

ImportPathResolver::ResultT<std::vector<std::string>> ImportPathResolver::CollectUserSourcesFromIndex(
    const std::string &path, const std::string &resolvedPath)
{
    std::vector<PathT> userPaths;

    auto files = ListFiles(resolvedPath);
    if (!files) {
        return files.ExpectError();
    }
    for (const auto &filename : files.ExpectOk()) {
        std::string filePath = JoinPaths(path, filename);

        if (IsIndexFile(filename)) {
            userPaths.clear();
            userPaths.emplace_back(filePath);
            break;
        }
        if (filename == "Object.ets") {
            userPaths.emplace(userPaths.begin(), filePath);
        } else {
            userPaths.emplace_back(filePath);
        }
    }
    return ResultT<std::vector<PathT>>::Ok(userPaths);
}

ImportPathResolver::ResultT<std::vector<ImportPathResolver::PathT>> ImportPathResolver::CollectDefaultSources(
    const std::vector<PathT> &stdlib, const parser::Program *program)
{
    std::vector<PathT> paths;
    std::vector<PathT> defaultSources;

    for (auto const &path : stdlib) {
        const auto maybeResolvedPath = ResolveImportPath(path, program);
        if (!maybeResolvedPath) {
            return maybeResolvedPath.ExpectError();
        }
        auto files = ListFiles(maybeResolvedPath.ExpectOk());
        if (!files) {
            return files.ExpectError();
        }
        for (const auto &filename : files.ExpectOk()) {
            const PathT filePath = JoinPaths(path, filename);
            if (filename == "Object.ets") {
                defaultSources.emplace(defaultSources.begin(), filePath);
            } else {
                defaultSources.emplace_back(filePath);
            }
        }
    }
    return ResultT<std::vector<PathT>>::Ok(defaultSources);
}

ImportPathResolver::ResultT<std::vector<ImportPathResolver::SourceInfo>> ImportPathResolver::ParseSources(
    const std::vector<PathT> &paths, const parser::Program *program)
{
    std::vector<SourceInfo> sources;

    for (const auto &path : paths) {
        auto resolvedPath = ResolveImportPath(path, program);
        if (!resolvedPath) {
            return resolvedPath.ExpectError();
        }

        const auto data = GetImportData(resolvedPath.ExpectOk());
        if (!data.hasDecl) {
            continue;
        }
        sources.emplace_back(SourceInfo {path, resolvedPath.ExpectOk(), false, data.lang});
    }
    return ResultT<std::vector<SourceInfo>>::Ok(sources);
}

void ImportPathResolver::FilterParsedSources(std::vector<std::string> &sources, const parser::Program *program)
{
    auto condition = [this, program](auto x) {
        auto resolved = GetImportPath(x, program).ExpectOk();
        auto found = std::count_if(resolvedParsedSources_.begin(), resolvedParsedSources_.end(),
                                   [resolved](const auto &p) { return p.second == resolved; });
        if (found != 0) {
            AddResolved(x, resolved);
        }
        return found;
    };

    // erase_if - c++20 ext
    for (auto it = sources.begin(); it != sources.end();) {
        if (condition(*it) != 0) {
            it = sources.erase(it);
        } else {
            ++it;
        }
    }
}

ImportPathResolver::ResultPath ImportPathResolver::ResolveRootPath(const ImportPathResolver::PathT &path) const
{
    std::string::size_type pos = path.find(PATH_DELIM);
    bool containsDelim = (pos != std::string::npos);
    std::string rootPart = containsDelim ? path.substr(0, pos) : path;

    static const std::unordered_set<std::string> STDLIB_PARTS = {"std", "escompat"};

    PathT resolvedPath;
    if (STDLIB_PARTS.count(rootPart) != 0 && !stdlibPath_.empty()) {
        resolvedPath = JoinPaths(stdlibPath_, rootPart);
        if (containsDelim) {
            resolvedPath.append(1, PATH_DELIM).append(path, rootPart.length() + 1, path.length());
        }
    } else {
        auto resolved = config_->ResolvePath(path);
        if (resolved) {
            resolvedPath = *resolved;
        } else {
            return ResultPath::Error(ErrorCode::ARKTS_PREFIX_FAIL, path);
        }
    }
    return ResultPath::Ok(resolvedPath);
}

std::vector<ImportPathResolver::PathT> ImportPathResolver::ReExportPaths(
    const ArenaVector<ir::ETSReExportDeclaration *, false> &reExports, const std::vector<PathT> &alreadyExported)
{
    std::vector<PathT> result;
    for (auto reExport : reExports) {
        auto progPath = reExport->GetProgramPath().Mutf8();
        if (std::count(alreadyExported.begin(), alreadyExported.end(), progPath) == 0U) {
            continue;
        }
        auto path = progPath.substr(0, progPath.find_last_of(PATH_DELIM));
        for (auto item : reExport->GetUserPaths()) {
            const auto reExPath = item.Mutf8();
            result.push_back(
                JoinPaths(path, reExPath.substr(reExPath.find_first_of(PATH_DELIM) + 1, reExPath.length())));
        }
    }
    return result;
}

bool ImportPathResolver::IsCompatibleExtension(const std::string &extension) const
{
    return allowedExts_.count(extension) != 0U;
}

bool ImportPathResolver::IsCompatibleFile(const std::string &fileName) const
{
    std::string::size_type pos = fileName.find_last_of('.');
    if (pos == std::string::npos) {
        return false;
    }
    return IsCompatibleExtension(fileName.substr(pos));
}

bool ImportPathResolver::IsStdLib(const parser::Program *program) const
{
    const auto &stdlib = StdLib();
    return std::count(stdlib.begin(), stdlib.end(), std::string(program->SourceFileFolder())) != 0;
}

ImportPathResolver::ResultT<std::vector<ImportPathResolver::PathT>> ImportPathResolver::ListFiles(
    const PathT &resolvedPath) const
{
    std::vector<PathT> files;
#ifdef USE_UNIX_SYSCALL
    DIR *dir = opendir(resolvedPath.c_str());

    if (dir == nullptr) {
        return ResultT<std::vector<PathT>>::Error(ErrorCode::INCORRECT_PATH, resolvedPath);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_REG) {
            continue;
        }

        std::string fileName = entry->d_name;
        if (!IsCompatibleFile(fileName)) {
            continue;
        }

        files.emplace_back(fileName);
    }

    closedir(dir);
#else
    for (auto const &entry : fs::directory_iterator(resolvedPath)) {
        if (!fs::is_regular_file(entry) || !IsCompatibleExtension(entry.path().extension().string())) {
            continue;
        }
        files.emplace_back(entry.path().filename().string());
    }
#endif
    std::sort(files.begin(), files.end());
    return ResultT<std::vector<ImportPathResolver::PathT>>::Ok(files);
}

bool ImportPathResolver::IsIndexFile(const std::string &fileName)
{
    return fileName == "index.ets" || fileName == "index.ts";
}

}  // namespace panda::es2panda::util
#undef USE_UNIX_SYSCALL
