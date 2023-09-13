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

#ifndef ES2PANDA_DOCKGEN_JSON_H
#define ES2PANDA_DOCKGEN_JSON_H

#include <vector>
#include <map>
#include <variant>
#include <string>

#include "util/ustring.h"

namespace panda::es2panda::docgen {

struct JValue;

using JObject = std::map<std::string, JValue>;
using JArray = std::vector<JValue>;
using JString = std::string;

namespace detail {
using JValueVar = std::variant<std::monostate, JString, double, bool, JArray, JObject>;
}  // namespace detail

struct JValue : public detail::JValueVar {
    explicit JValue() = default;

    explicit JValue(std::monostate m) noexcept : detail::JValueVar(m) {}

    explicit JValue(double d) : detail::JValueVar(d) {}

    explicit JValue(bool b) : detail::JValueVar(b) {}

    explicit JValue(util::StringView s) : JValue(s.Utf8()) {}

    explicit JValue(const char *s) : JValue(std::string(s)) {}

    explicit JValue(std::string_view s) : JValue(std::string(s)) {}

    explicit JValue(std::string s) : detail::JValueVar(std::move(s)) {}

    explicit JValue(JArray j) : detail::JValueVar(std::move(j)) {}

    explicit JValue(JObject j) : detail::JValueVar(std::move(j)) {}

    std::string ToString() const;

private:
    void ToString(std::stringstream &out) const;

    static void DumpArray(std::stringstream &out, const JArray &arr);
    static void DumpObject(std::stringstream &out, const JObject &obj);
};

inline const JValue JNULL = JValue {std::monostate {}};  // NOLINT(fuchsia-statically-constructed-objects)

}  // namespace panda::es2panda::docgen

#endif
