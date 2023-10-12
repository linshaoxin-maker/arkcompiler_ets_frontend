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

checker::Type *ETSAnalyzer::Check(ir::SpreadElement *expr) const
{
    (void)expr;
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
checker::Type *ETSAnalyzer::Check(ir::ArrayExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
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
    (void)expr;
    UNREACHABLE();
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

checker::Type *ETSAnalyzer::Check(ir::ObjectExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::OmittedExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::OpaqueTypeNode *expr) const
{
    (void)expr;
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
    ETSChecker *checker = GetETSChecker();
    if (expr->TsType() != nullptr) {
        return expr->TsType();
    }

    if (expr->Quasis().size() != expr->Expressions().size() + 1U) {
        checker->ThrowTypeError("Invalid string template expression", expr->Start());
    }

    for (auto *it : expr->Expressions()) {
        it->Check(checker);
    }

    for (auto *it : expr->Quasis()) {
        it->Check(checker);
    }

    expr->SetTsType(checker->GlobalBuiltinETSStringType());
    return expr->TsType();
}

checker::Type *ETSAnalyzer::Check(ir::ThisExpression *expr) const
{
    ETSChecker *checker = GetETSChecker();
    if (expr->TsType() != nullptr) {
        return expr->TsType();
    }

    /*
    example code:
    ```
        class A {
            prop
        }
        function A.method() {
            let a = () => {
                console.println(this.prop)
            }
        }
        is identical to
        function method(this: A) {
            let a = () => {
                console.println(this.prop)
            }
        }
    ```
    here when "this" is used inside an extension function, we need to bind "this" to the first
    parameter(MANDATORY_PARAM_THIS), and capture the paramter's variable other than containing class's variable
    */
    auto *variable = checker->AsETSChecker()->Scope()->Find(varbinder::VarBinder::MANDATORY_PARAM_THIS).variable;
    if (checker->HasStatus(checker::CheckerStatus::IN_INSTANCE_EXTENSION_METHOD)) {
        ASSERT(variable != nullptr);
        expr->SetTsType(variable->TsType());
    } else {
        expr->SetTsType(checker->CheckThisOrSuperAccess(expr, checker->Context().ContainingClass(), "this"));
    }

    if (checker->HasStatus(checker::CheckerStatus::IN_LAMBDA)) {
        if (checker->HasStatus(checker::CheckerStatus::IN_INSTANCE_EXTENSION_METHOD)) {
            checker->Context().AddCapturedVar(variable, expr->Start());
        } else {
            checker->Context().AddCapturedVar(checker->Context().ContainingClass()->Variable(), expr->Start());
        }
    }

    return expr->TsType();
}

checker::Type *ETSAnalyzer::Check(ir::UnaryExpression *expr) const
{
    ETSChecker *checker = GetETSChecker();

    if (expr->TsType() != nullptr) {
        return expr->TsType();
    }

    auto arg_type = expr->argument_->Check(checker);
    const auto is_cond_expr = expr->OperatorType() == lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK;
    checker::Type *operand_type = checker->ApplyUnaryOperatorPromotion(arg_type, true, true, is_cond_expr);
    auto unboxed_operand_type = is_cond_expr ? checker->ETSBuiltinTypeAsConditionalType(arg_type)
                                             : checker->ETSBuiltinTypeAsPrimitiveType(arg_type);

    switch (expr->OperatorType()) {
        case lexer::TokenType::PUNCTUATOR_MINUS:
        case lexer::TokenType::PUNCTUATOR_PLUS: {
            if (operand_type == nullptr || !operand_type->HasTypeFlag(checker::TypeFlag::ETS_NUMERIC)) {
                checker->ThrowTypeError("Bad operand type, the type of the operand must be numeric type.",
                                        expr->Argument()->Start());
            }

            if (operand_type->HasTypeFlag(checker::TypeFlag::CONSTANT) &&
                expr->OperatorType() == lexer::TokenType::PUNCTUATOR_MINUS) {
                expr->SetTsType(checker->NegateNumericType(operand_type, expr));
                break;
            }

            expr->SetTsType(operand_type);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_TILDE: {
            if (operand_type == nullptr || !operand_type->HasTypeFlag(checker::TypeFlag::ETS_INTEGRAL)) {
                checker->ThrowTypeError("Bad operand type, the type of the operand must be integral type.",
                                        expr->Argument()->Start());
            }

            if (operand_type->HasTypeFlag(checker::TypeFlag::CONSTANT)) {
                expr->SetTsType(checker->BitwiseNegateIntegralType(operand_type, expr));
                break;
            }

            expr->SetTsType(operand_type);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK: {
            if (checker->IsNullLikeOrVoidExpression(expr->Argument())) {
                auto ts_type = checker->CreateETSBooleanType(true);
                ts_type->AddTypeFlag(checker::TypeFlag::CONSTANT);
                expr->SetTsType(ts_type);
                break;
            }

            if (operand_type == nullptr || !operand_type->IsConditionalExprType()) {
                checker->ThrowTypeError("Bad operand type, the type of the operand must be boolean type.",
                                        expr->Argument()->Start());
            }

            auto expr_res = operand_type->ResolveConditionExpr();
            if (std::get<0>(expr_res)) {
                auto ts_type = checker->CreateETSBooleanType(!std::get<1>(expr_res));
                ts_type->AddTypeFlag(checker::TypeFlag::CONSTANT);
                expr->SetTsType(ts_type);
                break;
            }

            expr->SetTsType(operand_type);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_DOLLAR_DOLLAR: {
            expr->SetTsType(arg_type);
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }

    if (arg_type->IsETSObjectType() && (unboxed_operand_type != nullptr) &&
        unboxed_operand_type->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE)) {
        expr->Argument()->AddBoxingUnboxingFlag(checker->GetUnboxingFlag(unboxed_operand_type));
    }

    return expr->TsType();
}

checker::Type *ETSAnalyzer::Check(ir::UpdateExpression *expr) const
{
    (void)expr;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::YieldExpression *expr) const
{
    (void)expr;
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
    ETSChecker *checker = GetETSChecker();
    auto *arg_type = st->argument_->Check(checker);
    checker->CheckExceptionOrErrorType(arg_type, st->Start());

    if (checker->Relation()->IsAssignableTo(arg_type, checker->GlobalBuiltinExceptionType())) {
        checker->CheckThrowingStatements(st);
    }
    return nullptr;
}

checker::Type *ETSAnalyzer::Check(ir::TryStatement *st) const
{
    ETSChecker *checker = GetETSChecker();
    std::vector<checker::ETSObjectType *> exceptions;
    st->Block()->Check(checker);

    for (auto *catch_clause : st->CatchClauses()) {
        auto exception_type = catch_clause->Check(checker);

        if ((exception_type != nullptr) && (catch_clause->Param() != nullptr)) {
            auto *clause_type = exception_type->AsETSObjectType();

            for (auto *exception : exceptions) {
                checker->Relation()->IsIdenticalTo(clause_type, exception);
                if (checker->Relation()->IsTrue()) {
                    checker->ThrowTypeError("Redeclaration of exception type", catch_clause->Start());
                }
            }

            exceptions.push_back(clause_type);
        }
    }

    bool default_catch_found = false;

    for (auto *catch_clause : st->CatchClauses()) {
        if (default_catch_found) {
            checker->ThrowTypeError("Default catch clause should be the last in the try statement",
                                    catch_clause->Start());
        }

        default_catch_found = catch_clause->IsDefaultCatchClause();
    }

    if (st->HasFinalizer()) {
        st->finalizer_->Check(checker);
    }

    return nullptr;
}

checker::Type *ETSAnalyzer::Check(ir::VariableDeclarator *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::VariableDeclaration *st) const
{
    (void)st;
    UNREACHABLE();
}

checker::Type *ETSAnalyzer::Check(ir::WhileStatement *st) const
{
    (void)st;
    UNREACHABLE();
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
