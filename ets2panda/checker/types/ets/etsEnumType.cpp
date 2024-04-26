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

#include "etsEnumType.h"

#include "checker/ETSchecker.h"
#include "checker/ets/conversion.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/memberExpression.h"
#include "ir/ts/tsEnumMember.h"

namespace ark::es2panda::checker {

std::string EnumDescription(util::StringView name)
{
    return "#Enum#" + static_cast<std::string>(name);
}

ETSEnumType::ETSEnumType(const ObjectTypeParameters &objParams)
    : ETSObjectType(objParams.checker->Allocator(), objParams.name, objParams.assemblerName, objParams.declNode,
                    objParams.flags | ETSObjectFlags::ENUM, objParams.relation)
{
    AddTypeFlag(TypeFlag::ETS_ENUM);
    CreateLiteralTypes(objParams);
}

ETSEnumType::ETSEnumType(const ObjectTypeParameters &objParams, ir::Literal *value)
    : ETSObjectType(objParams.checker->Allocator(), objParams.name, objParams.assemblerName, objParams.declNode,
                    objParams.flags | ETSObjectFlags::ENUM, objParams.relation),
      value_(value)
{
    AddTypeFlag(TypeFlag::ETS_ENUM);
    ASSERT(value);
}

bool ETSEnumType::IsSameEnumType(const ETSEnumType *const other) const noexcept
{
    return GetDeclNode() != nullptr && other->GetDeclNode() == GetDeclNode();
}

bool ETSEnumType::IsLiteralType() const noexcept
{
    return value_ != nullptr;
}

bool ETSEnumType::IsSameEnumLiteralType(const ETSEnumType *other) const noexcept
{
    ASSERT(IsLiteralType() && IsSameEnumType(other));
    return value_ == other->value_;
}

bool ETSEnumType::AssignmentSource(TypeRelation *const relation, Type *const target)
{
    bool result = target->IsETSEnumType() && IsSameEnumType(target->AsETSEnumType());
    relation->Result(result);
    return relation->IsTrue();
}

void ETSEnumType::AssignmentTarget(TypeRelation *const relation, Type *const source)
{
    bool result = source->IsETSEnumType() && IsSameEnumType(source->AsETSEnumType());
    relation->Result(result);
}

void ETSEnumType::Identical(TypeRelation *const relation, Type *const other)
{
    bool result =
        other->IsETSEnumType() && IsSameEnumType(other->AsETSEnumType()) && value_ == other->AsETSEnumType()->value_;
    relation->Result(result);
}

void ETSEnumType::Cast(TypeRelation *relation, Type *target)
{
    if (target->IsIntType()) {
        relation->Result(true);
        return;
    }

    conversion::Forbidden(relation);
}

void ETSEnumType::CreateLiteralTypes(const ObjectTypeParameters &objParams)
{
#if !defined(NDEBUG)
    const int32_t argsCount = 4;
#endif
    const int32_t argIdx = 2;

    ASSERT(objParams.declNode->IsClassDefinition());

    for (auto &it : objParams.declNode->AsClassDefinition()->Body()) {
        if (!it->IsClassProperty() || !it->AsClassProperty()->Value()->IsCallExpression()) {
            // literals
            auto *arrIdent = it->AsClassProperty()->Id();
            auto *arrVar = arrIdent->Variable();
            ASSERT(arrIdent->Name() == GetLiteralArrayName() && arrVar != nullptr);

            auto *arrayType = objParams.checker->CreateETSArrayType(this);
            arrayType->SetVariable(arrVar);
            arrVar->SetTsType(arrayType);

            break;
        }

        auto *ident = it->AsClassProperty()->Id();
        auto *var = ident->Variable();
        auto &createArgs = it->AsClassProperty()->Value()->AsCallExpression()->Arguments();
        ASSERT(createArgs.size() == argsCount);
        auto *value = createArgs[argIdx];

        ir::Literal *literal = nullptr;
        if (value->IsNumberLiteral()) {
            literal = value->AsNumberLiteral();
        } else if (value->IsStringLiteral()) {
            literal = value->AsStringLiteral();
        } else {
            UNREACHABLE();
        }

        auto *enumLiteraltype = Allocator()->New<ETSEnumType>(objParams, literal);

        enumLiteraltype->SetVariable(var);
        var->SetTsType(enumLiteraltype);

        objParams.checker->GetSuperType(enumLiteraltype);
    }
}

bool ETSEnumType::IsIntEnum(ETSChecker *checker) const noexcept
{
    ASSERT(SuperType());
    ASSERT(SuperType()->SuperType());
    ASSERT(SuperType()->SuperType()->TypeArguments().size() == 1);
    return SuperType()->SuperType()->TypeArguments()[0] == checker->GetGlobalTypesHolder()->GlobalIntegerBuiltinType();
}
}  // namespace ark::es2panda::checker
