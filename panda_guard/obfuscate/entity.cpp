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

#include "entity.h"
#include "configs/guard_context.h"
#include "program.h"
#include "file_path.h"

namespace {
    std::string_view TAG = "[Entity]";
    std::string_view SCOPE_DELIMITER = "#";
}

void panda::guard::Entity::Create()
{
    this->Build();
    this->RefreshNeedUpdate();

    LOG(INFO, PANDAGUARD) << TAG << "needUpdate:" << (this->needUpdate ? "true" : "false");
}

void panda::guard::Entity::Obfuscate()
{
    if (!this->needUpdate) {
        return;
    }

    this->Update();

    this->obfuscated = true;
}

void panda::guard::Entity::Build()
{
}

void panda::guard::Entity::RefreshNeedUpdate()
{
}

void panda::guard::Entity::Update()
{
}

void panda::guard::Entity::WriteNameCache(const std::string &filePath)
{
    this->WriteFileCache(filePath);
    this->WritePropertyCache();
}

void panda::guard::Entity::SetNameCacheScope(const std::string &name)
{
    if (this->scope_ == TOP_LEVEL) {
        this->nameCacheScope_ = name;
    } else {
        if (this->insInfo_.function_) {
            this->nameCacheScope_ = this->insInfo_.function_->nameCacheScope_ + SCOPE_DELIMITER.data() + name;
        }
    }
}

std::string panda::guard::Entity::GetNameCacheScope() const
{
    return this->scope_ == TOP_LEVEL ? SCOPE_DELIMITER.data() + this->nameCacheScope_ : this->nameCacheScope_;
}

bool panda::guard::Entity::IsExport() const
{
    if (this->scope_ != TOP_LEVEL) {
        return false;
    }

    return this->export_;
}

void panda::guard::Entity::WriteFileCache(const std::string &filePath)
{
}

void panda::guard::Entity::WritePropertyCache()
{
}

bool panda::guard::TopLevelOptionEntity::NeedUpdate(const Entity &entity)
{
    bool needUpdate = true;
    do {
        const auto options = GuardContext::GetInstance()->GetGuardOptions();
        if (entity.IsExport()) {
            needUpdate = options->EnableExport() && options->EnableToplevel();
            break;
        }

        if (entity.scope_ == TOP_LEVEL) {
            needUpdate = options->EnableToplevel();
            break;
        }
    } while (false);

    if (!needUpdate) {
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(entity.name_);
    }

    return needUpdate;
}

void panda::guard::TopLevelOptionEntity::RefreshNeedUpdate()
{
    this->needUpdate = NeedUpdate(*this);
}

void panda::guard::TopLevelOptionEntity::WritePropertyCache(const panda::guard::Entity &entity)
{
    if (!entity.obfuscated || entity.name_.empty() || entity.obfName_.empty() || (entity.scope_ != TOP_LEVEL)) {
        return;
    }

    const auto options = GuardContext::GetInstance()->GetGuardOptions();
    if (!options->EnableProperty() || !options->EnableToplevel()) {
        return;
    }

    if (entity.IsExport()) {
        if (options->EnableExport()) {
            GuardContext::GetInstance()->GetNameCache()->AddObfPropertyName(entity.name_, entity.obfName_);
        }
    } else {
        GuardContext::GetInstance()->GetNameCache()->AddObfPropertyName(entity.name_, entity.obfName_);
    }
}

void panda::guard::TopLevelOptionEntity::WritePropertyCache()
{
    return WritePropertyCache(*this);
}

bool panda::guard::PropertyOptionEntity::NeedUpdate(const Entity &entity)
{
    const auto options = GuardContext::GetInstance()->GetGuardOptions();
    bool needUpdate;
    if (entity.IsExport()) {
        needUpdate = options->EnableExport() && options->EnableProperty();
    } else {
        needUpdate = options->EnableProperty();
    }
    if (!needUpdate) {
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(entity.name_);
    }
    return needUpdate;
}

void panda::guard::PropertyOptionEntity::RefreshNeedUpdate()
{
    this->needUpdate = NeedUpdate(*this);
}

void panda::guard::PropertyOptionEntity::WritePropertyCache(const panda::guard::Entity &entity)
{
    if (!entity.obfuscated || entity.name_.empty() || entity.obfName_.empty()) {
        return;
    }

    const auto options = GuardContext::GetInstance()->GetGuardOptions();
    if (!options->EnableProperty()) {
        return;
    }

    if (entity.IsExport()) {
        if (options->EnableExport()) {
            GuardContext::GetInstance()->GetNameCache()->AddObfPropertyName(entity.name_, entity.obfName_);
        }
    } else {
        GuardContext::GetInstance()->GetNameCache()->AddObfPropertyName(entity.name_, entity.obfName_);
    }
}

void panda::guard::PropertyOptionEntity::WritePropertyCache()
{
    return WritePropertyCache(*this);
}

bool panda::guard::InstructionInfo::IsInnerReg() const
{
    return this->ins_->regs[0] < this->function_->regsNum;
}

void panda::guard::InstructionInfo::UpdateInsName(bool generateNewName)
{
    this->origin_ = this->ins_->ids[0];
    this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->origin_, generateNewName);
    this->ins_->ids[0] = this->obfName_;
    this->function_->program_->prog_->strings.emplace(this->obfName_);
}

void panda::guard::InstructionInfo::UpdateInsFileName()
{
    this->origin_ = this->ins_->ids[0];
    ReferenceFilePath filePath(this->function_->program_);
    filePath.SetFilePath(this->origin_);
    filePath.Init();
    filePath.Update();
    this->obfName_ = filePath.obfFilePath_;
    this->ins_->ids[0] = this->obfName_;
    this->function_->program_->prog_->strings.emplace(this->obfName_);
}

void panda::guard::InstructionInfo::WriteNameCache()
{
    GuardContext::GetInstance()->GetNameCache()->AddObfPropertyName(this->origin_, this->obfName_);
}

bool panda::guard::InstructionInfo::IsValid() const
{
    return this->ins_ != nullptr;
}
