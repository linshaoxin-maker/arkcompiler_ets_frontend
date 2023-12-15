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
ETSEnumInterface::ETSEnumInterface(const ir::TSEnumDeclaration *const enumDecl, UType ordinal,
                                   const ir::TSEnumMember *const member, TypeFlag const typeFlag)
    : Type(typeFlag), decl_(enumDecl), ordinal_ {ordinal}, member_(member)
{
}

bool ETSEnumInterface::AssignmentSource(TypeRelation *const relation, Type *const target)
{
    auto const result = target->IsETSEnumType()
                            ? IsSameEnumType(target->AsETSEnumType())
                            : (target->IsETSStringEnumType() ? IsSameEnumType(target->AsETSStringEnumType()) : false);
    relation->Result(result);
    return relation->IsTrue();
}

void ETSEnumInterface::AssignmentTarget(TypeRelation *const relation, Type *const source)
{
    auto const result = source->IsETSEnumType()
                            ? IsSameEnumType(source->AsETSEnumType())
                            : (source->IsETSStringEnumType() ? IsSameEnumType(source->AsETSStringEnumType()) : false);
    relation->Result(result);
}

void ETSEnumInterface::Cast(TypeRelation *relation, Type *target)
{
    if (target->HasTypeFlag(TypeFlag::ENUM | TypeFlag::ETS_ENUM | TypeFlag::ETS_STRING_ENUM)) {
        conversion::Identity(relation, this, target);
        return;
    }

    if (target->IsIntType()) {
        relation->Result(true);
        return;
    }

    conversion::Forbidden(relation);
}

Type *ETSEnumInterface::Instantiate([[maybe_unused]] ArenaAllocator *allocator, [[maybe_unused]] TypeRelation *relation,
                                    [[maybe_unused]] GlobalTypesHolder *globalTypes)
{
    return this;
}

void ETSEnumInterface::Identical(TypeRelation *const relation, Type *const other)
{
    ETSEnumInterface const *const otherEnumType = [other]() -> ETSEnumInterface const * {
        if (other->IsETSEnumType()) {
            return other->AsETSEnumType();
        }
        if (other->IsETSStringEnumType()) {
            return other->AsETSStringEnumType();
        }
        return nullptr;
    }();

    relation->Result(otherEnumType != nullptr && IsSameEnumType(otherEnumType) && member_ == otherEnumType->member_);
}

void ETSEnumInterface::ToAssemblerType(std::stringstream &ss) const
{
    ToAssemblerTypeImpl<UType>(ss);
}

void ETSEnumInterface::ToDebugInfoType(std::stringstream &ss) const
{
    ToDebugInfoTypeImpl<UType>(ss);
}

void ETSEnumInterface::ToString(std::stringstream &ss, [[maybe_unused]] bool precise) const
{
    ss << decl_->Key()->Name();
}

const ir::TSEnumDeclaration *ETSEnumInterface::GetDecl() const noexcept
{
    return decl_;
}

const ArenaVector<ir::AstNode *> &ETSEnumInterface::GetMembers() const noexcept
{
    return decl_->Members();
}

varbinder::LocalVariable *ETSEnumInterface::GetMemberVar() const noexcept
{
    ASSERT(IsLiteralType());
    return member_->Key()->AsIdentifier()->Variable()->AsLocalVariable();
}

util::StringView ETSEnumInterface::GetName() const noexcept
{
    return decl_->Key()->Name();
}

ETSEnumInterface::UType ETSEnumInterface::GetOrdinal() const noexcept
{
    ASSERT(IsLiteralType());
    return ordinal_;
}

ETSEnumInterface *ETSEnumInterface::LookupConstant(ETSChecker *const checker, const ir::Expression *const expression,
                                                   const ir::Identifier *const prop) const
{
    if (!IsEnumTypeExpression(expression)) {
        if (expression->IsIdentifier() &&
            expression->AsIdentifier()->Variable()->HasFlag(varbinder::VariableFlags::TYPE_ALIAS)) {
            checker->ThrowTypeError({"Cannot refer to enum members through type alias."}, prop->Start());
        } else if (IsLiteralType()) {
            checker->ThrowTypeError({"Cannot refer to enum members through variable."}, prop->Start());
        } else {
            checker->ThrowTypeError({"Enum constant does not have property '", prop->Name(), "'."}, prop->Start());
        }
    }

    auto *const member = FindMember(prop->Name());
    if (member == nullptr) {
        checker->ThrowTypeError({"No enum constant named '", prop->Name(), "' in enum '", this, "'"}, prop->Start());
    }

    auto *const enumInterface =
        [enumType = member->Key()->AsIdentifier()->Variable()->TsType()]() -> checker::ETSEnumInterface * {
        if (enumType->IsETSEnumType()) {
            return enumType->AsETSEnumType();
        }
        return enumType->AsETSStringEnumType();
    }();

    ASSERT(enumInterface->IsLiteralType());
    return enumInterface;
}

ETSFunctionType *ETSEnumInterface::LookupMethod(ETSChecker *checker, const ir::Expression *const expression,
                                                const ir::Identifier *const prop) const
{
    if (IsEnumTypeExpression(expression)) {
        return LookupTypeMethod(checker, prop);
    }

    ASSERT(IsEnumInstanceExpression(expression));
    return LookupConstantMethod(checker, prop);
}

bool ETSEnumInterface::IsSameEnumType(const ETSEnumInterface *const other) const noexcept
{
    return other->decl_ == decl_;
}

bool ETSEnumInterface::IsSameEnumLiteralType(const ETSEnumInterface *const other) const noexcept
{
    ASSERT(IsLiteralType() && IsSameEnumType(other));
    return member_ == other->member_;
}

[[maybe_unused]] static const ETSEnumInterface *SpecifyEnumInterface(const checker::Type *enumType)
{
    if (enumType->IsETSEnumType()) {
        return enumType->AsETSEnumType();
    }
    if (enumType->IsETSStringEnumType()) {
        return enumType->AsETSStringEnumType();
    }
    return nullptr;
}

bool ETSEnumInterface::IsEnumInstanceExpression(const ir::Expression *const expression) const noexcept
{
    ASSERT(IsSameEnumType(SpecifyEnumInterface(expression->TsType())));

    return IsEnumLiteralExpression(expression) || !IsEnumTypeExpression(expression);
}

bool ETSEnumInterface::IsEnumLiteralExpression(const ir::Expression *const expression) const noexcept
{
    ASSERT(IsSameEnumType(SpecifyEnumInterface(expression->TsType())));

    if (expression->IsMemberExpression()) {
        const auto *const memberExpr = expression->AsMemberExpression();
        return memberExpr->Kind() == ir::MemberExpressionKind::PROPERTY_ACCESS &&
               IsEnumTypeExpression(memberExpr->Object());
    }

    return false;
}

bool ETSEnumInterface::IsEnumTypeExpression(const ir::Expression *const expression) const noexcept
{
    auto specifiedEnumInterface = SpecifyEnumInterface(expression->TsType());
    if (specifiedEnumInterface != nullptr) {
        ASSERT(IsSameEnumType(specifiedEnumInterface));
    } else {
        return false;
    }

    if (expression->IsCallExpression()) {
        return false;
    }

    const auto *const localVar = [expression]() -> const varbinder::LocalVariable * {
        if (expression->IsMemberExpression()) {
            const auto *const memberExpr = expression->AsMemberExpression();
            return memberExpr->PropVar() != nullptr
                       ? memberExpr->PropVar()
                       : memberExpr->Object()->AsIdentifier()->Variable()->AsLocalVariable();
        }
        return expression->AsIdentifier()->Variable()->AsLocalVariable();
    }();

    ASSERT(localVar->Declaration() == decl_->Key()->AsIdentifier()->Variable()->Declaration() ||
           !localVar->HasFlag(varbinder::VariableFlags::ENUM_LITERAL));
    return localVar->HasFlag(varbinder::VariableFlags::ENUM_LITERAL);
}

ETSEnumInterface::Method ETSEnumInterface::FromIntMethod() const noexcept
{
    ASSERT(fromIntMethod_.globalSignature != nullptr && fromIntMethod_.memberProxyType == nullptr);
    return fromIntMethod_;
}

ETSEnumInterface::Method ETSEnumInterface::GetValueMethod() const noexcept
{
    ASSERT(getValueMethod_.globalSignature != nullptr && getValueMethod_.memberProxyType != nullptr);
    return getValueMethod_;
}

ETSEnumInterface::Method ETSEnumInterface::GetNameMethod() const noexcept
{
    ASSERT(getNameMethod_.globalSignature != nullptr && getNameMethod_.memberProxyType != nullptr);
    return getNameMethod_;
}

ETSEnumInterface::Method ETSEnumInterface::ToStringMethod() const noexcept
{
    ASSERT(toStringMethod_.globalSignature != nullptr && toStringMethod_.memberProxyType != nullptr);
    return toStringMethod_;
}

ETSEnumInterface::Method ETSEnumInterface::ValueOfMethod() const noexcept
{
    ASSERT(valueOfMethod_.globalSignature != nullptr && valueOfMethod_.memberProxyType != nullptr);
    return valueOfMethod_;
}

ETSEnumInterface::Method ETSEnumInterface::ValuesMethod() const noexcept
{
    ASSERT(valuesMethod_.globalSignature != nullptr && valuesMethod_.memberProxyType != nullptr);
    return valuesMethod_;
}

bool ETSEnumInterface::IsLiteralType() const noexcept
{
    return member_ != nullptr;
}

ir::TSEnumMember *ETSEnumInterface::FindMember(const util::StringView &name) const noexcept
{
    ASSERT(!IsLiteralType());
    const auto &members = GetMembers();
    auto memberIt = std::find_if(members.begin(), members.end(), [name](const ir::AstNode *const node) {
        return node->AsTSEnumMember()->Key()->AsIdentifier()->Name() == name;
    });

    if (memberIt != members.end()) {
        return (*memberIt)->AsTSEnumMember();
    }

    return nullptr;
}

ETSFunctionType *ETSEnumInterface::LookupConstantMethod(ETSChecker *const checker,
                                                        const ir::Identifier *const prop) const
{
    if (prop->Name() == TO_STRING_METHOD_NAME) {
        ASSERT(toStringMethod_.memberProxyType != nullptr);
        return toStringMethod_.memberProxyType;
    }

    if (prop->Name() == GET_VALUE_METHOD_NAME) {
        ASSERT(getValueMethod_.memberProxyType != nullptr);
        return getValueMethod_.memberProxyType;
    }

    if (prop->Name() == GET_NAME_METHOD_NAME) {
        ASSERT(getNameMethod_.memberProxyType != nullptr);
        return getNameMethod_.memberProxyType;
    }

    checker->ThrowTypeError({"No enum item method called '", prop->Name(), "'"}, prop->Start());
}

ETSFunctionType *ETSEnumInterface::LookupTypeMethod(ETSChecker *const checker, const ir::Identifier *const prop) const
{
    if (prop->Name() == VALUES_METHOD_NAME) {
        ASSERT(valuesMethod_.memberProxyType != nullptr);
        return valuesMethod_.memberProxyType;
    }

    if (prop->Name() == VALUE_OF_METHOD_NAME) {
        ASSERT(valueOfMethod_.memberProxyType != nullptr);
        return valueOfMethod_.memberProxyType;
    }

    checker->ThrowTypeError({"No enum type method called '", prop->Name(), "'"}, prop->Start());
}

std::string EnumDescription(util::StringView name)
{
    return "#Enum#" + static_cast<std::string>(name);
}

ETSEnum2Type::ETSEnum2Type(ETSChecker *checker, util::StringView name, util::StringView assembler_name,
                           ir::AstNode *decl_node, ETSObjectFlags flags)
    : ETSObjectType(checker->Allocator(), name, assembler_name, decl_node, flags | ETSObjectFlags::ENUM2)
{
    AddTypeFlag(TypeFlag::ETS_ENUM2);
    CreateLiteralTypes(checker, name, assembler_name, decl_node, flags);
}

ETSEnum2Type::ETSEnum2Type(ArenaAllocator *allocator, util::StringView name, util::StringView assembler_name,
                           ir::AstNode *decl_node, ETSObjectFlags flags, ir::Literal *value)
    : ETSObjectType(allocator, name, assembler_name, decl_node, flags | ETSObjectFlags::ENUM2), value_(value)
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
                                      ir::AstNode *decl_node, ETSObjectFlags flags)
{
    ASSERT(decl_node->IsClassDefinition());

    for (auto &it : decl_node->AsClassDefinition()->Body()) {
        if (!it->IsClassProperty() || !it->AsClassProperty()->Value()->IsCallExpression()) {
            // @@arr property
            auto *arr_ident = it->AsClassProperty()->Id();
            auto *arr_var = arr_ident->Variable();
            ASSERT(arr_ident->Name() == "@@arr" && arr_var != nullptr);

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

        auto *enum_literal_type =
            Allocator()->New<ETSEnum2Type>(checker->Allocator(), name, assembler_name, decl_node, flags, literal);

        enum_literal_type->SetVariable(var);
        var->SetTsType(enum_literal_type);

        checker->GetSuperType(enum_literal_type);
    }
}
}  // namespace ark::es2panda::checker
