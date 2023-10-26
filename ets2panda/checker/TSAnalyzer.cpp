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

checker::Type *TSAnalyzer::Check(ir::SpreadElement *expr) const
{
    (void)expr;
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
    TSChecker *checker = GetTSChecker();
    if (node->TsType() != nullptr) {
        return node->TsType();
    }

    checker::ScopeContext scope_ctx(checker, node->Scope());

    auto *signature_info = checker->Allocator()->New<checker::SignatureInfo>(checker->Allocator());
    checker->CheckFunctionParameterDeclarations(node->Params(), signature_info);

    bool is_call_signature = (node->Kind() == ir::TSSignatureDeclaration::TSSignatureDeclarationKind::CALL_SIGNATURE);

    if (node->ReturnTypeAnnotation() == nullptr) {
        if (is_call_signature) {
            checker->ThrowTypeError(
                "Call signature, which lacks return-type annotation, implicitly has an 'any' return type.",
                node->Start());
        }

        checker->ThrowTypeError(
            "Construct signature, which lacks return-type annotation, implicitly has an 'any' return type.",
            node->Start());
    }

    node->return_type_annotation_->Check(checker);
    checker::Type *return_type = node->return_type_annotation_->GetType(checker);

    auto *signature = checker->Allocator()->New<checker::Signature>(signature_info, return_type);

    checker::Type *placeholder_obj = nullptr;

    if (is_call_signature) {
        placeholder_obj = checker->CreateObjectTypeWithCallSignature(signature);
    } else {
        placeholder_obj = checker->CreateObjectTypeWithConstructSignature(signature);
    }

    node->SetTsType(placeholder_obj);
    return placeholder_obj;
}
// from ets folder
checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSClassLiteral *expr) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSFunctionType *node) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSImportDeclaration *node) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSLaunchExpression *expr) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSNewArrayInstanceExpression *expr) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSNewClassInstanceExpression *expr) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSNewMultiDimArrayInstanceExpression *expr) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ETSPackageDeclaration *st) const
{
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
checker::Type *TSAnalyzer::Check(ir::ArrayExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
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
    (void)expr;
    UNREACHABLE();
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
    TSChecker *checker = GetTSChecker();
    if (expr->Variable() == nullptr) {
        if (expr->Name().Is("undefined")) {
            return checker->GlobalUndefinedType();
        }

        checker->ThrowTypeError({"Cannot find name ", expr->Name()}, expr->Start());
    }

    const varbinder::Decl *decl = expr->Variable()->Declaration();

    if (decl->IsTypeAliasDecl() || decl->IsInterfaceDecl()) {
        checker->ThrowTypeError({expr->Name(), " only refers to a type, but is being used as a value here."},
                                expr->Start());
    }

    expr->SetTsType(checker->GetTypeOfVariable(expr->Variable()));
    return expr->TsType();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::ImportExpression *expr) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::MemberExpression *expr) const
{
    TSChecker *checker = GetTSChecker();
    checker::Type *base_type = checker->CheckNonNullType(expr->Object()->Check(checker), expr->Object()->Start());

    if (expr->IsComputed()) {
        checker::Type *index_type = expr->Property()->Check(checker);
        checker::Type *indexed_access_type = checker->GetPropertyTypeForIndexType(base_type, index_type);

        if (indexed_access_type != nullptr) {
            return indexed_access_type;
        }

        if (!index_type->HasTypeFlag(checker::TypeFlag::STRING_LIKE | checker::TypeFlag::NUMBER_LIKE)) {
            checker->ThrowTypeError({"Type ", index_type, " cannot be used as index type"}, expr->Property()->Start());
        }

        if (index_type->IsNumberType()) {
            checker->ThrowTypeError("No index signature with a parameter of type 'string' was found on type this type",
                                    expr->Start());
        }

        if (index_type->IsStringType()) {
            checker->ThrowTypeError("No index signature with a parameter of type 'number' was found on type this type",
                                    expr->Start());
        }

        switch (expr->Property()->Type()) {
            case ir::AstNodeType::IDENTIFIER: {
                checker->ThrowTypeError(
                    {"Property ", expr->Property()->AsIdentifier()->Name(), " does not exist on this type."},
                    expr->Property()->Start());
            }
            case ir::AstNodeType::NUMBER_LITERAL: {
                checker->ThrowTypeError(
                    {"Property ", expr->Property()->AsNumberLiteral()->Str(), " does not exist on this type."},
                    expr->Property()->Start());
            }
            case ir::AstNodeType::STRING_LITERAL: {
                checker->ThrowTypeError(
                    {"Property ", expr->Property()->AsStringLiteral()->Str(), " does not exist on this type."},
                    expr->Property()->Start());
            }
            default: {
                UNREACHABLE();
            }
        }
    }

    varbinder::Variable *prop = checker->GetPropertyOfType(base_type, expr->Property()->AsIdentifier()->Name());

    if (prop != nullptr) {
        checker::Type *prop_type = checker->GetTypeOfVariable(prop);
        if (prop->HasFlag(varbinder::VariableFlags::READONLY)) {
            prop_type->AddTypeFlag(checker::TypeFlag::READONLY);
        }

        return prop_type;
    }

    if (base_type->IsObjectType()) {
        checker::ObjectType *obj_type = base_type->AsObjectType();

        if (obj_type->StringIndexInfo() != nullptr) {
            checker::Type *index_type = obj_type->StringIndexInfo()->GetType();
            if (obj_type->StringIndexInfo()->Readonly()) {
                index_type->AddTypeFlag(checker::TypeFlag::READONLY);
            }

            return index_type;
        }
    }

    checker->ThrowTypeError({"Property ", expr->Property()->AsIdentifier()->Name(), " does not exist on this type."},
                            expr->Property()->Start());
    return nullptr;
}

checker::Type *TSAnalyzer::Check(ir::NewExpression *expr) const
{
    TSChecker *checker = GetTSChecker();
    checker::Type *callee_type = expr->callee_->Check(checker);

    if (callee_type->IsObjectType()) {
        checker::ObjectType *callee_obj = callee_type->AsObjectType();
        return checker->ResolveCallOrNewExpression(callee_obj->ConstructSignatures(), expr->Arguments(), expr->Start());
    }

    checker->ThrowTypeError("This expression is not callable.", expr->Start());
    return nullptr;
}

checker::Type *TSAnalyzer::Check(ir::ObjectExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::OmittedExpression *expr) const
{
    TSChecker *checker = GetTSChecker();
    return checker->GlobalUndefinedType();
}

checker::Type *TSAnalyzer::Check(ir::OpaqueTypeNode *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::SequenceExpression *expr) const
{
    TSChecker *checker = GetTSChecker();
    // NOTE: aszilagyi.
    return checker->GlobalAnyType();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::SuperExpression *expr) const
{
    TSChecker *checker = GetTSChecker();
    // NOTE: aszilagyi.
    return checker->GlobalAnyType();
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

checker::Type *TSAnalyzer::Check(ir::YieldExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}
// compile methods for LITERAL EXPRESSIONS in alphabetical order
checker::Type *TSAnalyzer::Check(ir::BigIntLiteral *expr) const
{
    TSChecker *checker = GetTSChecker();
    auto search = checker->BigintLiteralMap().find(expr->Str());
    if (search != checker->BigintLiteralMap().end()) {
        return search->second;
    }

    auto *new_bigint_literal_type = checker->Allocator()->New<checker::BigintLiteralType>(expr->Str(), false);
    checker->BigintLiteralMap().insert({expr->Str(), new_bigint_literal_type});
    return new_bigint_literal_type;
}

checker::Type *TSAnalyzer::Check(ir::BooleanLiteral *expr) const
{
    TSChecker *checker = GetTSChecker();
    return expr->Value() ? checker->GlobalTrueType() : checker->GlobalFalseType();
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::CharLiteral *expr) const
{
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
checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::AssertStatement *st) const
{
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::BlockStatement *st) const
{
    TSChecker *checker = GetTSChecker();
    checker::ScopeContext scope_ctx(checker, st->Scope());

    for (auto *it : st->Statements()) {
        it->Check(checker);
    }

    return nullptr;
}

checker::Type *TSAnalyzer::Check([[maybe_unused]] ir::BreakStatement *st) const
{
    return nullptr;
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

checker::Type *TSAnalyzer::Check(ir::VariableDeclarator *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::VariableDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *TSAnalyzer::Check(ir::WhileStatement *st) const
{
    (void)st;
    UNREACHABLE();
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
