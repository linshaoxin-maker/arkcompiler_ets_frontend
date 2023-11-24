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

#include "ETSAnalyzer.h"

#include "varbinder/varbinder.h"
#include "varbinder/ETSBinder.h"
#include "checker/ETSchecker.h"
#include "checker/ets/castingContext.h"
#include "checker/ets/typeRelationContext.h"
#include "ir/base/catchClause.h"
#include "ir/base/classProperty.h"
#include "ir/base/classStaticBlock.h"
#include "ir/base/property.h"
#include "ir/base/scriptFunction.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/arrayExpression.h"
#include "ir/expressions/assignmentExpression.h"
#include "ir/expressions/literals/stringLiteral.h"
#include "ir/expressions/memberExpression.h"
#include "ir/expressions/objectExpression.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/returnStatement.h"
#include "ir/statements/variableDeclaration.h"
#include "ir/statements/variableDeclarator.h"
#include "ir/statements/whileStatement.h"
#include "util/helpers.h"
namespace panda::es2panda::checker {

ETSChecker *ETSAnalyzer::GetETSChecker() const
{
    return static_cast<ETSChecker *>(GetChecker());
}

// from as folder
checker::Type *ETSAnalyzer::Check([[maybe_unused]] ir::NamedType *node) const
{
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check([[maybe_unused]] ir::PrefixAssertionExpression *expr) const
{
    UNREACHABLE();
}
// from base folder
checker::Type *ETSAnalyzer::Check(ir::CatchClause *st) const
{
    ETSChecker *checker = GetETSChecker();
    checker::ETSObjectType *exception_type = checker->GlobalETSObjectType();

    ir::Identifier *param_ident = st->Param()->AsIdentifier();

    if (param_ident->TypeAnnotation() != nullptr) {
        checker::Type *catch_param_annotation_type = param_ident->TypeAnnotation()->GetType(checker);

        exception_type = checker->CheckExceptionOrErrorType(catch_param_annotation_type, st->Param()->Start());
    }

    param_ident->Variable()->SetTsType(exception_type);

    st->Body()->Check(checker);

    st->SetTsType(exception_type);
    return exception_type;
}

checker::Type *ETSAnalyzer::Check(ir::ClassDefinition *node) const
{
    ETSChecker *checker = GetETSChecker();
    if (node->TsType() == nullptr) {
        checker->BuildClassProperties(node);
    }

    checker->CheckClassDefinition(node);
    return nullptr;
}

checker::Type *ETSAnalyzer::Check(ir::ClassProperty *st) const
{
    ASSERT(st->Key()->IsIdentifier());
    ETSChecker *checker = GetETSChecker();

    if (st->TsType() != nullptr) {
        return st->TsType();
    }

    checker::SavedCheckerContext saved_context(checker, checker->Context().Status(),
                                               checker->Context().ContainingClass(),
                                               checker->Context().ContainingSignature());

    if (st->IsStatic()) {
        checker->AddStatus(checker::CheckerStatus::IN_STATIC_CONTEXT);
    }

    st->SetTsType(
        checker->CheckVariableDeclaration(st->Key()->AsIdentifier(), st->TypeAnnotation(), st->Value(), st->flags_));

    return st->TsType();
}

checker::Type *ETSAnalyzer::Check(ir::ClassStaticBlock *st) const
{
    ETSChecker *checker = GetETSChecker();

    if (checker->HasStatus(checker::CheckerStatus::INNER_CLASS)) {
        checker->ThrowTypeError("Static initializer is not allowed in inner class.", st->Start());
    }

    auto *func = st->Function();
    st->SetTsType(checker->BuildFunctionSignature(func));
    checker::ScopeContext scope_ctx(checker, func->Scope());
    checker::SavedCheckerContext saved_context(checker, checker->Context().Status(),
                                               checker->Context().ContainingClass());
    checker->AddStatus(checker::CheckerStatus::IN_STATIC_BLOCK | checker::CheckerStatus::IN_STATIC_CONTEXT);
    func->Body()->Check(checker);
    return st->TsType();
}

checker::Type *ETSAnalyzer::Check([[maybe_unused]] ir::Decorator *st) const
{
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::MetaProperty *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::MethodDefinition *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::Property *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ScriptFunction *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check([[maybe_unused]] ir::SpreadElement *expr) const
{
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TemplateElement *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSIndexSignature *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSMethodSignature *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSPropertySignature *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSSignatureDeclaration *node) const
{
    (void)node;
    UNREACHABLE();
}
// from ets folder
checker::Type *ETSAnalyzer::Check(ir::ETSClassLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ETSFunctionType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ETSImportDeclaration *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ETSLaunchExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ETSNewArrayInstanceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ETSNewClassInstanceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ETSNewMultiDimArrayInstanceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ETSPackageDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ETSParameterExpression *expr) const
{
    ETSChecker *checker = GetETSChecker();
    if (expr->TsType() == nullptr) {
        checker::Type *param_type;

        if (expr->Ident()->TsType() != nullptr) {
            param_type = expr->Ident()->TsType();
        } else {
            param_type = !expr->IsRestParameter() ? expr->Ident()->Check(checker) : expr->spread_->Check(checker);
            if (expr->IsDefault()) {
                [[maybe_unused]] auto *const init_type = expr->Initializer()->Check(checker);
            }
        }

        expr->SetTsType(param_type);
    }

    return expr->TsType();
}

checker::Type *ETSAnalyzer::Check([[maybe_unused]] ir::ETSPrimitiveType *node) const
{
    return nullptr;
}

checker::Type *ETSAnalyzer::Check(ir::ETSStructDeclaration *node) const
{
    ETSChecker *checker = GetETSChecker();
    node->Definition()->Check(checker);
    return nullptr;
}

checker::Type *ETSAnalyzer::Check(ir::ETSTypeReference *node) const
{
    ETSChecker *checker = GetETSChecker();
    return node->GetType(checker);
}

checker::Type *ETSAnalyzer::Check(ir::ETSTypeReferencePart *node) const
{
    ETSChecker *checker = GetETSChecker();
    return node->GetType(checker);
}

checker::Type *ETSAnalyzer::Check(ir::ETSUnionType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check([[maybe_unused]] ir::ETSWildcardType *node) const
{
    UNREACHABLE();
}

// compile methods for EXPRESSIONS in alphabetical order

checker::Type *ETSAnalyzer::GetPreferredType(ir::ArrayExpression *expr) const
{
    return expr->preferred_type_;
}

checker::Type *ETSAnalyzer::Check(ir::ArrayExpression *expr) const
{
    ETSChecker *checker = GetETSChecker();
    if (expr->TsType() != nullptr) {
        return expr->TsType();
    }

    if (!expr->Elements().empty()) {
        if (expr->preferred_type_ == nullptr) {
            expr->preferred_type_ = expr->Elements()[0]->Check(checker);
        }

        for (auto *element : expr->Elements()) {
            if (element->IsArrayExpression() && expr->preferred_type_->IsETSArrayType()) {
                element->AsArrayExpression()->SetPreferredType(expr->preferred_type_->AsETSArrayType()->ElementType());
            }
            if (element->IsObjectExpression()) {
                element->AsObjectExpression()->SetPreferredType(expr->preferred_type_);
            }

            checker::Type *element_type = element->Check(checker);
            checker::AssignmentContext(checker->Relation(), element, element_type, expr->preferred_type_,
                                       element->Start(),
                                       {"Array element type '", element_type, "' is not assignable to explicit type '",
                                        GetPreferredType(expr), "'"});
        }
    }

    if (expr->preferred_type_ == nullptr) {
        checker->ThrowTypeError("Can't resolve array type", expr->Start());
    }

    expr->SetTsType(checker->CreateETSArrayType(expr->preferred_type_));
    auto array_type = expr->TsType()->AsETSArrayType();
    checker->CreateBuiltinArraySignature(array_type, array_type->Rank());
    return expr->TsType();
}

checker::Type *ETSAnalyzer::Check(ir::ArrowFunctionExpression *expr) const
{
    ETSChecker *checker = GetETSChecker();
    if (expr->TsType() != nullptr) {
        return expr->TsType();
    }

    auto *func_type = checker->BuildFunctionSignature(expr->Function(), false);

    if (expr->Function()->IsAsyncFunc()) {
        auto *ret_type = static_cast<checker::ETSObjectType *>(expr->Function()->Signature()->ReturnType());
        if (ret_type->AssemblerName() != checker->GlobalBuiltinPromiseType()->AssemblerName()) {
            checker->ThrowTypeError("Return type of async lambda must be 'Promise'", expr->Function()->Start());
        }
    }

    checker::ScopeContext scope_ctx(checker, expr->Function()->Scope());

    if (checker->HasStatus(checker::CheckerStatus::IN_INSTANCE_EXTENSION_METHOD)) {
        /*
        example code:
        ```
            class A {
                prop:number
            }
            function A.method() {
                let a = () => {
                    console.println(this.prop)
                }
            }
        ```
        here the enclosing class of arrow function should be Class A
        */
        checker->Context().SetContainingClass(
            checker->Scope()->Find(varbinder::VarBinder::MANDATORY_PARAM_THIS).variable->TsType()->AsETSObjectType());
    }

    checker::SavedCheckerContext saved_context(checker, checker->Context().Status(),
                                               checker->Context().ContainingClass());
    checker->AddStatus(checker::CheckerStatus::IN_LAMBDA);
    checker->Context().SetContainingSignature(func_type->CallSignatures()[0]);

    auto *body_type = expr->Function()->Body()->Check(checker);

    if (expr->Function()->Body()->IsExpression()) {
        if (expr->Function()->ReturnTypeAnnotation() == nullptr) {
            func_type->CallSignatures()[0]->SetReturnType(body_type);
        }

        checker::AssignmentContext(
            checker->Relation(), expr->Function()->Body()->AsExpression(), body_type,
            func_type->CallSignatures()[0]->ReturnType(), expr->Function()->Start(),
            {"Return statements return type is not compatible with the containing functions return type"},
            checker::TypeRelationFlag::DIRECT_RETURN);
    }

    checker->Context().SetContainingSignature(nullptr);
    checker->CheckCapturedVariables();

    for (auto [var, _] : checker->Context().CapturedVars()) {
        (void)_;
        expr->CapturedVars().push_back(var);
    }

    expr->SetTsType(func_type);
    return expr->TsType();
}

checker::Type *ETSAnalyzer::Check(ir::AssignmentExpression *expr) const
{
    ETSChecker *checker = GetETSChecker();

    if (expr->TsType() != nullptr) {
        return expr->TsType();
    }

    auto *left_type = expr->Left()->Check(checker);
    if (expr->Left()->IsMemberExpression() &&
        expr->Left()->AsMemberExpression()->Object()->TsType()->IsETSArrayType() &&
        expr->Left()->AsMemberExpression()->Property()->IsIdentifier() &&
        expr->Left()->AsMemberExpression()->Property()->AsIdentifier()->Name().Is("length")) {
        checker->ThrowTypeError("Setting the length of an array is not permitted", expr->Left()->Start());
    }

    if (expr->Left()->IsIdentifier()) {
        expr->target_ = expr->Left()->AsIdentifier()->Variable();
    } else {
        expr->target_ = expr->Left()->AsMemberExpression()->PropVar();
    }

    if (expr->target_ != nullptr) {
        checker->ValidateUnaryOperatorOperand(expr->target_);
    }

    checker::Type *source_type {};
    ir::Expression *relation_node = expr->Right();
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
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_PLUS_EQUAL: {
            std::tie(std::ignore, expr->operation_type_) = checker->CheckBinaryOperator(
                expr->Left(), expr->Right(), expr, expr->OperatorType(), expr->Start(), true);

            auto unboxed_left = checker->ETSBuiltinTypeAsPrimitiveType(left_type);
            source_type = unboxed_left == nullptr ? left_type : unboxed_left;

            relation_node = expr;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_SUBSTITUTION: {
            if (left_type->IsETSArrayType() && expr->Right()->IsArrayExpression()) {
                expr->Right()->AsArrayExpression()->SetPreferredType(left_type->AsETSArrayType()->ElementType());
            }
            if (expr->Right()->IsObjectExpression()) {
                expr->Right()->AsObjectExpression()->SetPreferredType(left_type);
            }

            source_type = expr->Right()->Check(checker);
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }

    checker::AssignmentContext(checker->Relation(), relation_node, source_type, left_type, expr->Right()->Start(),
                               {"Initializers type is not assignable to the target type"});

    expr->SetTsType(expr->Left()->TsType());
    return expr->TsType();
}

checker::Type *ETSAnalyzer::Check(ir::AwaitExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::BinaryExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::CallExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ChainExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ClassExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ConditionalExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::DirectEvalExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::FunctionExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::Identifier *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ImportExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::MemberExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::NewExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}
checker::Type *ETSAnalyzer::PreferredType(ir::ObjectExpression *expr) const
{
    return expr->preferred_type_;
}

checker::Type *ETSAnalyzer::Check(ir::ObjectExpression *expr) const
{
    ETSChecker *checker = GetETSChecker();
    if (expr->TsType() != nullptr) {
        return expr->TsType();
    }

    if (expr->PreferredType() == nullptr) {
        checker->ThrowTypeError({"need to specify target type for class composite"}, expr->Start());
    }
    if (!expr->PreferredType()->IsETSObjectType()) {
        checker->ThrowTypeError({"target type for class composite needs to be an object type"}, expr->Start());
    }

    if (expr->PreferredType()->IsETSDynamicType()) {
        for (ir::Expression *prop_expr : expr->Properties()) {
            ASSERT(prop_expr->IsProperty());
            ir::Property *prop = prop_expr->AsProperty();
            ir::Expression *value = prop->Value();
            value->Check(checker);
            ASSERT(value->TsType());
        }

        expr->SetTsType(expr->PreferredType());
        return expr->PreferredType();
    }

    checker::ETSObjectType *obj_type = expr->PreferredType()->AsETSObjectType();
    if (obj_type->HasObjectFlag(checker::ETSObjectFlags::ABSTRACT | checker::ETSObjectFlags::INTERFACE)) {
        checker->ThrowTypeError({"target type for class composite ", obj_type->Name(), " is not instantiable"},
                                expr->Start());
    }

    bool have_empty_constructor = false;
    for (checker::Signature *sig : obj_type->ConstructSignatures()) {
        if (sig->Params().empty()) {
            have_empty_constructor = true;
            checker->ValidateSignatureAccessibility(obj_type, sig, expr->Start());
            break;
        }
    }
    if (!have_empty_constructor) {
        checker->ThrowTypeError({"type ", obj_type->Name(), " has no parameterless constructor"}, expr->Start());
    }

    for (ir::Expression *prop_expr : expr->Properties()) {
        ASSERT(prop_expr->IsProperty());
        ir::Property *prop = prop_expr->AsProperty();
        ir::Expression *key = prop->Key();
        ir::Expression *value = prop->Value();

        util::StringView pname;
        if (key->IsStringLiteral()) {
            pname = key->AsStringLiteral()->Str();
        } else if (key->IsIdentifier()) {
            pname = key->AsIdentifier()->Name();
        } else {
            checker->ThrowTypeError({"key in class composite should be either identifier or string literal"},
                                    expr->Start());
        }
        varbinder::LocalVariable *lv = obj_type->GetProperty(
            pname, checker::PropertySearchFlags::SEARCH_INSTANCE_FIELD | checker::PropertySearchFlags::SEARCH_IN_BASE);
        if (lv == nullptr) {
            checker->ThrowTypeError({"type ", obj_type->Name(), " has no property named ", pname}, prop_expr->Start());
        }
        checker->ValidatePropertyAccess(lv, obj_type, prop_expr->Start());
        if (lv->HasFlag(varbinder::VariableFlags::READONLY)) {
            checker->ThrowTypeError({"cannot assign to readonly property ", pname}, prop_expr->Start());
        }

        auto *prop_type = checker->GetTypeOfVariable(lv);
        key->SetTsType(prop_type);

        if (value->IsObjectExpression()) {
            value->AsObjectExpression()->SetPreferredType(prop_type);
        }
        value->SetTsType(value->Check(checker));
        checker::AssignmentContext(checker->Relation(), value, value->TsType(), prop_type, value->Start(),
                                   {"value type is not assignable to the property type"});
    }

    expr->SetTsType(obj_type);
    return obj_type;
}

checker::Type *ETSAnalyzer::Check(ir::OmittedExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check([[maybe_unused]] ir::OpaqueTypeNode *expr) const
{
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::SequenceExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::SuperExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TaggedTemplateExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TemplateLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ThisExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::UnaryExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::UpdateExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check([[maybe_unused]] ir::YieldExpression *expr) const
{
    UNREACHABLE();
}
// compile methods for LITERAL EXPRESSIONS in alphabetical order
checker::Type *ETSAnalyzer::Check(ir::BigIntLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::BooleanLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::CharLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::NullLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::NumberLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::RegExpLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::StringLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::UndefinedLiteral *expr) const
{
    (void)expr;
    UNREACHABLE();
}

// compile methods for MODULE-related nodes in alphabetical order
checker::Type *ETSAnalyzer::Check(ir::ExportAllDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ExportDefaultDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ExportNamedDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ExportSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ImportDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ImportDefaultSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ImportNamespaceSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ImportSpecifier *st) const
{
    (void)st;
    UNREACHABLE();
}
// compile methods for STATEMENTS in alphabetical order
checker::Type *ETSAnalyzer::Check(ir::AssertStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::BlockStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::BreakStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ClassDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ContinueStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::DebuggerStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::DoWhileStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::EmptyStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ExpressionStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ForInStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ForOfStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ForUpdateStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::FunctionDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::IfStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::LabelledStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

void CheckArgumentVoidType(checker::Type *&func_return_type, ETSChecker *checker, const std::string &name,
                           ir::ReturnStatement *st)
{
    if (name.find(compiler::Signatures::ETS_MAIN_WITH_MANGLE_BEGIN) != std::string::npos) {
        if (func_return_type == checker->GlobalBuiltinVoidType()) {
            func_return_type = checker->GlobalVoidType();
        } else if (!func_return_type->IsETSVoidType() && !func_return_type->IsIntType()) {
            checker->ThrowTypeError("Bad return type, main enable only void or int type.", st->Start());
        }
    }
}

void CheckReturnType(ETSChecker *checker, checker::Type *func_return_type, checker::Type *argument_type,
                     ir::Expression *st_argument)
{
    if (func_return_type->IsETSVoidType() || func_return_type == checker->GlobalBuiltinVoidType()) {
        if (argument_type != checker->GlobalVoidType() && argument_type != checker->GlobalBuiltinVoidType()) {
            checker->ThrowTypeError("Unexpected return value, enclosing method return type is void.",
                                    st_argument->Start());
        }
    } else {
        checker::AssignmentContext(checker->Relation(), st_argument, argument_type, func_return_type,
                                   st_argument->Start(),
                                   {"Return statement type is not compatible with the enclosing method's return type."},
                                   checker::TypeRelationFlag::DIRECT_RETURN);
    }
}

void InferReturnType(ETSChecker *checker, ir::ScriptFunction *containing_func, checker::Type *&func_return_type,
                     ir::Expression *st_argument)
{
    //  First (or single) return statement in the function:
    func_return_type = st_argument == nullptr ? checker->GlobalBuiltinVoidType() : st_argument->Check(checker);
    if (func_return_type->HasTypeFlag(checker::TypeFlag::CONSTANT)) {
        // remove CONSTANT type modifier if exists
        func_return_type =
            func_return_type->Instantiate(checker->Allocator(), checker->Relation(), checker->GetGlobalTypesHolder());
        func_return_type->RemoveTypeFlag(checker::TypeFlag::CONSTANT);
    }

    containing_func->Signature()->SetReturnType(func_return_type);
    containing_func->Signature()->RemoveSignatureFlag(checker::SignatureFlags::NEED_RETURN_TYPE);
    checker->VarBinder()->AsETSBinder()->BuildFunctionName(containing_func);

    if (st_argument != nullptr && st_argument->IsObjectExpression()) {
        st_argument->AsObjectExpression()->SetPreferredType(func_return_type);
    }
}

void ProcessReturnStatements(ETSChecker *checker, ir::ScriptFunction *containing_func, checker::Type *&func_return_type,
                             ir::ReturnStatement *st, ir::Expression *st_argument)
{
    func_return_type = containing_func->Signature()->ReturnType();

    if (st_argument == nullptr) {
        // previous return statement(s) have value
        if (!func_return_type->IsETSVoidType() && func_return_type != checker->GlobalBuiltinVoidType()) {
            checker->ThrowTypeError("All return statements in the function should be empty or have a value.",
                                    st->Start());
        }
    } else {
        //  previous return statement(s) don't have any value
        if (func_return_type->IsETSVoidType() || func_return_type == checker->GlobalBuiltinVoidType()) {
            checker->ThrowTypeError("All return statements in the function should be empty or have a value.",
                                    st_argument->Start());
        }

        const auto name = containing_func->Scope()->InternalName().Mutf8();
        if (name.find(compiler::Signatures::ETS_MAIN_WITH_MANGLE_BEGIN) != std::string::npos) {
            if (func_return_type == checker->GlobalBuiltinVoidType()) {
                func_return_type = checker->GlobalVoidType();
            } else if (!func_return_type->IsETSVoidType() && !func_return_type->IsIntType()) {
                checker->ThrowTypeError("Bad return type, main enable only void or int type.", st->Start());
            }
        }

        if (st_argument->IsObjectExpression()) {
            st_argument->AsObjectExpression()->SetPreferredType(func_return_type);
        }

        if (st_argument->IsMemberExpression()) {
            checker->SetArrayPreferredTypeForNestedMemberExpressions(st_argument->AsMemberExpression(),
                                                                     func_return_type);
        }

        checker::Type *argument_type = st_argument->Check(checker);
        // remove CONSTANT type modifier if exists
        if (argument_type->HasTypeFlag(checker::TypeFlag::CONSTANT)) {
            argument_type =
                argument_type->Instantiate(checker->Allocator(), checker->Relation(), checker->GetGlobalTypesHolder());
            argument_type->RemoveTypeFlag(checker::TypeFlag::CONSTANT);
        }

        auto *const relation = checker->Relation();
        relation->SetNode(st_argument);

        if (!relation->IsIdenticalTo(func_return_type, argument_type)) {
            if (func_return_type->HasTypeFlag(checker::TypeFlag::ETS_ARRAY_OR_OBJECT) ||
                argument_type->HasTypeFlag(checker::TypeFlag::ETS_ARRAY_OR_OBJECT)) {
                // function return type should be of reference (object) type
                relation->SetFlags(checker::TypeRelationFlag::NONE);

                if (!argument_type->HasTypeFlag(checker::TypeFlag::ETS_ARRAY_OR_OBJECT)) {
                    argument_type = checker->PrimitiveTypeAsETSBuiltinType(argument_type);
                    if (argument_type == nullptr) {
                        checker->ThrowTypeError("Invalid return statement expression", st_argument->Start());
                    }
                    st_argument->AddBoxingUnboxingFlag(checker->GetBoxingFlag(argument_type));
                }

                if (!func_return_type->HasTypeFlag(checker::TypeFlag::ETS_ARRAY_OR_OBJECT)) {
                    func_return_type = checker->PrimitiveTypeAsETSBuiltinType(func_return_type);
                    if (func_return_type == nullptr) {
                        checker->ThrowTypeError("Invalid return function expression", st->Start());
                    }
                }

                func_return_type = checker->FindLeastUpperBound(func_return_type, argument_type);
                containing_func->Signature()->SetReturnType(func_return_type);
                containing_func->Signature()->AddSignatureFlag(checker::SignatureFlags::INFERRED_RETURN_TYPE);
            } else if (func_return_type->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE_RETURN) &&
                       argument_type->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE_RETURN)) {
                // function return type is of primitive type (including enums):
                relation->SetFlags(checker::TypeRelationFlag::DIRECT_RETURN |
                                   checker::TypeRelationFlag::IN_ASSIGNMENT_CONTEXT |
                                   checker::TypeRelationFlag::ASSIGNMENT_CONTEXT);
                if (relation->IsAssignableTo(func_return_type, argument_type)) {
                    func_return_type = argument_type;
                    containing_func->Signature()->SetReturnType(func_return_type);
                    containing_func->Signature()->AddSignatureFlag(checker::SignatureFlags::INFERRED_RETURN_TYPE);
                } else if (!relation->IsAssignableTo(argument_type, func_return_type)) {
                    checker->ThrowTypeError(
                        "Return statement type is not compatible with previous method's return statement "
                        "type(s).",
                        st_argument->Start());
                }
            } else {
                checker->ThrowTypeError("Invalid return statement type(s).", st->Start());
            }
        }

        relation->SetNode(nullptr);
        relation->SetFlags(checker::TypeRelationFlag::NONE);
    }
}

checker::Type *ETSAnalyzer::Check(ir::ReturnStatement *st) const
{
    ETSChecker *checker = GetETSChecker();

    ir::AstNode *ancestor = util::Helpers::FindAncestorGivenByType(st, ir::AstNodeType::SCRIPT_FUNCTION);
    ASSERT(ancestor && ancestor->IsScriptFunction());
    auto *containing_func = ancestor->AsScriptFunction();

    if (containing_func->IsConstructor()) {
        if (st->argument_ != nullptr) {
            checker->ThrowTypeError("Return statement with expression isn't allowed in constructor.", st->Start());
        }
        return nullptr;
    }

    ASSERT(containing_func->ReturnTypeAnnotation() != nullptr || containing_func->Signature()->ReturnType() != nullptr);

    checker::Type *func_return_type = nullptr;

    if (auto *const return_type_annotation = containing_func->ReturnTypeAnnotation();
        return_type_annotation != nullptr) {
        // Case when function's return type is defined explicitly:
        func_return_type = checker->GetTypeFromTypeAnnotation(return_type_annotation);

        if (st->argument_ == nullptr) {
            if (!func_return_type->IsETSVoidType() && func_return_type != checker->GlobalBuiltinVoidType()) {
                checker->ThrowTypeError("Missing return value.", st->Start());
            }
            func_return_type =
                containing_func->IsEntryPoint() ? checker->GlobalVoidType() : checker->GlobalBuiltinVoidType();
        } else {
            const auto name = containing_func->Scope()->InternalName().Mutf8();
            CheckArgumentVoidType(func_return_type, checker, name, st);

            if (st->argument_->IsObjectExpression()) {
                st->argument_->AsObjectExpression()->SetPreferredType(func_return_type);
            }
            if (st->argument_->IsMemberExpression()) {
                checker->SetArrayPreferredTypeForNestedMemberExpressions(st->argument_->AsMemberExpression(),
                                                                         func_return_type);
            }

            checker::Type *argument_type = st->argument_->Check(checker);

            CheckReturnType(checker, func_return_type, argument_type, st->argument_);
        }
    } else {
        //  Case when function's return type should be inferred from return statement(s):
        if (containing_func->Signature()->HasSignatureFlag(checker::SignatureFlags::NEED_RETURN_TYPE)) {
            InferReturnType(checker, containing_func, func_return_type, st->argument_);
        } else {
            //  All subsequent return statements:
            ProcessReturnStatements(checker, containing_func, func_return_type, st, st->argument_);
        }
    }

    if ((st->argument_ != nullptr) && st->argument_->IsArrayExpression()) {
        st->argument_->AsArrayExpression()->SetPreferredType(
            func_return_type->IsETSArrayType() ? func_return_type->AsETSArrayType()->ElementType() : func_return_type);
    }

    st->return_type_ = func_return_type;
    return nullptr;
}

checker::Type *ETSAnalyzer::Check(ir::SwitchCaseStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::SwitchStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::ThrowStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TryStatement *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::VariableDeclarator *st) const
{
    ETSChecker *checker = GetETSChecker();
    ASSERT(st->Id()->IsIdentifier());
    ir::ModifierFlags flags = ir::ModifierFlags::NONE;

    if (st->Id()->Parent()->Parent()->AsVariableDeclaration()->Kind() ==
        ir::VariableDeclaration::VariableDeclarationKind::CONST) {
        flags |= ir::ModifierFlags::CONST;
    }

    st->SetTsType(checker->CheckVariableDeclaration(st->Id()->AsIdentifier(),
                                                    st->Id()->AsIdentifier()->TypeAnnotation(), st->Init(), flags));
    return st->TsType();
}

checker::Type *ETSAnalyzer::Check(ir::VariableDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::WhileStatement *st) const
{
    ETSChecker *checker = GetETSChecker();
    checker::ScopeContext scope_ctx(checker, st->Scope());

    checker->CheckTruthinessOfType(st->Test());

    st->Body()->Check(checker);
    return nullptr;
}
// from ts folder
checker::Type *ETSAnalyzer::Check(ir::TSAnyKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSArrayType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSAsExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSBigintKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSBooleanKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSClassImplements *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSConditionalType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSConstructorType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSEnumDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSEnumMember *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSExternalModuleReference *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSFunctionType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSImportEqualsDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSImportType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSIndexedAccessType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSInferType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSInterfaceBody *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSInterfaceDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSInterfaceHeritage *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSIntersectionType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSLiteralType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSMappedType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSModuleBlock *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSModuleDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSNamedTupleMember *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSNeverKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSNonNullExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSNullKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSNumberKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSObjectKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSParameterProperty *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSParenthesizedType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSQualifiedName *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSStringKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSThisType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSTupleType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSTypeAliasDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSTypeAssertion *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSTypeLiteral *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSTypeOperator *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSTypeParameter *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSTypeParameterDeclaration *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSTypeParameterInstantiation *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSTypePredicate *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSTypeQuery *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSTypeReference *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSUndefinedKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSUnionType *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSUnknownKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::TSVoidKeyword *node) const
{
    (void)node;
    UNREACHABLE();
}

}  // namespace panda::es2panda::checker
