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
    const std::string TARGET_API_VERSION_ = "targetApiVersion";
    const std::string TARGET_API_SUB_VERSION_ = "targetApiSubVersion";
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
        option.enable_ = panda::guard::JsonUtil::GetBoolValue(innerObj, ENABLE);
        option.reservedList_ = panda::guard::JsonUtil::GetArrayStringValue(innerObj, reservedKey);
        option.universalReservedList_ = panda::guard::JsonUtil::GetArrayStringValue(innerObj, universalKey);
        for (auto &str: option.universalReservedList_) {
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
        option.enable_ = panda::guard::JsonUtil::GetBoolValue(innerObj, ENABLE);
        option.keepPaths_ = panda::guard::JsonUtil::GetArrayStringValue(innerObj, KEEP_PATHS);
    }

    void FillObfuscationConfig(const std::string &content, panda::guard::ObfuscationConfig &obfConfig)
    {
        panda::JsonObject configObj(content);
        PANDA_GUARD_ASSERT_PRINT(!configObj.IsValid(), TAG << "config file content is invalid json");

        obfConfig.abcFilePath_ = panda::guard::JsonUtil::GetStringValue(&configObj, ABC_FILE_PATH);
        obfConfig.obfAbcFilePath_ = panda::guard::JsonUtil::GetStringValue(&configObj, OBF_ABC_FILE_PATH);
        obfConfig.obfPaFilePath_ = panda::guard::JsonUtil::GetStringValue(&configObj, OBF_PA_FILE_PATH);
        obfConfig.compileSdkVersion_ = panda::guard::JsonUtil::GetStringValue(&configObj, COMPILE_SDK_VERSION);
        obfConfig.targetApiVersion_ = (uint8_t)panda::guard::JsonUtil::GetDoubleValue(&configObj, TARGET_API_VERSION_);
        obfConfig.targetApiSubVersion_ = panda::guard::JsonUtil::GetStringValue(&configObj, TARGET_API_SUB_VERSION_);
        obfConfig.entryPackageInfo_ = panda::guard::JsonUtil::GetStringValue(&configObj, ENTRY_PACKAGE_INFO);
        obfConfig.defaultNameCachePath_ = panda::guard::JsonUtil::GetStringValue(&configObj, DEFAULT_NAME_CACHE_PATH);

        auto rulesObj = panda::guard::JsonUtil::GetJsonObject(&configObj, OBFUSCATION_RULES);
        PANDA_GUARD_ASSERT_PRINT(!rulesObj, TAG << "not config obfuscation rules");

        auto obfRule = &obfConfig.obfuscationRules_;
        obfRule->disableObfuscation_ = panda::guard::JsonUtil::GetBoolValue(rulesObj, DISABLE_OBFUSCATION);
        obfRule->enableExportObfuscation_ = panda::guard::JsonUtil::GetBoolValue(rulesObj, ENABLE_EXPORT_OBFUSCATION);
        obfRule->enableRemoveLog_ = panda::guard::JsonUtil::GetBoolValue(rulesObj, ENABLE_REMOVE_LOG);
        obfRule->printNameCache_ = panda::guard::JsonUtil::GetStringValue(rulesObj, PRINT_NAME_CACHE);
        obfRule->applyNameCache_ = panda::guard::JsonUtil::GetStringValue(rulesObj, APPLY_NAME_CACHE);
        obfRule->reservedNames_ = panda::guard::JsonUtil::GetArrayStringValue(rulesObj, RESERVED_NAMES);
        FillPropertyOption(rulesObj, obfRule->propertyOption_);
        FillToplevelOption(rulesObj, obfRule->toplevelOption_);
        FillFileNameOption(rulesObj, obfRule->fileNameOption_);
        FillKeepOption(rulesObj, obfRule->keepOption_);
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
}

void panda::guard::GuardOptions::Load(const std::string &configFilePath)
{
    std::string fileContent = FileUtil::GetFileContent(configFilePath);
    PANDA_GUARD_ASSERT_PRINT(fileContent.empty(), TAG << "config file is empty");

    FillObfuscationConfig(fileContent, this->obfConfig_);
    PANDA_GUARD_ASSERT_PRINT(
            obfConfig_.abcFilePath_.empty() || obfConfig_.obfAbcFilePath_.empty(),
            TAG << "abcFilePath and obfAbcFilePath must not empty");

    PANDA_GUARD_ASSERT_PRINT(
            (obfConfig_.targetApiVersion_ == 0) || obfConfig_.targetApiSubVersion_.empty(),
            TAG << "targetApiVersion and targetApiSubVersion must not empty");

    LOG(INFO, PANDAGUARD) << TAG << "disableObfuscation_:" << obfConfig_.obfuscationRules_.disableObfuscation_;
    LOG(INFO, PANDAGUARD) << TAG << "export obfuscation:" << obfConfig_.obfuscationRules_.enableExportObfuscation_;
    LOG(INFO, PANDAGUARD) << TAG << "removeLog obfuscation:" << obfConfig_.obfuscationRules_.enableRemoveLog_;
    LOG(INFO, PANDAGUARD) << TAG << "property obfuscation:" << obfConfig_.obfuscationRules_.propertyOption_.enable_;
    LOG(INFO, PANDAGUARD) << TAG << "topLevel obfuscation:" << obfConfig_.obfuscationRules_.toplevelOption_.enable_;
    LOG(INFO, PANDAGUARD) << TAG << "fileName obfuscation:" << obfConfig_.obfuscationRules_.fileNameOption_.enable_;
}

const std::string &panda::guard::GuardOptions::GetAbcFilePath() const
{
    return obfConfig_.abcFilePath_;
}

const std::string &panda::guard::GuardOptions::GetObfAbcFilePath() const
{
    return obfConfig_.obfAbcFilePath_;
}

const std::string &panda::guard::GuardOptions::GetObfPaFilePath() const
{
    return obfConfig_.obfPaFilePath_;
}

const std::string &panda::guard::GuardOptions::GetCompileSdkVersion() const
{
    return obfConfig_.compileSdkVersion_;
}

uint8_t panda::guard::GuardOptions::GetTargetApiVersion() const
{
    return obfConfig_.targetApiVersion_;
}

const std::string &panda::guard::GuardOptions::GetTargetApiSubVersion() const
{
    return obfConfig_.targetApiSubVersion_;
}

const std::string &panda::guard::GuardOptions::GetEntryPackageInfo() const
{
    return obfConfig_.entryPackageInfo_;
}

const std::string &panda::guard::GuardOptions::GetDefaultNameCachePath() const
{
    return obfConfig_.defaultNameCachePath_;
}

bool panda::guard::GuardOptions::DisableObfuscation() const
{
    return obfConfig_.obfuscationRules_.disableObfuscation_;
}

bool panda::guard::GuardOptions::EnableExport() const
{
    return obfConfig_.obfuscationRules_.enableExportObfuscation_;
}

bool panda::guard::GuardOptions::EnableRemoveLog() const
{
    return obfConfig_.obfuscationRules_.enableRemoveLog_;
}

const std::string &panda::guard::GuardOptions::GetPrintNameCache() const
{
    return obfConfig_.obfuscationRules_.printNameCache_;
}

const std::string &panda::guard::GuardOptions::GetApplyNameCache() const
{
    return obfConfig_.obfuscationRules_.applyNameCache_;
}

bool panda::guard::GuardOptions::EnableProperty() const
{
    return obfConfig_.obfuscationRules_.propertyOption_.enable_;
}

bool panda::guard::GuardOptions::EnableToplevel() const
{
    return obfConfig_.obfuscationRules_.toplevelOption_.enable_;
}

bool panda::guard::GuardOptions::EnableFileName() const
{
    return obfConfig_.obfuscationRules_.fileNameOption_.enable_;
}

bool panda::guard::GuardOptions::IsKeepPath(const std::string &path) const
{
    const auto keepOption = &obfConfig_.obfuscationRules_.keepOption_;
    if (!keepOption->enable_ || path.empty()) {
        return false;
    }

    std::vector<std::string> universalKeepPaths;
    return NeedToBeReserved(keepOption->keepPaths_, universalKeepPaths, path);
}

bool panda::guard::GuardOptions::IsReservedNames(const std::string &name) const
{
    std::vector<std::string> universalReservedNames;
    return NeedToBeReserved(obfConfig_.obfuscationRules_.reservedNames_, universalReservedNames, name);
}

bool panda::guard::GuardOptions::IsReservedProperties(const std::string &name) const
{
    return NeedToBeReserved(obfConfig_.obfuscationRules_.propertyOption_.reservedList_,
                            obfConfig_.obfuscationRules_.propertyOption_.universalReservedList_, name);
}

bool panda::guard::GuardOptions::IsReservedToplevelNames(const std::string &name) const
{
    return NeedToBeReserved(obfConfig_.obfuscationRules_.toplevelOption_.reservedList_,
                            obfConfig_.obfuscationRules_.toplevelOption_.universalReservedList_, name);
}

bool panda::guard::GuardOptions::IsReservedFileNames(const std::string &name) const
{
    return NeedToBeReserved(obfConfig_.obfuscationRules_.fileNameOption_.reservedList_,
                            obfConfig_.obfuscationRules_.fileNameOption_.universalReservedList_, name);
}
