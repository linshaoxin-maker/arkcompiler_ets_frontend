/*
 * Copyright (c) 2021 - 2023 Huawei Device Co., Ltd.
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

#include "TSAnalyzer.h"

#include "checker/TSchecker.h"
#include "checker/ts/destructuringContext.h"
#include "util/helpers.h"

namespace panda::es2panda::checker {

TSChecker *TSAnalyzer::GetTSChecker() const
{
    return static_cast<TSChecker *>(GetChecker());
}

// from as folder
checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::NamedType *node) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::PrefixAssertionExpression *expr) const
{
    UNREACHABLE();
}
// from base folder
checker::Type *TSAnalyzer::Check(ir::CatchClause *st) const
{
    TSChecker *checker = GetTSChecker();
    ir::Expression *type_annotation = st->Param()->AsAnnotatedExpression()->TypeAnnotation();

    if (type_annotation != nullptr) {
        checker::Type *catch_param_type = type_annotation->Check(checker);

        if (!catch_param_type->HasTypeFlag(checker::TypeFlag::ANY_OR_UNKNOWN)) {
            checker->ThrowTypeError("Catch clause variable type annotation must be 'any' or 'unknown' if specified",
                                    st->Start());
        }
    }

    st->Body()->Check(checker);

    return nullptr;
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ClassDefinition *node) const
{
    TSChecker *checker = GetTSChecker();
    // NOTE: aszilagyi.
    return checker->GlobalAnyType();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ClassProperty *st) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ClassStaticBlock *st) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::Decorator *st) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::MetaProperty *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::MethodDefinition *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::Property *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ScriptFunction *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::SpreadElement *expr) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TemplateElement *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSIndexSignature *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSMethodSignature *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSPropertySignature *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSSignatureDeclaration *node) const
{
    (void)node;
    UNREACHABLE();
}
// from ets folder
checker::Type *TSAnalyzer::Check(ir::ETSClassLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ETSFunctionType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ETSImportDeclaration *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ETSLaunchExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ETSNewArrayInstanceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ETSNewClassInstanceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ETSNewMultiDimArrayInstanceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ETSPackageDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSParameterExpression *expr) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSPrimitiveType *node) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSStructDeclaration *node) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSTypeReference *node) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSTypeReferencePart *node) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ETSUnionType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSWildcardType *node) const
{
    UNREACHABLE();
}
// compile methods for EXPRESSIONS in alphabetical order

static void GetSpreadElementType(checker::TSChecker *checker, checker::Type *spread_type,
                                 ArenaVector<checker::Type *> &element_types, const lexer::SourcePosition &loc)
{
    bool in_const_context = checker->HasStatus(checker::CheckerStatus::IN_CONST_CONTEXT);

    if (spread_type->IsObjectType() && spread_type->AsObjectType()->IsTupleType()) {
        ArenaVector<checker::Type *> tuple_element_types(checker->Allocator()->Adapter());
        checker::TupleType *spread_tuple = spread_type->AsObjectType()->AsTupleType();

        for (auto *it : spread_tuple->Properties()) {
            if (in_const_context) {
                element_types.push_back(it->TsType());
                continue;
            }

            tuple_element_types.push_back(it->TsType());
        }

        if (in_const_context) {
            return;
        }

        element_types.push_back(checker->CreateUnionType(std::move(tuple_element_types)));
        return;
    }

    if (spread_type->IsUnionType()) {
        ArenaVector<checker::Type *> spread_types(checker->Allocator()->Adapter());
        bool throw_error = false;

        for (auto *type : spread_type->AsUnionType()->ConstituentTypes()) {
            if (type->IsArrayType()) {
                spread_types.push_back(type->AsArrayType()->ElementType());
                continue;
            }

            if (type->IsObjectType() && type->AsObjectType()->IsTupleType()) {
                checker::TupleType *tuple = type->AsObjectType()->AsTupleType();

                for (auto *it : tuple->Properties()) {
                    spread_types.push_back(it->TsType());
                }

                continue;
            }

            throw_error = true;
            break;
        }

        if (!throw_error) {
            element_types.push_back(checker->CreateUnionType(std::move(spread_types)));
            return;
        }
    }

    checker->ThrowTypeError(
        {"Type '", spread_type, "' must have a '[Symbol.iterator]()' method that returns an iterator."}, loc);
}

checker::Type *TSAnalyzer::Check(ir::ArrayExpression *expr) const
{
    TSChecker *checker = GetTSChecker();
    ArenaVector<checker::Type *> element_types(checker->Allocator()->Adapter());
    ArenaVector<checker::ElementFlags> element_flags(checker->Allocator()->Adapter());
    bool in_const_context = checker->HasStatus(checker::CheckerStatus::IN_CONST_CONTEXT);
    bool create_tuple = checker->HasStatus(checker::CheckerStatus::FORCE_TUPLE);

    for (auto *it : expr->Elements()) {
        if (it->IsSpreadElement()) {
            checker::Type *spread_type = it->AsSpreadElement()->Argument()->Check(checker);

            if (spread_type->IsArrayType()) {
                element_types.push_back(in_const_context ? spread_type : spread_type->AsArrayType()->ElementType());
                element_flags.push_back(checker::ElementFlags::VARIADIC);
                continue;
            }

            GetSpreadElementType(checker, spread_type, element_types, it->Start());
            element_flags.push_back(checker::ElementFlags::REST);
            continue;
        }

        checker::Type *element_type = it->Check(checker);

        if (!in_const_context) {
            element_type = checker->GetBaseTypeOfLiteralType(element_type);
        }

        element_flags.push_back(checker::ElementFlags::REQUIRED);
        element_types.push_back(element_type);
    }

    if (in_const_context || create_tuple) {
        checker::ObjectDescriptor *desc = checker->Allocator()->New<checker::ObjectDescriptor>(checker->Allocator());
        uint32_t index = 0;

        for (auto it = element_types.begin(); it != element_types.end(); it++, index++) {
            util::StringView member_index = util::Helpers::ToStringView(checker->Allocator(), index);
            varbinder::LocalVariable *tuple_member = varbinder::Scope::CreateVar(
                checker->Allocator(), member_index, varbinder::VariableFlags::PROPERTY, nullptr);

            if (in_const_context) {
                tuple_member->AddFlag(varbinder::VariableFlags::READONLY);
            }

            tuple_member->SetTsType(*it);
            desc->properties.push_back(tuple_member);
        }

        return checker->CreateTupleType(desc, std::move(element_flags), checker::ElementFlags::REQUIRED, index, index,
                                        in_const_context);
    }

    checker::Type *array_element_type = nullptr;
    if (element_types.empty()) {
        array_element_type = checker->GlobalAnyType();
    } else {
        array_element_type = checker->CreateUnionType(std::move(element_types));
    }

    return checker->Allocator()->New<checker::ArrayType>(array_element_type);
}

checker::Type *TSAnalyzer::Check(ir::ArrowFunctionExpression *expr) const
{
    TSChecker *checker = GetTSChecker();
    varbinder::Variable *func_var = nullptr;

    if (expr->Function()->Parent()->Parent() != nullptr &&
        expr->Function()->Parent()->Parent()->IsVariableDeclarator() &&
        expr->Function()->Parent()->Parent()->AsVariableDeclarator()->Id()->IsIdentifier()) {
        func_var = expr->Function()->Parent()->Parent()->AsVariableDeclarator()->Id()->AsIdentifier()->Variable();
    }

    checker::ScopeContext scope_ctx(checker, expr->Function()->Scope());

    auto *signature_info = checker->Allocator()->New<checker::SignatureInfo>(checker->Allocator());
    checker->CheckFunctionParameterDeclarations(expr->Function()->Params(), signature_info);

    auto *signature = checker->Allocator()->New<checker::Signature>(
        signature_info, checker->GlobalResolvingReturnType(), expr->Function());
    checker::Type *func_type = checker->CreateFunctionTypeWithSignature(signature);

    if (func_var != nullptr && func_var->TsType() == nullptr) {
        func_var->SetTsType(func_type);
    }

    signature->SetReturnType(checker->HandleFunctionReturn(expr->Function()));

    if (!expr->Function()->Body()->IsExpression()) {
        expr->Function()->Body()->Check(checker);
    }

    return func_type;
}

checker::Type *TSAnalyzer::Check(ir::AssignmentExpression *expr) const
{
    TSChecker *checker = GetTSChecker();
    if (expr->Left()->IsArrayPattern()) {
        auto saved_context = checker::SavedCheckerContext(checker, checker::CheckerStatus::FORCE_TUPLE);
        auto destructuring_context =
            checker::ArrayDestructuringContext(checker, expr->Left(), true, true, nullptr, expr->Right());
        destructuring_context.Start();
        return destructuring_context.InferredType();
    }

    if (expr->Left()->IsObjectPattern()) {
        auto saved_context = checker::SavedCheckerContext(checker, checker::CheckerStatus::FORCE_TUPLE);
        auto destructuring_context =
            checker::ObjectDestructuringContext(checker, expr->Left(), true, true, nullptr, expr->Right());
        destructuring_context.Start();
        return destructuring_context.InferredType();
    }

    if (expr->Left()->IsIdentifier() && expr->Left()->AsIdentifier()->Variable() != nullptr &&
        expr->Left()->AsIdentifier()->Variable()->Declaration()->IsConstDecl()) {
        checker->ThrowTypeError(
            {"Cannot assign to ", expr->Left()->AsIdentifier()->Name(), " because it is a constant."},
            expr->Left()->Start());
    }

    auto *left_type = expr->Left()->Check(checker);

    if (left_type->HasTypeFlag(checker::TypeFlag::READONLY)) {
        checker->ThrowTypeError("Cannot assign to this property because it is readonly.", expr->Left()->Start());
    }

    if (expr->OperatorType() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        checker->ElaborateElementwise(left_type, expr->Right(), expr->Left()->Start());
        return checker->CheckTypeCached(expr->Right());
    }

    auto *right_type = expr->Right()->Check(checker);

    switch (expr->OperatorType()) {
        case lexer::TokenType::PUNCTUATOR_MULTIPLY_EQUAL:
        case lexer::TokenType::PUNCTUATOR_EXPONENTIATION_EQUAL:
        case lexer::TokenType::PUNCTUATOR_DIVIDE_EQUAL:
        case lexer::TokenType::PUNCTUATOR_MOD_EQUAL:
        case lexer::TokenType::PUNCTUATOR_MINUS_EQUAL:
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR_EQUAL: {
            return checker->CheckBinaryOperator(left_type, right_type, expr->Left(), expr->Right(), expr,
                                                expr->OperatorType());
        }
        case lexer::TokenType::PUNCTUATOR_PLUS_EQUAL: {
            return checker->CheckPlusOperator(left_type, right_type, expr->Left(), expr->Right(), expr,
                                              expr->OperatorType());
        }
        case lexer::TokenType::PUNCTUATOR_SUBSTITUTION: {
            checker->CheckAssignmentOperator(expr->OperatorType(), expr->Left(), left_type, right_type);
            return right_type;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }

    return nullptr;
}

checker::Type *TSAnalyzer::Check(ir::AwaitExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::BinaryExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::CallExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ChainExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ClassExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ConditionalExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::DirectEvalExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::FunctionExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::Identifier *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ImportExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::MemberExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::NewExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}
static const util::StringView &GetPropertyName(const ir::Expression *key)
{
    if (key->IsIdentifier()) {
        return key->AsIdentifier()->Name();
    }

    if (key->IsStringLiteral()) {
        return key->AsStringLiteral()->Str();
    }

    ASSERT(key->IsNumberLiteral());
    return key->AsNumberLiteral()->Str();
}

static varbinder::VariableFlags GetFlagsForProperty(const ir::Property *prop)
{
    if (!prop->IsMethod()) {
        return varbinder::VariableFlags::PROPERTY;
    }

    varbinder::VariableFlags prop_flags = varbinder::VariableFlags::METHOD;

    if (prop->IsAccessor() && prop->Kind() == ir::PropertyKind::GET) {
        prop_flags |= varbinder::VariableFlags::READONLY;
    }

    return prop_flags;
}

static checker::Type *GetTypeForProperty(ir::Property *prop, checker::TSChecker *checker)
{
    if (prop->IsAccessor()) {
        checker::Type *func_type = prop->Value()->Check(checker);

        if (prop->Kind() == ir::PropertyKind::SET) {
            return checker->GlobalAnyType();
        }

        ASSERT(func_type->IsObjectType() && func_type->AsObjectType()->IsFunctionType());
        return func_type->AsObjectType()->CallSignatures()[0]->ReturnType();
    }

    if (prop->IsShorthand()) {
        return prop->Key()->Check(checker);
    }

    return prop->Value()->Check(checker);
}

checker::Type *TSAnalyzer::Check(ir::ObjectExpression *expr) const
{
    TSChecker *checker = GetTSChecker();

    checker::ObjectDescriptor *desc = checker->Allocator()->New<checker::ObjectDescriptor>(checker->Allocator());
    std::unordered_map<util::StringView, lexer::SourcePosition> all_properties_map;
    bool in_const_context = checker->HasStatus(checker::CheckerStatus::IN_CONST_CONTEXT);
    ArenaVector<checker::Type *> computed_number_prop_types(checker->Allocator()->Adapter());
    ArenaVector<checker::Type *> computed_string_prop_types(checker->Allocator()->Adapter());
    bool has_computed_number_property = false;
    bool has_computed_string_property = false;
    bool seen_spread = false;

    for (auto *it : expr->Properties()) {
        if (it->IsProperty()) {
            auto *prop = it->AsProperty();

            if (prop->IsComputed()) {
                checker::Type *computed_name_type = checker->CheckComputedPropertyName(prop->Key());

                if (computed_name_type->IsNumberType()) {
                    has_computed_number_property = true;
                    computed_number_prop_types.push_back(prop->Value()->Check(checker));
                    continue;
                }

                if (computed_name_type->IsStringType()) {
                    has_computed_string_property = true;
                    computed_string_prop_types.push_back(prop->Value()->Check(checker));
                    continue;
                }
            }

            checker::Type *prop_type = GetTypeForProperty(prop, checker);
            varbinder::VariableFlags flags = GetFlagsForProperty(prop);
            const util::StringView &prop_name = GetPropertyName(prop->Key());

            auto *member_var = varbinder::Scope::CreateVar(checker->Allocator(), prop_name, flags, it);

            if (in_const_context) {
                member_var->AddFlag(varbinder::VariableFlags::READONLY);
            } else {
                prop_type = checker->GetBaseTypeOfLiteralType(prop_type);
            }

            member_var->SetTsType(prop_type);

            if (prop->Key()->IsNumberLiteral()) {
                member_var->AddFlag(varbinder::VariableFlags::NUMERIC_NAME);
            }

            varbinder::LocalVariable *found_member = desc->FindProperty(prop_name);
            all_properties_map.insert({prop_name, it->Start()});

            if (found_member != nullptr) {
                found_member->SetTsType(prop_type);
                continue;
            }

            desc->properties.push_back(member_var);
            continue;
        }

        ASSERT(it->IsSpreadElement());

        checker::Type *const spread_type = it->AsSpreadElement()->Argument()->Check(checker);
        seen_spread = true;

        // NOTE: aszilagyi. handle union of object types
        if (!spread_type->IsObjectType()) {
            checker->ThrowTypeError("Spread types may only be created from object types.", it->Start());
        }

        for (auto *spread_prop : spread_type->AsObjectType()->Properties()) {
            auto found = all_properties_map.find(spread_prop->Name());
            if (found != all_properties_map.end()) {
                checker->ThrowTypeError(
                    {found->first, " is specified more than once, so this usage will be overwritten."}, found->second);
            }

            varbinder::LocalVariable *found_member = desc->FindProperty(spread_prop->Name());

            if (found_member != nullptr) {
                found_member->SetTsType(spread_prop->TsType());
                continue;
            }

            desc->properties.push_back(spread_prop);
        }
    }

    if (!seen_spread && (has_computed_number_property || has_computed_string_property)) {
        for (auto *it : desc->properties) {
            computed_string_prop_types.push_back(it->TsType());

            if (has_computed_number_property && it->HasFlag(varbinder::VariableFlags::NUMERIC_NAME)) {
                computed_number_prop_types.push_back(it->TsType());
            }
        }

        if (has_computed_number_property) {
            desc->number_index_info = checker->Allocator()->New<checker::IndexInfo>(
                checker->CreateUnionType(std::move(computed_number_prop_types)), "x", in_const_context);
        }

        if (has_computed_string_property) {
            desc->string_index_info = checker->Allocator()->New<checker::IndexInfo>(
                checker->CreateUnionType(std::move(computed_string_prop_types)), "x", in_const_context);
        }
    }

    checker::Type *return_type = checker->Allocator()->New<checker::ObjectLiteralType>(desc);
    return_type->AsObjectType()->AddObjectFlag(checker::ObjectFlags::RESOLVED_MEMBERS |
                                               checker::ObjectFlags::CHECK_EXCESS_PROPS);
    return return_type;
}

checker::Type *TSAnalyzer::Check(ir::OmittedExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::OpaqueTypeNode *expr) const
{
    return expr->TsType();
}

checker::Type *TSAnalyzer::Check(ir::SequenceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::SuperExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TaggedTemplateExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TemplateLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ThisExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::UnaryExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::UpdateExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::YieldExpression *expr) const
{
    TSChecker *checker = GetTSChecker();
    // NOTE: aszilagyi.
    return checker->GlobalAnyType();
}
// compile methods for LITERAL EXPRESSIONS in alphabetical order
checker::Type *TSAnalyzer::Check(ir::BigIntLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::BooleanLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::CharLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::NullLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::NumberLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::RegExpLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::StringLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::UndefinedLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

// compile methods for MODULE-related nodes in alphabetical order
checker::Type *TSAnalyzer::Check(ir::ExportAllDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ExportDefaultDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ExportNamedDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ExportSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ImportDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ImportDefaultSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ImportNamespaceSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ImportSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}
// compile methods for STATEMENTS in alphabetical order
checker::Type *TSAnalyzer::Check(ir::AssertStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::BlockStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::BreakStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ClassDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ContinueStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::DebuggerStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::DoWhileStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::EmptyStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ExpressionStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ForInStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ForOfStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ForUpdateStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::FunctionDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::IfStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::LabelledStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ReturnStatement *st) const
{
    TSChecker *checker = GetTSChecker();
    ir::AstNode *ancestor = util::Helpers::FindAncestorGivenByType(st, ir::AstNodeType::SCRIPT_FUNCTION);
    ASSERT(ancestor && ancestor->IsScriptFunction());
    auto *containing_func = ancestor->AsScriptFunction();

    if (containing_func->Parent()->Parent()->IsMethodDefinition()) {
        const ir::MethodDefinition *containing_class_method = containing_func->Parent()->Parent()->AsMethodDefinition();
        if (containing_class_method->Kind() == ir::MethodDefinitionKind::SET) {
            checker->ThrowTypeError("Setters cannot return a value", st->Start());
        }
    }

    if (containing_func->ReturnTypeAnnotation() != nullptr) {
        checker::Type *return_type = checker->GlobalUndefinedType();
        checker::Type *func_return_type = containing_func->ReturnTypeAnnotation()->GetType(checker);

        if (st->Argument() != nullptr) {
            checker->ElaborateElementwise(func_return_type, st->Argument(), st->Start());
            return_type = checker->CheckTypeCached(st->Argument());
        }

        checker->IsTypeAssignableTo(return_type, func_return_type,
                                    {"Type '", return_type, "' is not assignable to type '", func_return_type, "'."},
                                    st->Start());
    }

    return nullptr;
}

checker::Type *TSAnalyzer::Check(ir::SwitchCaseStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::SwitchStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::ThrowStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TryStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

static void CheckSimpleVariableDeclaration(checker::TSChecker *checker, ir::VariableDeclarator *declarator)
{
    varbinder::Variable *const binding_var = declarator->Id()->AsIdentifier()->Variable();
    checker::Type *previous_type = binding_var->TsType();
    auto *const type_annotation = declarator->Id()->AsIdentifier()->TypeAnnotation();
    auto *const initializer = declarator->Init();
    const bool is_const = declarator->Parent()->AsVariableDeclaration()->Kind() ==
                          ir::VariableDeclaration::VariableDeclarationKind::CONST;

    if (is_const) {
        checker->AddStatus(checker::CheckerStatus::IN_CONST_CONTEXT);
    }

    if (type_annotation != nullptr) {
        type_annotation->Check(checker);
    }

    if (type_annotation != nullptr && initializer != nullptr) {
        checker::Type *const annotation_type = type_annotation->GetType(checker);
        checker->ElaborateElementwise(annotation_type, initializer, declarator->Id()->Start());
        binding_var->SetTsType(annotation_type);
    } else if (type_annotation != nullptr) {
        binding_var->SetTsType(type_annotation->GetType(checker));
    } else if (initializer != nullptr) {
        checker::Type *initializer_type = checker->CheckTypeCached(initializer);

        if (!is_const) {
            initializer_type = checker->GetBaseTypeOfLiteralType(initializer_type);
        }

        if (initializer_type->IsNullType()) {
            checker->ThrowTypeError(
                {"Cannot infer type for variable '", declarator->Id()->AsIdentifier()->Name(), "'."},
                declarator->Id()->Start());
        }

        binding_var->SetTsType(initializer_type);
    } else {
        checker->ThrowTypeError({"Variable ", declarator->Id()->AsIdentifier()->Name(), " implicitly has an any type."},
                                declarator->Id()->Start());
    }

    if (previous_type != nullptr) {
        checker->IsTypeIdenticalTo(binding_var->TsType(), previous_type,
                                   {"Subsequent variable declaration must have the same type. Variable '",
                                    binding_var->Name(), "' must be of type '", previous_type, "', but here has type '",
                                    binding_var->TsType(), "'."},
                                   declarator->Id()->Start());
    }

    checker->RemoveStatus(checker::CheckerStatus::IN_CONST_CONTEXT);
}

checker::Type *TSAnalyzer::Check(ir::VariableDeclarator *st) const
{
    TSChecker *checker = GetTSChecker();

    if (st->TsType() == st->CHECKED) {
        return nullptr;
    }

    if (st->Id()->IsIdentifier()) {
        CheckSimpleVariableDeclaration(checker, st);
        st->SetTsType(st->CHECKED);
        return nullptr;
    }

    if (st->Id()->IsArrayPattern()) {
        auto context = checker::SavedCheckerContext(checker, checker::CheckerStatus::FORCE_TUPLE);
        checker::ArrayDestructuringContext(checker, st->Id(), false,
                                           st->Id()->AsArrayPattern()->TypeAnnotation() == nullptr,
                                           st->Id()->AsArrayPattern()->TypeAnnotation(), st->Init())
            .Start();

        st->SetTsType(st->CHECKED);
        return nullptr;
    }

    ASSERT(st->Id()->IsObjectPattern());
    auto context = checker::SavedCheckerContext(checker, checker::CheckerStatus::FORCE_TUPLE);
    checker::ObjectDestructuringContext(checker, st->Id(), false,
                                        st->Id()->AsObjectPattern()->TypeAnnotation() == nullptr,
                                        st->Id()->AsObjectPattern()->TypeAnnotation(), st->Init())
        .Start();

    st->SetTsType(st->CHECKED);
    return nullptr;
}

checker::Type *TSAnalyzer::Check(ir::VariableDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::WhileStatement *st) const
{
    TSChecker *checker = GetTSChecker();
    checker::ScopeContext scope_ctx(checker, st->Scope());

    checker::Type *test_type = st->Test()->Check(checker);
    checker->CheckTruthinessOfType(test_type, st->Test()->Start());

    st->Body()->Check(checker);
    return nullptr;
}
// from ts folder
checker::Type *TSAnalyzer::Check(ir::TSAnyKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSArrayType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSAsExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSBigintKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSBooleanKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSClassImplements *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSConditionalType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSConstructorType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSEnumDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSEnumMember *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSExternalModuleReference *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSFunctionType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSImportEqualsDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSImportType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSIndexedAccessType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSInferType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSInterfaceBody *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSInterfaceDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSInterfaceHeritage *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSIntersectionType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSLiteralType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSMappedType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSModuleBlock *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSModuleDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSNamedTupleMember *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSNeverKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSNonNullExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSNullKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSNumberKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSObjectKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSParameterProperty *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSParenthesizedType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSQualifiedName *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSStringKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSThisType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSTupleType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSTypeAliasDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSTypeAssertion *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSTypeLiteral *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSTypeOperator *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSTypeParameter *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSTypeParameterDeclaration *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSTypeParameterInstantiation *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSTypePredicate *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSTypeQuery *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSTypeReference *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSUndefinedKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSUnionType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSUnknownKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::TSVoidKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

}  // namespace panda::es2panda::checker
