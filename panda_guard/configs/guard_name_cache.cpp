/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "guard_name_cache.h"

#include "nlohmann/json.hpp"
#include "utils/logger.h"

#include "util/file_util.h"
#include "util/json_util.h"
#include "util/assert_util.h"

namespace {
    const std::string TAG = "[Guard_NameCache]";
    const std::string IDENTIFIER_CACHE = "IdentifierCache";
    const std::string MEMBER_METHOD_CACHE = "MemberMethodCache";
    const std::string OBF_NAME = "obfName";
    const std::string ORI_SOURCE_FILE = "OriSourceFile";
    const std::string OBF_SOURCE_FILE = "ObfSourceFile";
    const std::string ENTRY_PACKAGE_INFO = "entryPackageInfo";
    const std::string COMPILE_SDK_VERSION = "compileSdkVersion";
    const std::string PROPERTY_CACHE = "PropertyCache";
    const std::string FILE_NAME_CACHE = "FileNameCache";
    constexpr int JSON_FILE_INDENT_SIZE = 2;

    std::map<std::string, std::string> DeleteScopeAndLineNum(const std::map<std::string, std::string> &originMap)
    {
        std::map<std::string, std::string> res;
        for (const auto &item: originMap) {
            auto key = item.first;
            size_t pos = key.rfind('#');
            if (pos != std::string::npos) {
                key = key.substr(pos + 1);
            }
            pos = key.find(':');
            if (pos != std::string::npos) {
                key = key.substr(0, pos);
            }
            res.emplace(key, item.second);
        }
        return res;
    }

    nlohmann::json Map2Json(const std::map<std::string, std::string> &table, bool deduplicate = false)
    {
        if (table.empty()) {
            return nlohmann::json({});
        }

        nlohmann::json obj;
        for (const auto &item: table) {
            if (deduplicate && item.first == item.second) {
                continue;
            }
            obj[item.first] = item.second;
        }
        if (obj.empty()) {
            return nlohmann::json({});
        }
        return obj;
    }

    bool IsExist(const std::map<std::string, std::string> &table, const std::string &str)
    {
        std::string match = str + ":\\d+:\\d+";
        std::regex pattern(match);
        return std::any_of(table.begin(), table.end(), [&](const auto &item) {
            return std::regex_search(item.first, pattern);
        });
    }
}

void panda::guard::NameCache::Load(const std::string &applyNameCachePath)
{
    if (applyNameCachePath.empty()) {
        return;
    }
    std::string content = FileUtil::GetFileContent(applyNameCachePath);
    if (content.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "get apply name cache file content failed";
        return;
    }

    ParseHistory(content);
}

const std::set<std::string> &panda::guard::NameCache::GetHistoryUsedNames() const
{
    return historyUsedNames_;
}

std::map<std::string, std::string> panda::guard::NameCache::GetHistoryFileNameMapping() const
{
    return historyNameCache_.fileNameCacheMap_;
}

std::map<std::string, std::string> panda::guard::NameCache::GetHistoryNameMapping() const
{
    std::map<std::string, std::string> res = historyNameCache_.propertyCacheMap_;
    for (const auto &item: historyNameCache_.fileCacheInfoMap_) {
        res.insert(item.second.identifierCacheMap_.begin(), item.second.identifierCacheMap_.end());
        res.insert(item.second.memberMethodCacheMap_.begin(), item.second.memberMethodCacheMap_.end());
    }
    return res;
}

void panda::guard::NameCache::AddObfIdentifierName(const std::string &filePath,
                                                   const std::string &origin,
                                                   const std::string &obfName)
{
    if (filePath.empty() || origin.empty() || obfName.empty()) {
        LOG(WARNING, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], filePath or origin or obfName is empty";
        LOG(INFO, PANDAGUARD) << TAG << "filePath:" << filePath;
        LOG(INFO, PANDAGUARD) << TAG << "origin:" << origin;
        LOG(INFO, PANDAGUARD) << TAG << "obfName:" << obfName;
        return;
    }
    auto fileItem = newNameCache_.fileCacheInfoMap_.find(filePath);
    if (fileItem != newNameCache_.fileCacheInfoMap_.end()) {
        if (!IsExist(fileItem->second.identifierCacheMap_, origin)) {
            fileItem->second.identifierCacheMap_.emplace(origin, obfName);
        }
        return;
    }
    FileNameCacheInfo cacheInfo;
    cacheInfo.identifierCacheMap_.emplace(origin, obfName);
    newNameCache_.fileCacheInfoMap_.emplace(filePath, cacheInfo);
}

void panda::guard::NameCache::AddObfMemberMethodName(const std::string &filePath,
                                                     const std::string &origin,
                                                     const std::string &obfName)
{
    if (filePath.empty() || origin.empty() || obfName.empty()) {
        LOG(ERROR, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], filePath or origin or obfName is empty";
        LOG(INFO, PANDAGUARD) << TAG << "filePath:" << filePath;
        LOG(INFO, PANDAGUARD) << TAG << "origin:" << origin;
        LOG(INFO, PANDAGUARD) << TAG << "obfName:" << obfName;
        return;
    }
    auto fileItem = newNameCache_.fileCacheInfoMap_.find(filePath);
    if (fileItem != newNameCache_.fileCacheInfoMap_.end()) {
        fileItem->second.memberMethodCacheMap_.emplace(origin, obfName);
        return;
    }
    FileNameCacheInfo cacheInfo;
    cacheInfo.memberMethodCacheMap_.emplace(origin, obfName);
    newNameCache_.fileCacheInfoMap_.emplace(filePath, cacheInfo);
}

void panda::guard::NameCache::AddObfName(const std::string &filePath, const std::string &obfName)
{
    if (filePath.empty() || obfName.empty()) {
        LOG(ERROR, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], filePath or obfName is empty";
        LOG(INFO, PANDAGUARD) << TAG << "filePath:" << filePath;
        LOG(INFO, PANDAGUARD) << TAG << "obfName:" << obfName;
        return;
    }
    auto fileItem = newNameCache_.fileCacheInfoMap_.find(filePath);
    if (fileItem != newNameCache_.fileCacheInfoMap_.end()) {
        fileItem->second.obfName_ = obfName;
        return;
    }
    FileNameCacheInfo cacheInfo;
    cacheInfo.obfName_ = obfName;
    newNameCache_.fileCacheInfoMap_.emplace(filePath, cacheInfo);
}

void panda::guard::NameCache::AddSourceFile(const std::string &filePath,
                                            const std::string &oriSource, const std::string &obfSource)
{
    if (filePath.empty() || oriSource.empty() || obfSource.empty()) {
        LOG(ERROR, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], filePath or obfName is empty";
        LOG(INFO, PANDAGUARD) << TAG << "filePath:" << filePath;
        LOG(INFO, PANDAGUARD) << TAG << "oriSourceFile:" << oriSource;
        LOG(INFO, PANDAGUARD) << TAG << "obfSourceFile:" << obfSource;
        return;
    }
    auto fileItem = newNameCache_.fileCacheInfoMap_.find(filePath);
    if (fileItem != newNameCache_.fileCacheInfoMap_.end()) {
        fileItem->second.oriSourceFile_ = oriSource;
        fileItem->second.obfSourceFile_ = obfSource;
        return;
    }
    FileNameCacheInfo cacheInfo;
    cacheInfo.oriSourceFile_ = oriSource;
    cacheInfo.obfSourceFile_ = obfSource;
    newNameCache_.fileCacheInfoMap_.emplace(filePath, cacheInfo);
}

void panda::guard::NameCache::AddNewNameCacheObfFileName(const std::string &origin, const std::string &obfName)
{
    if (origin.empty() || obfName.empty()) {
        LOG(ERROR, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], origin or obfName is empty";
        LOG(INFO, PANDAGUARD) << TAG << "origin:" << origin;
        LOG(INFO, PANDAGUARD) << TAG << "obfName:" << obfName;
        return;
    }
    newNameCache_.fileNameCacheMap_.emplace(origin, obfName);
}

void panda::guard::NameCache::AddObfPropertyName(const std::string &origin, const std::string &obfName)
{
    if (origin.empty() || obfName.empty()) {
        LOG(ERROR, PANDAGUARD) << TAG << "[" << __FUNCTION__ << "], origin or obfName is empty";
        LOG(INFO, PANDAGUARD) << TAG << "origin:" << origin;
        LOG(INFO, PANDAGUARD) << TAG << "obfName:" << obfName;
        return;
    }
    newNameCache_.propertyCacheMap_.emplace(origin, obfName);
}

void panda::guard::NameCache::WriteCache()
{
    std::string defaultPath = options_->GetDefaultNameCachePath();
    std::string printPath = options_->GetPrintNameCache();
    PANDA_GUARD_ASSERT_PRINT(
        defaultPath.empty() && printPath.empty(),
        TAG << "default and print name cache path is empty");

    ProjectNameCacheInfo merge;
    merge.fileCacheInfoMap_ = newNameCache_.fileCacheInfoMap_;
    merge.entryPackageInfo_ = options_->GetEntryPackageInfo();
    merge.compileSdkVersion_ = options_->GetCompileSdkVersion();
    merge.propertyCacheMap_ = historyNameCache_.propertyCacheMap_;
    merge.propertyCacheMap_.insert(newNameCache_.propertyCacheMap_.begin(), newNameCache_.propertyCacheMap_.end());
    merge.fileNameCacheMap_ = historyNameCache_.fileNameCacheMap_;
    merge.fileNameCacheMap_.insert(newNameCache_.fileNameCacheMap_.begin(), newNameCache_.fileNameCacheMap_.end());

    std::string jsonStr = BuildJson(merge);
    if (!defaultPath.empty()) {
        FileUtil::WriteFile(defaultPath, jsonStr);
    }

    if (!printPath.empty()) {
        FileUtil::WriteFile(printPath, jsonStr);
    }
}

void panda::guard::NameCache::ParseHistory(const std::string &content)
{
    JsonObject nameCacheObj(content);
    PANDA_GUARD_ASSERT_PRINT(!nameCacheObj.IsValid(), TAG << "name cache file content is invalid json");

    for (size_t idx = 0; idx < nameCacheObj.GetSize(); idx++) {
        auto key = nameCacheObj.GetKeyByIndex(idx);
        if (key == ENTRY_PACKAGE_INFO) {
            historyNameCache_.entryPackageInfo_ = JsonUtil::GetStringValue(&nameCacheObj, ENTRY_PACKAGE_INFO);
            AddHistoryUsedNames(historyNameCache_.entryPackageInfo_);
        } else if (key == COMPILE_SDK_VERSION) {
            historyNameCache_.compileSdkVersion_ = JsonUtil::GetStringValue(&nameCacheObj, COMPILE_SDK_VERSION);
            AddHistoryUsedNames(historyNameCache_.compileSdkVersion_);
        } else if (key == PROPERTY_CACHE) {
            historyNameCache_.propertyCacheMap_ = JsonUtil::GetMapStringValue(&nameCacheObj, PROPERTY_CACHE);
            AddHistoryUsedNames(historyNameCache_.propertyCacheMap_);
        } else if (key == FILE_NAME_CACHE) {
            historyNameCache_.fileNameCacheMap_ = JsonUtil::GetMapStringValue(&nameCacheObj, FILE_NAME_CACHE);
            AddHistoryUsedNames(historyNameCache_.fileNameCacheMap_);
        } else {
            auto fileCacheInfoObj = JsonUtil::GetJsonObject(&nameCacheObj, key);
            if (fileCacheInfoObj) {
                auto identifierCache = JsonUtil::GetMapStringValue(fileCacheInfoObj, IDENTIFIER_CACHE);
                auto memberMethodCache = JsonUtil::GetMapStringValue(fileCacheInfoObj, MEMBER_METHOD_CACHE);

                FileNameCacheInfo fileCacheInfo;
                fileCacheInfo.identifierCacheMap_ = DeleteScopeAndLineNum(identifierCache);
                fileCacheInfo.memberMethodCacheMap_ = DeleteScopeAndLineNum(memberMethodCache);
                fileCacheInfo.obfName_ = JsonUtil::GetStringValue(fileCacheInfoObj, OBF_NAME);
                fileCacheInfo.oriSourceFile_ = JsonUtil::GetStringValue(fileCacheInfoObj, ORI_SOURCE_FILE);
                fileCacheInfo.obfSourceFile_ = JsonUtil::GetStringValue(fileCacheInfoObj, OBF_SOURCE_FILE);

                historyNameCache_.fileCacheInfoMap_.emplace(key, fileCacheInfo);

                AddHistoryUsedNames(fileCacheInfo.identifierCacheMap_);
                AddHistoryUsedNames(fileCacheInfo.memberMethodCacheMap_);
                AddHistoryUsedNames(fileCacheInfo.obfName_);
            }
        }
    }
}

void panda::guard::NameCache::AddHistoryUsedNames(const std::string &value)
{
    historyUsedNames_.emplace(value);
}

void panda::guard::NameCache::AddHistoryUsedNames(const std::map<std::string, std::string> &values)
{
    for (const auto &item: values) {
        historyUsedNames_.emplace(item.second);
    }
}

std::string panda::guard::NameCache::BuildJson(const ProjectNameCacheInfo &nameCacheInfo)
{
    nlohmann::ordered_json jsonObject;
    for (const auto &item: nameCacheInfo.fileCacheInfoMap_) {
        jsonObject[item.first][IDENTIFIER_CACHE] = Map2Json(item.second.identifierCacheMap_);
        jsonObject[item.first][MEMBER_METHOD_CACHE] = Map2Json(item.second.memberMethodCacheMap_);
        jsonObject[item.first][OBF_NAME] = item.second.obfName_;
        jsonObject[item.first][ORI_SOURCE_FILE] = item.second.oriSourceFile_;
        jsonObject[item.first][OBF_SOURCE_FILE] = item.second.obfSourceFile_;
    }
    jsonObject[ENTRY_PACKAGE_INFO] = nameCacheInfo.entryPackageInfo_;
    jsonObject[COMPILE_SDK_VERSION] = nameCacheInfo.compileSdkVersion_;

    if (options_->EnableProperty()) {
        jsonObject[PROPERTY_CACHE] = Map2Json(nameCacheInfo.propertyCacheMap_, true);
    }
    if (options_->EnableFileName()) {
        jsonObject[FILE_NAME_CACHE] = Map2Json(nameCacheInfo.fileNameCacheMap_, true);
    }
    return jsonObject.dump(JSON_FILE_INDENT_SIZE);
}
