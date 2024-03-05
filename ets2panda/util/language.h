/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_DYNAMIC_LANGUAGE_H
#define ES2PANDA_DYNAMIC_LANGUAGE_H

#include <array>
#include <optional>
#include <string_view>

#include "libpandabase/macros.h"

namespace panda::es2panda {

class Language {
public:
    enum class Id {
        AS,
        JS,
        TS,
        ETS,

        COUNT
    };

    constexpr explicit Language(Id id) : id_(id) {}

    constexpr std::string_view ToString() const
    {
        if (id_ == Id::AS) {
            return "as";
        }
        if (id_ == Id::JS) {
            return "js";
        }
        if (id_ == Id::TS) {
            return "ts";
        }
        if (id_ == Id::ETS) {
            return "ets";
        }

        UNREACHABLE();
    }

    static std::optional<Language> FromString(std::string_view str)
    {
        if (str == "as") {
            return Language(Id::AS);
        }
        if (str == "js") {
            return Language(Id::JS);
        }
        if (str == "ts") {
            return Language(Id::TS);
        }
        if (str == "ets") {
            return Language(Id::ETS);
        }

        return {};
    }

    Id GetId() const
    {
        return id_;
    }

    bool IsDynamic() const
    {
        if (id_ == Id::AS) {
            return false;
        }
        if (id_ == Id::JS) {
            return true;
        }
        if (id_ == Id::TS) {
            return true;
        }
        if (id_ == Id::ETS) {
            return false;
        }

        UNREACHABLE();
    }

    bool operator==(const Language &l) const
    {
        return id_ == l.id_;
    }

    bool operator!=(const Language &l) const
    {
        return id_ != l.id_;
    }

private:
    static constexpr auto COUNT = static_cast<size_t>(Id::COUNT);

public:
    static std::array<Language, COUNT> All()
    {
        return {
            Language(Id::AS),
            Language(Id::JS),
            Language(Id::TS),
            Language(Id::ETS),

        };
    }

private:
    Id id_;
};
}  // namespace panda::es2panda

// NOLINTNEXTLINE(cert-dcl58-cpp)
namespace std {

template <>
// NOLINTNEXTLINE(altera-struct-pack-align)
struct hash<panda::es2panda::Language> {
    std::size_t operator()(panda::es2panda::Language lang) const
    {
        return std::hash<panda::es2panda::Language::Id> {}(lang.GetId());
    }
};

}  // namespace std

#endif  // ES2PANDA_DYNAMIC_LANGUAGE_H
