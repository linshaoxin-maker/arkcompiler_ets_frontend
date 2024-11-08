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

#include "method.h"

#include "program.h"
#include "utils/logger.h"
#include "util/assert_util.h"
#include "configs/guard_context.h"

namespace {
    constexpr std::string_view TAG = "[Method]";
}

void panda::guard::Method::InitNameCacheScope()
{
    this->nameCacheScope_ = this->name_;
}

void panda::guard::Method::RefreshNeedUpdate()
{
    this->contentNeedUpdate_ = PropertyOptionEntity::NeedUpdate(*this);
    this->nameNeedUpdate_ = this->contentNeedUpdate_ && !IsWhiteListOrAnonymousFunction(this->idx_);

    LOG(INFO, PANDAGUARD) << TAG << "Method contentNeedUpdate: "
                          << (this->contentNeedUpdate_ ? "true" : "false");
    LOG(INFO, PANDAGUARD) << TAG << "Method nameNeedUpdate: "
                          << (this->nameNeedUpdate_ ? "true" : "false");
}

void panda::guard::Method::UpdateDefine() const
{
    auto &literalArrayTable = program_->prog_->literalarray_table;
    auto it = literalArrayTable.find(this->literalArrayIdx_);
    PANDA_GUARD_ASSERT_PRINT(it == literalArrayTable.end(), TAG << "get bad literalArrayIdx:" << literalArrayIdx_);

    it->second.literals_[this->idxIndex_].value_ = this->obfIdx_;
    it->second.literals_[this->nameIndex_].value_ = this->obfName_;
}

void panda::guard::Method::WriteFileCache(const std::string &filePath)
{
    GuardContext::GetInstance()->GetNameCache()->AddObfMemberMethodName(
            filePath, this->nameCacheScope_ + this->GetLines(), this->obfName_);
}

void panda::guard::Method::WritePropertyCache()
{
    PropertyOptionEntity::WritePropertyCache(*this);
}

void panda::guard::OuterMethod::InitNameCacheScope()
{
    this->nameCacheScope_ = this->name_;
}

void panda::guard::OuterMethod::RefreshNeedUpdate()
{
    this->contentNeedUpdate_ = PropertyOptionEntity::NeedUpdate(*this);
    this->nameNeedUpdate_ = this->contentNeedUpdate_ && !IsWhiteListOrAnonymousFunction(this->idx_);

    LOG(INFO, PANDAGUARD) << TAG << "OuterMethod contentNeedUpdate: "
                          << (this->contentNeedUpdate_ ? "true" : "false");
    LOG(INFO, PANDAGUARD) << TAG << "OuterMethod nameNeedUpdate: "
                          << (this->nameNeedUpdate_ ? "true" : "false");
}

void panda::guard::OuterMethod::WriteFileCache(const std::string &filePath)
{
    GuardContext::GetInstance()->GetNameCache()->AddObfMemberMethodName(
            filePath, this->nameCacheScope_ + this->GetLines(), this->obfName_);
}

void panda::guard::OuterMethod::WritePropertyCache()
{
    PropertyOptionEntity::WritePropertyCache(*this);
}

void panda::guard::PropertyMethod::RefreshNeedUpdate()
{}
