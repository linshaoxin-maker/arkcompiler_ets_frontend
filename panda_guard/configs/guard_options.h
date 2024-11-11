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

#ifndef PANDA_GUARD_CONFIGS_GUARD_OPTIONS_H
#define PANDA_GUARD_CONFIGS_GUARD_OPTIONS_H

#include <string>
#include <vector>

#include "utils/json_parser.h"

namespace panda::guard {
    struct ObfuscationOption {
        bool enable = false;
        std::vector<std::string> reservedList;
        std::vector<std::string> universalReservedList;
    };

    struct KeepOption {
        bool enable = false;
        std::vector<std::string> keepPaths;
    };

    struct ObfuscationRules {
        bool disableObfuscation = false;
        bool enableExportObfuscation = false;
        bool enableRemoveLog = false;
        std::string printNameCache;
        std::string applyNameCache;
        std::vector<std::string> reservedNames;
        ObfuscationOption propertyOption;
        ObfuscationOption toplevelOption;
        ObfuscationOption fileNameOption;
        KeepOption keepOption;
    };

    struct ObfuscationConfig {
        std::string abcFilePath;
        std::string obfAbcFilePath;
        std::string obfPaFilePath;
        std::string compileSdkVersion;
        uint8_t targetApiVersion;
        std::string targetApiSubVersion;
        std::string entryPackageInfo;
        std::string defaultNameCachePath;
        ObfuscationRules obfuscationRules;
    };

    class GuardOptions {
    public:
        void Load(const std::string &configFilePath);

        [[nodiscard]] const std::string &GetAbcFilePath() const;

        [[nodiscard]] const std::string &GetObfAbcFilePath() const;

        [[nodiscard]] const std::string &GetObfPaFilePath() const;

        [[nodiscard]] const std::string &GetCompileSdkVersion() const;

        [[nodiscard]] uint8_t GetTargetApiVersion() const;

        [[nodiscard]] const std::string &GetTargetApiSubVersion() const;

        [[nodiscard]] const std::string &GetEntryPackageInfo() const;

        [[nodiscard]] const std::string &GetDefaultNameCachePath() const;

        [[nodiscard]] const std::string &GetPrintNameCache() const;

        [[nodiscard]] const std::string &GetApplyNameCache() const;

        [[nodiscard]] bool DisableObfuscation() const;

        [[nodiscard]] bool EnableExport() const;

        [[nodiscard]] bool EnableRemoveLog() const;

        [[nodiscard]] bool EnableProperty() const;

        [[nodiscard]] bool EnableToplevel() const;

        [[nodiscard]] bool EnableFileName() const;

        [[nodiscard]] bool IsKeepPath(const std::string &path = "") const;

        [[nodiscard]] bool IsReservedNames(const std::string &name) const;

        [[nodiscard]] bool IsReservedProperties(const std::string &name) const;

        [[nodiscard]] bool IsReservedToplevelNames(const std::string &name) const;

        [[nodiscard]] bool IsReservedFileNames(const std::string &name) const;

    private:
        ObfuscationConfig obfConfig_;
    };
}

#endif //PANDA_GUARD_CONFIGS_GUARD_OPTIONS_H