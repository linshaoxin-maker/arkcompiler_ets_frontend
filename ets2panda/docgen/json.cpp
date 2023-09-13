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

#include "json.h"

#include <sstream>
#include <iomanip>

namespace panda::es2panda::docgen {

namespace {
auto DumpEscape(std::stringstream &out, std::string_view str)
{
    out << '"';
    for (auto c : str) {
        uint8_t uc = c;
        constexpr uint8_t NON_PRINTABLE = 0x1f;
        if (uc == '"' || uc == '\\' || uc <= NON_PRINTABLE) {
            constexpr int JSON_UNICODE_SIZE = 4;
            out << "\\u" << std::setw(JSON_UNICODE_SIZE) << std::setfill('0') << std::hex << static_cast<uint32_t>(uc);
        } else {
            out << c;
        }
    }
    out << '"';
}
}  // namespace

void JValue::DumpArray(std::stringstream &out, const JArray &arr)
{
    out << '[';
    bool first = true;
    for (const auto &e : arr) {
        if (!first) {
            out << ',';
        }
        first = false;
        e.ToString(out);
    }
    out << ']';
}

void JValue::DumpObject(std::stringstream &out, const JObject &obj)
{
    out << '{';
    bool first = true;
    for (const auto &[k, v] : obj) {
        if (!first) {
            out << ',';
        }
        first = false;
        DumpEscape(out, k);
        out << ":";
        v.ToString(out);
    }
    out << '}';
}

void JValue::ToString(std::stringstream &out) const
{
    std::visit(
        [&out](auto &&el) -> void {
            using T = std::decay_t<decltype(el)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                std::ignore = el;
                out << "null";
            } else if constexpr (std::is_same_v<T, bool>) {
                out << (el ? "true" : "false");
            } else if constexpr (std::is_same_v<T, double>) {
                out << el;
            } else if constexpr (std::is_same_v<T, JString>) {
                DumpEscape(out, el);
            } else if constexpr (std::is_same_v<T, JArray>) {
                DumpArray(out, el);
            } else if constexpr (std::is_same_v<T, JObject>) {
                DumpObject(out, el);
            } else {
                UNREACHABLE();
            }
        },
        *static_cast<const detail::JValueVar *>(this));
}

std::string JValue::ToString() const
{
    std::stringstream out;
    ToString(out);
    return out.str();
}

}  // namespace panda::es2panda::docgen
