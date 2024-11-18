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

#include "guard_options.h"

#include "utils/logger.h"

#include "util/file_util.h"
#include "util/json_util.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
    const std::string TAG = "[Guard_Options]";

    const std::string ABC_FILE_PATH = "abcFilePath";
    const std::string OBF_ABC_FILE_PATH = "obfAbcFilePath";
    const std::string OBF_PA_FILE_PATH = "obfPaFilePath";
    const std::string COMPILE_SDK_VERSION = "compileSdkVersion";
    const std::string TARGET_API_VERSION = "targetApiVersion";
    const std::string TARGET_API_SUB_VERSION = "targetApiSubVersion";
    const std::string FILES_INFO_PATH = "filesInfoPath";
    const std::string SOURCE_MAPS_PATH = "sourceMapsPath";
    const std::string ENTRY_PACKAGE_INFO = "entryPackageInfo";
    const std::string DEFAULT_NAME_CACHE_PATH = "defaultNameCachePath";
    const std::string OBFUSCATION_RULES = "obfuscationRules";
    const std::string DISABLE_OBFUSCATION = "disableObfuscation";
    const std::string ENABLE_EXPORT_OBFUSCATION = "enableExportObfuscation";
    const std::string ENABLE_REMOVE_LOG = "enableRemoveLog";
    const std::string PRINT_NAME_CACHE = "printNameCache";
    const std::string APPLY_NAME_CACHE = "applyNameCache";
    const std::string RESERVED_NAMES = "reservedNames";
    const std::string ENABLE = "enable";
    const std::string PROPERTY_OBFUSCATION = "propertyObfuscation";
    const std::string RESERVED_PROPERTIES = "reservedProperties";
    const std::string UNIVERSAL_RESERVED_PROPERTIES = "universalReservedProperties";
    const std::string TOPLEVEL_OBFUSCATION = "toplevelObfuscation";
    const std::string RESERVED_TOPLEVEL_NAMES = "reservedToplevelNames";
    const std::string UNIVERSAL_RESERVED_TOPLEVEL_NAMES = "universalReservedToplevelNames";
    const std::string FILE_NAME_OBFUSCATION = "fileNameObfuscation";
    const std::string RESERVED_FILE_NAMES = "reservedFileNames";
    const std::string UNIVERSAL_RESERVED_FILE_NAMES = "universalReservedFileNames";
    const std::string KEEP_OPTIONS = "keepOptions";
    const std::string KEEP_PATHS = "keepPaths";
    const std::string SKIPPED_REMOTE_HAR_LIST = "skippedRemoteHarList";
    const std::string USE_NORMALIZED_OHM_URL = "useNormalizedOHMUrl";

    constexpr std::string_view FILES_INFO_DELIMITER = ";";
    constexpr size_t FILES_INFO_INDEX_1_RECORD_NAME = 1;
    constexpr size_t FILES_INFO_INDEX_3_SOURCE_KEY = 3;
    const std::string_view SOURCE_MAPS_SOURCES = "sources";

    void FillObfuscationOption(const panda::JsonObject *obj,
                               const std::string &objKey, const std::string &reservedKey,
                               const std::string &universalKey,
                               panda::guard::ObfuscationOption &option)
    {
        auto innerObj = panda::guard::JsonUtil::GetJsonObject(obj, objKey);
        if (!innerObj) {
            LOG(INFO, PANDAGUARD) << TAG << "not config " << objKey;
            return;
        }
        option.enable = panda::guard::JsonUtil::GetBoolValue(innerObj, ENABLE);
        option.reservedList = panda::guard::JsonUtil::GetArrayStringValue(innerObj, reservedKey);
        option.universalReservedList = panda::guard::JsonUtil::GetArrayStringValue(innerObj, universalKey);
        for (auto &str: option.universalReservedList) {
            panda::guard::StringUtil::RemoveSlashFromBothEnds(str);
        }
    }

    void FillPropertyOption(const panda::JsonObject *obj, panda::guard::ObfuscationOption &option)
    {
        FillObfuscationOption(obj, PROPERTY_OBFUSCATION, RESERVED_PROPERTIES, UNIVERSAL_RESERVED_PROPERTIES, option);
    }

    void FillToplevelOption(const panda::JsonObject *obj, panda::guard::ObfuscationOption &option)
    {
        FillObfuscationOption(obj, TOPLEVEL_OBFUSCATION, RESERVED_TOPLEVEL_NAMES, UNIVERSAL_RESERVED_TOPLEVEL_NAMES,
                              option);
    }

    void FillFileNameOption(const panda::JsonObject *obj, panda::guard::ObfuscationOption &option)
    {
        FillObfuscationOption(obj, FILE_NAME_OBFUSCATION, RESERVED_FILE_NAMES, UNIVERSAL_RESERVED_FILE_NAMES, option);
    }

    void FillKeepOption(const panda::JsonObject *obj, panda::guard::KeepOption &option)
    {
        auto innerObj = panda::guard::JsonUtil::GetJsonObject(obj, KEEP_OPTIONS);
        if (!innerObj) {
            LOG(INFO, PANDAGUARD) << TAG << "not config " << KEEP_OPTIONS;
            return;
        }
        option.enable = panda::guard::JsonUtil::GetBoolValue(innerObj, ENABLE);
        option.keepPaths = panda::guard::JsonUtil::GetArrayStringValue(innerObj, KEEP_PATHS);
    }

    void FillObfuscationConfig(const std::string &content, panda::guard::ObfuscationConfig &obfConfig)
    {
        panda::JsonObject configObj(content);
        PANDA_GUARD_ASSERT_PRINT(!configObj.IsValid(), TAG << "config file content is invalid json");

        obfConfig.abcFilePath = panda::guard::JsonUtil::GetStringValue(&configObj, ABC_FILE_PATH);
        obfConfig.obfAbcFilePath = panda::guard::JsonUtil::GetStringValue(&configObj, OBF_ABC_FILE_PATH);
        obfConfig.obfPaFilePath = panda::guard::JsonUtil::GetStringValue(&configObj, OBF_PA_FILE_PATH);
        obfConfig.compileSdkVersion = panda::guard::JsonUtil::GetStringValue(&configObj, COMPILE_SDK_VERSION);
        obfConfig.targetApiVersion = (uint8_t) panda::guard::JsonUtil::GetDoubleValue(&configObj, TARGET_API_VERSION);
        obfConfig.targetApiSubVersion = panda::guard::JsonUtil::GetStringValue(&configObj, TARGET_API_SUB_VERSION);
        obfConfig.filesInfoPath = panda::guard::JsonUtil::GetStringValue(&configObj, FILES_INFO_PATH);
        obfConfig.sourceMapsPath = panda::guard::JsonUtil::GetStringValue(&configObj, SOURCE_MAPS_PATH);
        obfConfig.entryPackageInfo = panda::guard::JsonUtil::GetStringValue(&configObj, ENTRY_PACKAGE_INFO);
        obfConfig.defaultNameCachePath = panda::guard::JsonUtil::GetStringValue(&configObj, DEFAULT_NAME_CACHE_PATH);
        obfConfig.skippedRemoteHarList = panda::guard::JsonUtil::GetArrayStringValue(&configObj,
                                                                                     SKIPPED_REMOTE_HAR_LIST);
        obfConfig.useNormalizedOHMUrl = panda::guard::JsonUtil::GetBoolValue(&configObj, USE_NORMALIZED_OHM_URL);

        auto rulesObj = panda::guard::JsonUtil::GetJsonObject(&configObj, OBFUSCATION_RULES);
        PANDA_GUARD_ASSERT_PRINT(!rulesObj, TAG << "not config obfuscation rules");

        auto obfRule = &obfConfig.obfuscationRules;
        obfRule->disableObfuscation = panda::guard::JsonUtil::GetBoolValue(rulesObj, DISABLE_OBFUSCATION);
        obfRule->enableExportObfuscation = panda::guard::JsonUtil::GetBoolValue(rulesObj, ENABLE_EXPORT_OBFUSCATION);
        obfRule->enableRemoveLog = panda::guard::JsonUtil::GetBoolValue(rulesObj, ENABLE_REMOVE_LOG);
        obfRule->printNameCache = panda::guard::JsonUtil::GetStringValue(rulesObj, PRINT_NAME_CACHE);
        obfRule->applyNameCache = panda::guard::JsonUtil::GetStringValue(rulesObj, APPLY_NAME_CACHE);
        obfRule->reservedNames = panda::guard::JsonUtil::GetArrayStringValue(rulesObj, RESERVED_NAMES);
        FillPropertyOption(rulesObj, obfRule->propertyOption);
        FillToplevelOption(rulesObj, obfRule->toplevelOption);
        FillFileNameOption(rulesObj, obfRule->fileNameOption);
        FillKeepOption(rulesObj, obfRule->keepOption);
    }

    bool NeedToBeReserved(const std::vector<std::string> &reservedNames,
                          const std::vector<std::string> &universalReservedNames,
                          const std::string &name)
    {
        if (std::any_of(reservedNames.begin(), reservedNames.end(), [&](const auto &field) {
            return field == name;
        })) {
            return true;
        }

        return std::any_of(universalReservedNames.begin(), universalReservedNames.end(), [&](const auto &field) {
            std::regex pattern(field);
            return std::regex_search(name, pattern);
        });
    }

    void FillFilesInfoTable(const std::string &filesInfoPath,
                            std::unordered_map<std::string, std::string> &filesInfoTable)
    {
        if (filesInfoPath.empty()) {
            LOG(INFO, PANDAGUARD) << TAG << "filesInfoPath is empty";
            return;
        }
        auto lineDataList = panda::guard::FileUtil::GetLineDataFromFile(filesInfoPath);
        if (lineDataList.empty()) {
            LOG(WARNING, PANDAGUARD) << TAG << "get line data from filesInfoPath failed";
            return;
        }
        for (const auto &line: lineDataList) {
            auto infoList = panda::guard::StringUtil::StrictSplit(line, FILES_INFO_DELIMITER.data());
            if (infoList.size() < (FILES_INFO_INDEX_3_SOURCE_KEY + 1)) {
                LOG(WARNING, PANDAGUARD) << TAG << "line info is not normal size : " << line;
                continue;
            }
            filesInfoTable.emplace(infoList[FILES_INFO_INDEX_1_RECORD_NAME], infoList[FILES_INFO_INDEX_3_SOURCE_KEY]);
        }
    }

    void FillSourceMapsTable(const std::string &sourceMapsPath,
                             std::unordered_map<std::string, std::string> &sourceMapsTable)
    {
        if (sourceMapsPath.empty()) {
            LOG(INFO, PANDAGUARD) << TAG << "sourceMapsPath is empty";
            return;
        }
        std::string content = panda::guard::FileUtil::GetFileContent(sourceMapsPath);
        if (content.empty()) {
            LOG(WARNING, PANDAGUARD) << TAG << "get sourceMaps file content failed";
            return;
        }
        panda::JsonObject sourceMapsObj(content);
        if (!sourceMapsObj.IsValid()) {
            LOG(WARNING, PANDAGUARD) << TAG << "sourceMaps file content is invalid json";
            return;
        }
        for (size_t idx = 0; idx < sourceMapsObj.GetSize(); idx++) {
            auto key = sourceMapsObj.GetKeyByIndex(idx);
            auto sourceObj = panda::guard::JsonUtil::GetJsonObject(&sourceMapsObj, key);
            if (!sourceObj) {
                LOG(WARNING, PANDAGUARD) << TAG << key << "is invalid object in sourceMaps file";
                continue;
            }
            auto sources = panda::guard::JsonUtil::GetArrayStringValue(sourceObj, SOURCE_MAPS_SOURCES.data());
            if (sources.empty()) {
                LOG(WARNING, PANDAGUARD) << TAG << "sources is empty array in " << key << " in sourceMaps file";
                continue;
            }
            sourceMapsTable.emplace(key, sources[0]);
        }
    }

    void FillSourceNameTable(const std::string &filesInfoPath, const std::string &sourceMapsPath,
                             std::unordered_map<std::string, std::string> &sourceNameTable)
    {
        std::unordered_map<std::string, std::string> filesInfoTable;
        FillFilesInfoTable(filesInfoPath, filesInfoTable);
        if (filesInfoTable.empty()) {
            return;
        }

        std::unordered_map<std::string, std::string> sourceMapsTable;
        FillSourceMapsTable(sourceMapsPath, sourceMapsTable);
        if (sourceMapsTable.empty()) {
            return;
        }

        for (const auto &[recordName, sourceMapsKey]: filesInfoTable) {
            auto item = sourceMapsTable.find(sourceMapsKey);
            if (item == sourceMapsTable.end()) {
                LOG(WARNING, PANDAGUARD) << TAG << sourceMapsKey << " int filesInfo, but not in sourceMaps";
                continue;
            }
            sourceNameTable.emplace(recordName, item->second);
        }
    }
}

void panda::guard::GuardOptions::Load(const std::string &configFilePath)
{
    std::string fileContent = FileUtil::GetFileContent(configFilePath);
    PANDA_GUARD_ASSERT_PRINT(fileContent.empty(), TAG << "config file is empty");

    FillObfuscationConfig(fileContent, this->obfConfig_);
    PANDA_GUARD_ASSERT_PRINT(
        obfConfig_.abcFilePath.empty() || obfConfig_.obfAbcFilePath.empty(),
        TAG << "abcFilePath and obfAbcFilePath must not empty");

    PANDA_GUARD_ASSERT_PRINT(
        (obfConfig_.targetApiVersion == 0) || obfConfig_.targetApiSubVersion.empty(),
        TAG << "targetApiVersion and targetApiSubVersion must not empty");

    LOG(INFO, PANDAGUARD) << TAG << "disableObfuscation_:" << obfConfig_.obfuscationRules.disableObfuscation;
    LOG(INFO, PANDAGUARD) << TAG << "export obfuscation:" << obfConfig_.obfuscationRules.enableExportObfuscation;
    LOG(INFO, PANDAGUARD) << TAG << "removeLog obfuscation:" << obfConfig_.obfuscationRules.enableRemoveLog;
    LOG(INFO, PANDAGUARD) << TAG << "property obfuscation:" << obfConfig_.obfuscationRules.propertyOption.enable;
    LOG(INFO, PANDAGUARD) << TAG << "topLevel obfuscation:" << obfConfig_.obfuscationRules.toplevelOption.enable;
    LOG(INFO, PANDAGUARD) << TAG << "fileName obfuscation:" << obfConfig_.obfuscationRules.fileNameOption.enable;

    FillSourceNameTable(obfConfig_.filesInfoPath, obfConfig_.sourceMapsPath, this->sourceNameTable_);
}

const std::string &panda::guard::GuardOptions::GetAbcFilePath() const
{
    return obfConfig_.abcFilePath;
}

const std::string &panda::guard::GuardOptions::GetObfAbcFilePath() const
{
    return obfConfig_.obfAbcFilePath;
}

const std::string &panda::guard::GuardOptions::GetObfPaFilePath() const
{
    return obfConfig_.obfPaFilePath;
}

const std::string &panda::guard::GuardOptions::GetCompileSdkVersion() const
{
    return obfConfig_.compileSdkVersion;
}

uint8_t panda::guard::GuardOptions::GetTargetApiVersion() const
{
    return obfConfig_.targetApiVersion;
}

const std::string &panda::guard::GuardOptions::GetTargetApiSubVersion() const
{
    return obfConfig_.targetApiSubVersion;
}

const std::string &panda::guard::GuardOptions::GetEntryPackageInfo() const
{
    return obfConfig_.entryPackageInfo;
}

const std::string &panda::guard::GuardOptions::GetDefaultNameCachePath() const
{
    return obfConfig_.defaultNameCachePath;
}

bool panda::guard::GuardOptions::DisableObfuscation() const
{
    return obfConfig_.obfuscationRules.disableObfuscation;
}

bool panda::guard::GuardOptions::EnableExport() const
{
    return obfConfig_.obfuscationRules.enableExportObfuscation;
}

bool panda::guard::GuardOptions::EnableRemoveLog() const
{
    return obfConfig_.obfuscationRules.enableRemoveLog;
}

const std::string &panda::guard::GuardOptions::GetPrintNameCache() const
{
    return obfConfig_.obfuscationRules.printNameCache;
}

const std::string &panda::guard::GuardOptions::GetApplyNameCache() const
{
    return obfConfig_.obfuscationRules.applyNameCache;
}

bool panda::guard::GuardOptions::EnableProperty() const
{
    return obfConfig_.obfuscationRules.propertyOption.enable;
}

bool panda::guard::GuardOptions::EnableToplevel() const
{
    return obfConfig_.obfuscationRules.toplevelOption.enable;
}

bool panda::guard::GuardOptions::EnableFileName() const
{
    return obfConfig_.obfuscationRules.fileNameOption.enable;
}

bool panda::guard::GuardOptions::IsKeepPath(const std::string &path) const
{
    const auto keepOption = &obfConfig_.obfuscationRules.keepOption;
    if (!keepOption->enable || path.empty()) {
        return false;
    }

    std::vector<std::string> universalKeepPaths;
    return NeedToBeReserved(keepOption->keepPaths, universalKeepPaths, path);
}

bool panda::guard::GuardOptions::IsReservedNames(const std::string &name) const
{
    std::vector<std::string> universalReservedNames;
    return NeedToBeReserved(obfConfig_.obfuscationRules.reservedNames, universalReservedNames, name);
}

bool panda::guard::GuardOptions::IsReservedProperties(const std::string &name) const
{
    return NeedToBeReserved(obfConfig_.obfuscationRules.propertyOption.reservedList,
                            obfConfig_.obfuscationRules.propertyOption.universalReservedList, name);
}

bool panda::guard::GuardOptions::IsReservedToplevelNames(const std::string &name) const
{
    return NeedToBeReserved(obfConfig_.obfuscationRules.toplevelOption.reservedList,
                            obfConfig_.obfuscationRules.toplevelOption.universalReservedList, name);
}

bool panda::guard::GuardOptions::IsReservedFileNames(const std::string &name) const
{
    return NeedToBeReserved(obfConfig_.obfuscationRules.fileNameOption.reservedList,
                            obfConfig_.obfuscationRules.fileNameOption.universalReservedList, name);
}

const std::string &panda::guard::GuardOptions::GetSourceName(const std::string &name) const
{
    auto item = this->sourceNameTable_.find(name);
    if (item == this->sourceNameTable_.end()) {
        return name;
    }
    return item->second;
}

bool panda::guard::GuardOptions::IsSkippedRemoteHar(const std::string &pkgName) const
{
    return std::any_of(obfConfig_.skippedRemoteHarList.begin(), obfConfig_.skippedRemoteHarList.end(),
                       [&](const std::string &remoteHar) {
                           return StringUtil::IsSuffixMatched(remoteHar, pkgName);
                       });
}

bool panda::guard::GuardOptions::UseNormalizedOhmUrl() const
{
    return obfConfig_.useNormalizedOHMUrl;
}
