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
        bool enable_ = false;
        std::vector<std::string> reservedList_;
        std::vector<std::string> universalReservedList_;
    };

    struct KeepOption {
        bool enable_ = false;
        std::vector<std::string> keepPaths_;
    };

    struct ObfuscationRules {
        bool disableObfuscation_ = false;
        bool enableExportObfuscation_ = false;
        bool enableRemoveLog_ = false;
        std::string printNameCache_;
        std::string applyNameCache_;
        std::vector<std::string> reservedNames_;
        ObfuscationOption propertyOption_;
        ObfuscationOption toplevelOption_;
        ObfuscationOption fileNameOption_;
        KeepOption keepOption_;
    };

    struct ObfuscationConfig {
        std::string abcFilePath_;
        std::string obfAbcFilePath_;
        std::string obfPaFilePath_;
        std::string compileSdkVersion_;
        uint8_t targetApiVersion_;
        std::string targetApiSubVersion_;
        std::string entryPackageInfo_;
        std::string defaultNameCachePath_;
        ObfuscationRules obfuscationRules_;
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