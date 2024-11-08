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

#include "object.h"

#include "utils/logger.h"

#include "program.h"
#include "util/assert_util.h"
#include "util/string_util.h"
#include "configs/guard_context.h"

namespace {
    constexpr std::string_view TAG = "[Object]";
    constexpr size_t LITERAL_OBJECT_COMMON_ITEM_GROUP_LEN = 4; // 每4个元素表示一组普通key-value
    constexpr size_t LITERAL_OBJECT_METHOD_ITEM_GROUP_LEN = 6; // 每6个元素表示一组方法key-value
    constexpr size_t MAX_EXPORT_ITEM_LEN = 10000;
}

void panda::guard::ObjectProperty::ExtractNames(std::set<std::string> &strings) const
{
    strings.emplace(this->name_);
}

void panda::guard::ObjectProperty::Build()
{
    if (this->method_ == nullptr) {
        return;
    }

    this->method_->nameNeedUpdate_ = this->needUpdate;
    this->method_->Init();
    this->method_->Create();
}

void panda::guard::ObjectProperty::Update()
{
    auto &literalArrayTable = this->program_->prog_->literalarray_table;
    PANDA_GUARD_ASSERT_PRINT(
            literalArrayTable.find(this->literalArrayIdx_) == literalArrayTable.end(),
            TAG << "get bad literalArrayIdx:" << this->literalArrayIdx_);

    auto &literalArray = literalArrayTable.at(this->literalArrayIdx_);
    this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->name_);
    LOG(INFO, PANDAGUARD) << TAG << "obfName:" << this->obfName_;
    literalArray.literals_[this->index_].value_ = this->obfName_;

    if (this->method_) {
        this->method_->Obfuscate();
        literalArray.literals_[this->index_ + 2].value_ = this->method_->obfIdx_;
    }
}

void panda::guard::Object::Build()
{
    LOG(INFO, PANDAGUARD) << TAG << "object create for " << this->literalArrayIdx_ << " start";

    const auto &parentFunc = this->program_->prog_->function_table.at(this->insInfo_.function_->idx_);
    const size_t nextInsIndex = this->insInfo_.index_ + 1;
    PANDA_GUARD_ASSERT_PRINT(
            nextInsIndex >= parentFunc.ins.size(),
            TAG << "try to find next ins of createobjectwithbuffer get bad ins index:" << nextInsIndex);

    const auto &ins = parentFunc.ins[nextInsIndex];
    this->export_ = ins.opcode == pandasm::Opcode::STMODULEVAR; // 下一条指令为stmodulevar则为导出object

    LOG(INFO, PANDAGUARD) << TAG << "export:" << (this->export_ ? "true" : "false");
    if (this->export_) {
        int64_t index = std::get<int64_t>(ins.imms[0]);
        PANDA_GUARD_ASSERT_PRINT(index < 0 || index > MAX_EXPORT_ITEM_LEN, "unexpect export item index:" << index);
        this->name_ = this->moduleRecord->GetLocalExportName(index);
        this->obfName_ = this->name_;
        this->SetNameCacheScope(this->name_);
        LOG(INFO, PANDAGUARD) << TAG << "name:" << this->name_;
    }

    const auto &literalArray = this->program_->prog_->literalarray_table.at(this->literalArrayIdx_);
    size_t keyIndex = 1; // object item key index
    size_t valueIndex = keyIndex + 2; // object item value index
    while (valueIndex < literalArray.literals_.size()) {
        auto &valueLiteral = literalArray.literals_[valueIndex];
        bool isMethod = valueLiteral.tag_ == panda_file::LiteralTag::METHOD;
        CreateProperty(literalArray, keyIndex, isMethod);
        if (isMethod) {
            keyIndex += LITERAL_OBJECT_METHOD_ITEM_GROUP_LEN;
        } else {
            keyIndex += LITERAL_OBJECT_COMMON_ITEM_GROUP_LEN;
        }
        valueIndex = keyIndex + 2;
    }
    LOG(INFO, PANDAGUARD) << TAG << "object create for " << this->literalArrayIdx_ << " end";
}

void panda::guard::Object::CreateProperty(const pandasm::LiteralArray &literalArray, size_t index, bool isMethod)
{
    const auto &[keyTag, keyValue] = literalArray.literals_[index];
    PANDA_GUARD_ASSERT_PRINT(keyTag != panda_file::LiteralTag::STRING, TAG << "bad keyTag literal tag");

    ObjectProperty property(this->program_, this->literalArrayIdx_);
    property.name_ = StringUtil::UnicodeEscape(std::get<std::string>(keyValue));
    property.scope_ = this->scope_;
    property.export_ = this->export_;
    property.index_ = index;

    if (isMethod) {
        size_t valueLiteralIndex = index + 2;
        PANDA_GUARD_ASSERT_PRINT(
                valueLiteralIndex >= literalArray.literals_.size(), "bad valueLiteralIndex:" << valueLiteralIndex);
        const auto &[valueTag, valueValue] = literalArray.literals_[valueLiteralIndex];
        PANDA_GUARD_ASSERT_PRINT(
                valueTag != panda_file::LiteralTag::METHOD, "bad valueLiteral tag:" << (int) valueTag);
        property.method_ = std::make_shared<PropertyMethod>(this->program_, std::get<std::string>(valueValue));
        property.method_->export_ = this->export_;
        property.method_->scope_ = this->scope_;
    }

    property.Create();

    LOG(INFO, PANDAGUARD) << TAG << "find object property:" << property.name_;

    this->properties_.push_back(property);
}

void panda::guard::Object::ForEachMethod(const std::function<FunctionTraver> &callback)
{
    for (const auto &property: this->properties_) {
        if (property.method_) {
            callback(property.method_.operator*());
        }
    }

    for (auto &method: this->outerMethods_) {
        callback(method);
    }
}

void panda::guard::Object::ExtractNames(std::set<std::string> &strings) const
{
    for (const auto &property: this->properties_) {
        property.ExtractNames(strings);
    }
}

void panda::guard::Object::RefreshNeedUpdate()
{
    this->needUpdate = true;
    this->needUpdateName_ = TopLevelOptionEntity::NeedUpdate(*this);
}

void panda::guard::Object::Update()
{
    LOG(INFO, PANDAGUARD) << TAG << "object update for " << this->literalArrayIdx_ << " start";

    if (this->needUpdateName_ && !this->name_.empty()) {
        this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->name_);
    }

    for (auto &property: this->properties_) {
        property.Obfuscate();
    }

    for (auto &method: this->outerMethods_) {
        method.Obfuscate();
    }

    LOG(INFO, PANDAGUARD) << TAG << "object update for " << this->literalArrayIdx_ << " end";
}

void panda::guard::Object::WriteNameCache(const std::string &filePath)
{
    if (!this->obfuscated) {
        return;
    }

    if (this->needUpdateName_ && !this->obfName_.empty()) {
        GuardContext::GetInstance()->GetNameCache()->AddObfIdentifierName(filePath, this->GetNameCacheScope(),
                                                                          this->obfName_);
    }

    for (auto &property: this->properties_) {
        property.WriteNameCache(filePath);
    }
}