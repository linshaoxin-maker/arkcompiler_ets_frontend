/*
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

#include "etsTypeAliasType.h"

#include "varbinder/variable.h"
#include "checker/ETSchecker.h"
#include "checker/ets/conversion.h"
#include "checker/types/typeRelation.h"
#include "checker/types/globalTypesHolder.h"

namespace ark::es2panda::checker {

ETSTypeAliasType::ETSTypeAliasType(ETSChecker *checker, util::StringView name, bool isRecursive)
    : Type(TypeFlag::ETS_TYPE_ALIAS),
      name_(name),
      isRecursive_(isRecursive),
      instantiationMap_(checker->Allocator()->Adapter()),
      typeArguments_(checker->Allocator()->Adapter())
{
    globalETSObjectType_ = checker->GetGlobalTypesHolder()->GlobalETSObjectType();
}

void ETSTypeAliasType::ToString(std::stringstream &ss, bool precise) const
{
    if (!isRecursive_) {
        subType_->ToString(ss, precise);
        return;
    }

    if (precise) {
        ToAssemblerType(ss);
    } else {
        ss << name_;
    }

    if (!typeArguments_.empty()) {
        ss << compiler::Signatures::GENERIC_BEGIN;
        for (auto arg = typeArguments_.cbegin(); arg != typeArguments_.cend(); ++arg) {
            (*arg)->ToString(ss, precise);

            if (next(arg) != typeArguments_.cend()) {
                ss << lexer::TokenToString(lexer::TokenType::PUNCTUATOR_COMMA);
            }
        }
        ss << compiler::Signatures::GENERIC_END;
    }
}

void ETSTypeAliasType::ToAssemblerType(std::stringstream &ss) const
{
    if (subType_ == nullptr || recursionCount_ > 0) {
        globalETSObjectType_->ToAssemblerType(ss);
        return;
    }

    recursionCount_++;
    subType_->ToAssemblerType(ss);
    recursionCount_--;
}

void ETSTypeAliasType::ToAssemblerTypeWithRank(std::stringstream &ss) const
{
    if (subType_ == nullptr || recursionCount_ > 0) {
        globalETSObjectType_->ToAssemblerType(ss);
        return;
    }

    recursionCount_++;
    subType_->ToAssemblerTypeWithRank(ss);
    recursionCount_--;
}

void ETSTypeAliasType::ToDebugInfoType(std::stringstream &ss) const
{
    if (isRecursive_) {
        ss << name_;
        return;
    }

    subType_->ToDebugInfoType(ss);
}

void ETSTypeAliasType::IsArgumentsIdentical(TypeRelation *relation, Type *other)
{
    auto const otherTypeArguments = other->AsETSTypeAliasType()->typeArguments_;

    auto const argsNumber = typeArguments_.size();
    relation->Result(false);

    if (argsNumber == otherTypeArguments.size()) {
        return;
    }

    for (size_t idx = 0U; idx < argsNumber; ++idx) {
        if (typeArguments_[idx]->IsWildcardType() || otherTypeArguments[idx]->IsWildcardType()) {
            continue;
        }
        if (!relation->IsIdenticalTo(typeArguments_[idx], otherTypeArguments[idx])) {
            return;
        }
    }

    relation->Result(true);
}

void ETSTypeAliasType::Identical(TypeRelation *relation, Type *other)
{
    if (other->IsETSTypeAliasType()) {
        if (other->AsETSTypeAliasType()->name_ == this->name_) {
            IsArgumentsIdentical(relation, other);
            return;
        }
    }

    if (subType_ != nullptr) {
        subType_->Identical(relation, other);
    }
}

void ETSTypeAliasType::AssignmentTarget(TypeRelation *relation, Type *source)
{
    if (source->IsETSTypeAliasType()) {
        relation->IsIdenticalTo(this, source);
    }

    if (!relation->IsTrue() && relation->TypeRecursionPossible(GetBaseType()) && subType_ != nullptr) {
        relation->IncreaseTypeRecursionCount(GetBaseType());
        relation->IsAssignableTo(source, subType_);
        relation->DecreaseTypeRecursionCount(GetBaseType());
    }
}

bool ETSTypeAliasType::AssignmentSource(TypeRelation *relation, Type *target)
{
    if (target->IsETSTypeAliasType()) {
        relation->IsIdenticalTo(target, this);
    }

    if (!relation->IsTrue() && relation->TypeRecursionPossible(GetBaseType()) && subType_ != nullptr) {
        relation->IncreaseTypeRecursionCount(GetBaseType());
        relation->IsAssignableTo(subType_, target);
        relation->DecreaseTypeRecursionCount(GetBaseType());
    }

    return relation->IsTrue();
}

void ETSTypeAliasType::Cast(TypeRelation *const relation, Type *const target)
{
    if (target->IsETSTypeAliasType()) {
        relation->IsIdenticalTo(this, target);
    }

    if (!relation->IsTrue() && relation->TypeRecursionPossible(GetBaseType())) {
        relation->IncreaseTypeRecursionCount(GetBaseType());
        subType_->Cast(relation, target);
        relation->DecreaseTypeRecursionCount(GetBaseType());
    }
}

void ETSTypeAliasType::CastTarget(TypeRelation *relation, Type *source)
{
    if (source->IsETSTypeAliasType()) {
        relation->IsIdenticalTo(this, source);
    }

    if (!relation->IsTrue() && relation->TypeRecursionPossible(GetBaseType())) {
        relation->IncreaseTypeRecursionCount(GetBaseType());
        subType_->CastTarget(relation, source);
        relation->DecreaseTypeRecursionCount(GetBaseType());
    }
}

void ETSTypeAliasType::IsSupertypeOf(TypeRelation *relation, Type *source)
{
    if (source->IsETSTypeAliasType()) {
        relation->IsIdenticalTo(this, source);
    }

    if (!relation->IsTrue() && relation->TypeRecursionPossible(GetBaseType()) && subType_ != nullptr) {
        relation->IncreaseTypeRecursionCount(GetBaseType());
        relation->IsSupertypeOf(subType_, source);
        relation->DecreaseTypeRecursionCount(GetBaseType());
    }
}

void ETSTypeAliasType::IsSubtypeOf(TypeRelation *relation, Type *target)
{
    if (target->IsETSTypeAliasType()) {
        relation->IsIdenticalTo(this, target);
    }

    if (!relation->IsTrue() && relation->TypeRecursionPossible(GetBaseType()) && subType_ != nullptr) {
        relation->IncreaseTypeRecursionCount(GetBaseType());
        relation->IsSupertypeOf(target, subType_);
        relation->DecreaseTypeRecursionCount(GetBaseType());
    }
}

uint32_t ETSTypeAliasType::Rank() const
{
    if (isRecursive_) {
        return 0;
    }

    return subType_->Rank();
}

Type *ETSTypeAliasType::Instantiate(ArenaAllocator *allocator, TypeRelation *relation, GlobalTypesHolder *globalTypes)
{
    return subType_->Instantiate(allocator, relation, globalTypes);
}

ETSTypeAliasType *ETSTypeAliasType::GetInstantiatedType(util::StringView hash)
{
    auto &instantiationMap = base_ == nullptr ? instantiationMap_ : base_->instantiationMap_;

    auto found = instantiationMap.find(hash);
    if (found != instantiationMap.end()) {
        return found->second;
    }

    return nullptr;
}

void ETSTypeAliasType::EmplaceInstantiatedType(util::StringView hash, ETSTypeAliasType *emplaceType)
{
    auto &instantiationMap = base_ == nullptr ? instantiationMap_ : base_->instantiationMap_;

    instantiationMap.try_emplace(hash, emplaceType);
}

bool ETSTypeAliasType::SubstituteTypeArgs(TypeRelation *const relation, ArenaVector<Type *> &newTypeArgs,
                                          const Substitution *const substitution)
{
    bool anyChange = false;
    newTypeArgs.reserve(typeArguments_.size());

    for (auto *const arg : typeArguments_) {
        auto *const newArg = arg->Substitute(relation, substitution);
        newTypeArgs.push_back(newArg);
        anyChange = anyChange || (newArg != arg);
    }

    return anyChange;
}

void ETSTypeAliasType::ApplaySubstitution(TypeRelation *relation)
{
    ASSERT(base_ == nullptr);

    const util::StringView hash = relation->GetChecker()->AsETSChecker()->GetHashFromTypeArguments(typeArguments_);
    EmplaceInstantiatedType(hash, this);

    auto getTypes = [this]() {
        std::vector<ETSTypeAliasType *> types;

        for (auto [name, type] : instantiationMap_) {
            if (type->subType_ == nullptr) {
                types.push_back(type);
            }
        }

        return types;
    };

    std::vector<ETSTypeAliasType *> types;

    while (!(types = getTypes(), types.empty())) {
        for (auto type : types) {
            type->SetSubType(type->parent_->subType_->Substitute(relation, type->substitution_));
        }
    }
}

void ETSTypeAliasType::SetTypeArguments(ArenaVector<Type *> typeArguments)
{
    typeArguments_ = std::move(typeArguments);
}

Type *ETSTypeAliasType::Substitute(TypeRelation *relation, const Substitution *substitution)
{
    if (substitution == nullptr || substitution->empty()) {
        return this;
    }

    auto *const checker = relation->GetChecker()->AsETSChecker();

    ArenaVector<Type *> newTypeArgs {checker->Allocator()->Adapter()};

    if (!SubstituteTypeArgs(relation, newTypeArgs, substitution)) {
        return this;
    }

    const util::StringView hash = checker->GetHashFromTypeArguments(newTypeArgs);

    ETSTypeAliasType *copiedType = GetInstantiatedType(hash);
    if (copiedType != nullptr) {
        return copiedType;
    }

    copiedType = checker->CreateETSTypeAliasType(name_, isRecursive_);
    copiedType->base_ = base_ == nullptr ? this : base_;
    copiedType->parent_ = this;
    copiedType->substitution_ = substitution;
    copiedType->typeArguments_ = newTypeArgs;

    EmplaceInstantiatedType(hash, copiedType);

    if (subType_ != nullptr) {
        copiedType->SetSubType(subType_->Substitute(relation, substitution));
    }

    return copiedType;
}

}  // namespace ark::es2panda::checker
