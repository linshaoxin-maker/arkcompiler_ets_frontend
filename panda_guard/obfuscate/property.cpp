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

#include "property.h"

#include "program.h"
#include "configs/guard_context.h"
#include "util/assert_util.h"

namespace {
    constexpr std::string_view TAG = "[Property]";
}

void panda::guard::Property::ExtractNames(std::set<std::string> &strings) const
{
    strings.emplace(this->name_);
}

void panda::guard::Property::Update()
{
    PANDA_GUARD_ASSERT_PRINT(
            !this->insInfo_.IsValid() || !this->nameInfo_.IsValid(),
            TAG << "get bad insInfo for ins" << this->name_);

    this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->name_);
    this->nameInfo_.ins_->ids[0] = this->obfName_;
    this->program_->prog_->strings.emplace(this->obfName_);
}
