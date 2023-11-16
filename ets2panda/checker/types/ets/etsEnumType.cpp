/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

namespace panda::es2panda::checker {
ETSEnumInterface::ETSEnumInterface(const ir::TSEnumDeclaration *const enum_decl, UType ordinal,
                                   const ir::TSEnumMember *const member, TypeFlag const type_flag)
    : Type(type_flag), decl_(enum_decl), ordinal_ {ordinal}, member_(member)
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
    if (target->IsIntType()) {
        relation->Result(true);
        return;
    }

    conversion::Forbidden(relation);
}

Type *ETSEnumInterface::Instantiate([[maybe_unused]] ArenaAllocator *allocator, [[maybe_unused]] TypeRelation *relation,
                                    [[maybe_unused]] GlobalTypesHolder *global_types)
{
    return this;
}

void ETSEnumInterface::Identical(TypeRelation *const relation, Type *const other)
{
    ETSEnumInterface const *const other_enum_type = [other]() -> ETSEnumInterface const * {
        if (other->IsETSEnumType()) {
            return other->AsETSEnumType();
        }
        if (other->IsETSStringEnumType()) {
            return other->AsETSStringEnumType();
        }
        return nullptr;
    }();

    relation->Result(other_enum_type != nullptr && IsSameEnumType(other_enum_type) &&
                     member_ == other_enum_type->member_);
}

void ETSEnumInterface::ToAssemblerType(std::stringstream &ss) const
{
    ToAssemblerTypeImpl<UType>(ss);
}

void ETSEnumInterface::ToDebugInfoType(std::stringstream &ss) const
{
    ToDebugInfoTypeImpl<UType>(ss);
}

void ETSEnumInterface::ToString(std::stringstream &ss) const
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
        checker->ThrowTypeError({"Enum constant do not have property '", prop->Name(), "'"}, prop->Start());
    }

    auto *const member = FindMember(prop->Name());
    if (member == nullptr) {
        checker->ThrowTypeError({"No enum constant named '", prop->Name(), "' in enum '", this, "'"}, prop->Start());
    }

    auto *const enum_interface =
        [enum_type = member->Key()->AsIdentifier()->Variable()->TsType()]() -> checker::ETSEnumInterface * {
        if (enum_type->IsETSEnumType()) {
            return enum_type->AsETSEnumType();
        }
        return enum_type->AsETSStringEnumType();
    }();

    ASSERT(enum_interface->IsLiteralType());
    return enum_interface;
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

bool ETSEnumInterface::IsEnumInstanceExpression(const ir::Expression *const expression) const noexcept
{
    [[maybe_unused]] ETSEnumInterface const *const enum_interface =
        [enum_type = expression->TsType()]() -> ETSEnumInterface const * {
        if (enum_type->IsETSEnumType()) {
            return enum_type->AsETSEnumType();
        }
        if (enum_type->IsETSStringEnumType()) {
            return enum_type->AsETSStringEnumType();
        }
        return nullptr;
    }();

    ASSERT(IsSameEnumType(enum_interface));

    return IsEnumLiteralExpression(expression) || !IsEnumTypeExpression(expression);
}

bool ETSEnumInterface::IsEnumLiteralExpression(const ir::Expression *const expression) const noexcept
{
    [[maybe_unused]] ETSEnumInterface const *const enum_interface =
        [enum_type = expression->TsType()]() -> ETSEnumInterface const * {
        if (enum_type->IsETSEnumType()) {
            return enum_type->AsETSEnumType();
        }
        if (enum_type->IsETSStringEnumType()) {
            return enum_type->AsETSStringEnumType();
        }
        return nullptr;
    }();

    ASSERT(IsSameEnumType(enum_interface));

    if (expression->IsMemberExpression()) {
        const auto *const member_expr = expression->AsMemberExpression();
        return member_expr->Kind() == ir::MemberExpressionKind::PROPERTY_ACCESS &&
               IsEnumTypeExpression(member_expr->Object());
    }

    return false;
}

bool ETSEnumInterface::IsEnumTypeExpression(const ir::Expression *const expression) const noexcept
{
    [[maybe_unused]] ETSEnumInterface const *const enum_interface =
        [enum_type = expression->TsType()]() -> ETSEnumInterface const * {
        if (enum_type->IsETSEnumType()) {
            return enum_type->AsETSEnumType();
        }
        if (enum_type->IsETSStringEnumType()) {
            return enum_type->AsETSStringEnumType();
        }
        return nullptr;
    }();

    ASSERT(IsSameEnumType(enum_interface));

    if (expression->IsCallExpression()) {
        return false;
    }

    const auto *const local_var = [expression]() -> const varbinder::LocalVariable * {
        if (expression->IsMemberExpression()) {
            const auto *const member_expr = expression->AsMemberExpression();
            return member_expr->PropVar() != nullptr
                       ? member_expr->PropVar()
                       : member_expr->Object()->AsIdentifier()->Variable()->AsLocalVariable();
        }
        return expression->AsIdentifier()->Variable()->AsLocalVariable();
    }();

    ASSERT(local_var->Declaration() == decl_->Key()->AsIdentifier()->Variable()->Declaration() ||
           !local_var->HasFlag(varbinder::VariableFlags::ENUM_LITERAL));
    return local_var->HasFlag(varbinder::VariableFlags::ENUM_LITERAL);
}

ETSEnumInterface::Method ETSEnumInterface::FromIntMethod() const noexcept
{
    ASSERT(from_int_method_.global_signature != nullptr && from_int_method_.member_proxy_type == nullptr);
    return from_int_method_;
}

ETSEnumInterface::Method ETSEnumInterface::GetValueMethod() const noexcept
{
    ASSERT(get_value_method_.global_signature != nullptr && get_value_method_.member_proxy_type != nullptr);
    return get_value_method_;
}

ETSEnumInterface::Method ETSEnumInterface::GetNameMethod() const noexcept
{
    ASSERT(get_name_method_.global_signature != nullptr && get_name_method_.member_proxy_type != nullptr);
    return get_name_method_;
}

ETSEnumInterface::Method ETSEnumInterface::ToStringMethod() const noexcept
{
    ASSERT(to_string_method_.global_signature != nullptr && to_string_method_.member_proxy_type != nullptr);
    return to_string_method_;
}

ETSEnumInterface::Method ETSEnumInterface::ValueOfMethod() const noexcept
{
    ASSERT(value_of_method_.global_signature != nullptr && value_of_method_.member_proxy_type != nullptr);
    return value_of_method_;
}

ETSEnumInterface::Method ETSEnumInterface::ValuesMethod() const noexcept
{
    ASSERT(values_method_.global_signature != nullptr && values_method_.member_proxy_type != nullptr);
    return values_method_;
}

bool ETSEnumInterface::IsLiteralType() const noexcept
{
    return member_ != nullptr;
}

ir::TSEnumMember *ETSEnumInterface::FindMember(const util::StringView &name) const noexcept
{
    ASSERT(!IsLiteralType());
    const auto &members = GetMembers();
    auto member_it = std::find_if(members.begin(), members.end(), [name](const ir::AstNode *const node) {
        return node->AsTSEnumMember()->Key()->AsIdentifier()->Name() == name;
    });

    if (member_it != members.end()) {
        return (*member_it)->AsTSEnumMember();
    }

    return nullptr;
}

ETSFunctionType *ETSEnumInterface::LookupConstantMethod(ETSChecker *const checker,
                                                        const ir::Identifier *const prop) const
{
    if (prop->Name() == TO_STRING_METHOD_NAME) {
        ASSERT(to_string_method_.member_proxy_type != nullptr);
        return to_string_method_.member_proxy_type;
    }

    if (prop->Name() == GET_VALUE_METHOD_NAME) {
        ASSERT(get_value_method_.member_proxy_type != nullptr);
        return get_value_method_.member_proxy_type;
    }

    if (prop->Name() == GET_NAME_METHOD_NAME) {
        ASSERT(get_name_method_.member_proxy_type != nullptr);
        return get_name_method_.member_proxy_type;
    }

    checker->ThrowTypeError({"No enum item method called '", prop->Name(), "'"}, prop->Start());
}

ETSFunctionType *ETSEnumInterface::LookupTypeMethod(ETSChecker *const checker, const ir::Identifier *const prop) const
{
    if (prop->Name() == VALUES_METHOD_NAME) {
        ASSERT(values_method_.member_proxy_type != nullptr);
        return values_method_.member_proxy_type;
    }

    if (prop->Name() == VALUE_OF_METHOD_NAME) {
        ASSERT(value_of_method_.member_proxy_type != nullptr);
        return value_of_method_.member_proxy_type;
    }

    checker->ThrowTypeError({"No enum type method called '", prop->Name(), "'"}, prop->Start());
}

}  // namespace panda::es2panda::checker
