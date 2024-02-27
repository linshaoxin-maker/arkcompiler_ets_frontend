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
#include "ir/expressions/literals/numberLiteral.h"
#include "ir/expressions/memberExpression.h"
#include "ir/ts/tsEnumMember.h"

namespace ark::es2panda::checker {

std::string EnumDescription(util::StringView name)
{
    return "#Enum#" + static_cast<std::string>(name);
}

ETSEnum2Type::ETSEnum2Type(ETSChecker *checker, util::StringView name, util::StringView assembler_name,
                           ir::AstNode *decl_node, ETSObjectFlags flags, TypeRelation *relation)
    : ETSObjectType(checker->Allocator(), name, assembler_name, decl_node, flags | ETSObjectFlags::ENUM2, relation)
{
    AddTypeFlag(TypeFlag::ETS_ENUM2);
    CreateLiteralTypes(checker, name, assembler_name, decl_node, flags, relation);
}

ETSEnum2Type::ETSEnum2Type(ArenaAllocator *allocator, util::StringView name, util::StringView assembler_name,
                           ir::AstNode *decl_node, ETSObjectFlags flags, ir::Literal *value, TypeRelation *relation)
    : ETSObjectType(allocator, name, assembler_name, decl_node, flags | ETSObjectFlags::ENUM2, relation), value_(value)
{
    AddTypeFlag(TypeFlag::ETS_ENUM2);
    ASSERT(value);
}

bool ETSEnum2Type::IsSameEnumType(const ETSEnum2Type *const other) const noexcept
{
    return GetDeclNode() != nullptr && other->GetDeclNode() == GetDeclNode();
}

bool ETSEnum2Type::IsLiteralType() const noexcept
{
    return value_ != nullptr;
}

bool ETSEnum2Type::IsSameEnumLiteralType(const ETSEnum2Type *other) const noexcept
{
    ASSERT(IsLiteralType() && IsSameEnumType(other));
    return value_ == other->value_;
}

bool ETSEnum2Type::AssignmentSource(TypeRelation *const relation, Type *const target)
{
    bool result = target->IsETSEnum2Type() && IsSameEnumType(target->AsETSEnum2Type());
    relation->Result(result);
    return relation->IsTrue();
}

void ETSEnum2Type::AssignmentTarget(TypeRelation *const relation, Type *const source)
{
    bool result = source->IsETSEnum2Type() && IsSameEnumType(source->AsETSEnum2Type());
    relation->Result(result);
}

void ETSEnum2Type::Identical(TypeRelation *const relation, Type *const other)
{
    bool result =
        other->IsETSEnum2Type() && IsSameEnumType(other->AsETSEnum2Type()) && value_ == other->AsETSEnum2Type()->value_;
    relation->Result(result);
}

void ETSEnum2Type::Cast(TypeRelation *relation, Type *target)
{
    if (target->IsIntType()) {
        relation->Result(true);
        return;
    }

    conversion::Forbidden(relation);
}

void ETSEnum2Type::CreateLiteralTypes(ETSChecker *checker, util::StringView name, util::StringView assembler_name,
                                      ir::AstNode *decl_node, ETSObjectFlags flags, TypeRelation *relation)
{
    ASSERT(decl_node->IsClassDefinition());

    for (auto &it : decl_node->AsClassDefinition()->Body()) {
        if (!it->IsClassProperty() || !it->AsClassProperty()->Value()->IsCallExpression()) {
            // arr property
            auto *arr_ident = it->AsClassProperty()->Id();
            auto *arr_var = arr_ident->Variable();
            ASSERT(arr_ident->Name() == "arr" && arr_var != nullptr);

            auto *array_type = checker->CreateETSArrayType(this);
            array_type->SetVariable(arr_var);
            arr_var->SetTsType(array_type);

            break;
        }

        // if (decl_node->IsClassDefinition()) continue;

        auto *ident = it->AsClassProperty()->Id();
        auto *var = ident->Variable();
        auto &create_args = it->AsClassProperty()->Value()->AsCallExpression()->Arguments();
        ASSERT(create_args.size() == 4);
        auto *value = create_args[2];

        ir::Literal *literal = nullptr;
        if (value->IsNumberLiteral()) {
            literal = value->AsNumberLiteral();
        } else if (value->IsStringLiteral()) {
            literal = value->AsStringLiteral();
        } else {
            UNREACHABLE();
        }

        auto *enum_literal_type = Allocator()->New<ETSEnum2Type>(checker->Allocator(), name, assembler_name, decl_node,
                                                                 flags, literal, relation);

        enum_literal_type->SetVariable(var);
        var->SetTsType(enum_literal_type);

        checker->GetSuperType(enum_literal_type);
    }
}
}  // namespace ark::es2panda::checker
