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

#ifndef PANDA_IMPORTPATHRESOLVER_H
#define PANDA_IMPORTPATHRESOLVER_H

#include <string>
#include <utility>
#include "util/helpers.h"
#include "util/arktsconfig.h"
#include "ir/ets/etsReExportDeclaration.h"
#include "parser/program/program.h"
#include "es2panda.h"
#include "os/filesystem.h"
#include "util/error_handler.h"
#include "util/result.h"

namespace panda::es2panda::util {

class ImportPathResolver {
public:
    enum class ErrorCode {
        INCORRECT_PATH,
        ARKTS_PREFIX_FAIL,
    };

    template <typename T>
    using ResultT = Result<T, ErrorCode, std::string>;

private:
    using PathT = std::string;

    using ResultPath = ResultT<PathT>;

    struct UserSources {
        std::vector<PathT> sources;
        bool isModule {false};
    };
    using ResultSources = ResultT<UserSources>;

    const std::unordered_set<std::string> allowedExts_ = {".ets", ".ts"};

public:
    struct ImportData {
        Language lang;
        std::string module;
        bool hasDecl;
    };

    ImportPathResolver(std::shared_ptr<const ArkTsConfig> config, PathT stdlibPath, ScriptExtension extension)
        : config_(std::move(config)), stdlibPath_(std::move(stdlibPath)), extension_(extension)
    {
    }

    static std::vector<PathT> ReExportPaths(const ArenaVector<ir::ETSReExportDeclaration *> &reExports,
                                            const std::vector<std::string> &alreadyExported);

    void AddResolved(const PathT &path, const PathT &realPath)
    {
        resolvedParsedSources_.emplace(path, realPath);
    }

    ResultPath ResolveImportPath(const PathT &path, const parser::Program *program);

    ImportData GetImportData(const PathT &path) const;

    const std::unordered_map<std::string, std::string> &ResolvedParsedSourcesMap() const
    {
        return resolvedParsedSources_;
    }

    static std::vector<PathT> &StdLib()
    {
        static std::vector<std::string> stdlib {"std/core", "std/math",       "std/containers",
                                                "std/time", "std/interop/js", "escompat"};
        return stdlib;
    }

    bool IsStdLib(const parser::Program *program) const;

    ResultSources CollectUserSources(const PathT &path, const parser::Program *program);

    ResultT<std::vector<PathT>> CollectDefaultSources(const std::vector<PathT> &stdlib, const parser::Program *program);

    struct SourceInfo {
        PathT filePath;
        PathT resolvedPath;
        bool isModule;
        Language lang;
    };

    ResultT<std::vector<SourceInfo>> ParseSources(const std::vector<PathT> &paths, const parser::Program *program);

    /**
     * Removes from `sources` already parsed files, and mark this paths as resolved
     * @param sources
     * @param resolved_fp
     * @param source_fp
     */
    void FilterParsedSources(std::vector<std::string> &sources, const parser::Program *program);

    /**
     * Resolves root path. E.g. rel_dir/file.ets -> absolute_path/rel_dir
     * @param path
     * @return root path
     */
    ResultPath ResolveRootPath(const PathT &path) const;

private:
    ResultT<std::vector<PathT>> CollectUserSourcesFromIndex(const PathT &path, const PathT &resolvedPath);

    /**
     * @param path
     * @param resolved_path
     * @return (src path, is_module)
     */
    std::pair<ResultPath, bool> GetSourceRegularPath(const PathT &path, const PathT &resolvedPath) const;
    static PathT ResolveFullPathFromRelative(const PathT &path, const parser::Program *program);
    ResultPath GetImportPath(const PathT &path, const parser::Program *program) const;

    /**
     * @param resolvedPath directory absolute path
     * @return alphabetically ordered ts/ets files in directory
     */
    ResultT<std::vector<PathT>> ListFiles(const PathT &resolvedPath) const;

    bool IsCompatibleExtension(const std::string &extension) const;

    bool IsCompatibleFile(const std::string &fileName) const;

    static bool IsIndexFile(const std::string &fileName);

private:
    std::shared_ptr<const ArkTsConfig> config_;
    std::unordered_map<std::string, std::string> resolvedParsedSources_;
    const PathT stdlibPath_;
    const ScriptExtension extension_;
    static constexpr char PATH_DELIM = panda::os::file::File::GetPathDelim().at(0);
};
}  // namespace panda::es2panda::util

#endif  // PANDA_IMPORTPATHRESOLVER_H
