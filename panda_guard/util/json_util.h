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

#ifndef PANDA_GUARD_UTIL_JSON_UTIL_H
#define PANDA_GUARD_UTIL_JSON_UTIL_H

#include <string>
#include <regex>
#include <map>

#include "utils/json_parser.h"

namespace panda::guard {
    class JsonUtil {
    public:
        static JsonObject *GetJsonObject(const JsonObject *obj, const std::string &key);

        static std::string GetStringValue(const JsonObject *obj, const std::string &key);

        static double GetDoubleValue(const JsonObject *obj, const std::string &key);

        static bool GetBoolValue(const JsonObject *obj, const std::string &key);

        static std::vector<std::string> GetArrayStringValue(const JsonObject *obj, const std::string &key);

        static std::map<std::string, std::string> GetMapStringValue(const JsonObject *obj, const std::string &key);
    };
}

#endif //PANDA_GUARD_UTIL_JSON_UTIL_H
