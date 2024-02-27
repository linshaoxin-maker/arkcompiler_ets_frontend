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

#include "enumLoweringPostPhase.h"
#include "compiler/lowering/scopesInit/scopesInitPhase.h"

#include "utils/arena_containers.h"
#include "checker/ETSchecker.h"
#include "compiler/core/compilerContext.h"
#include "ir/base/classDefinition.h"
#include "ir/base/classProperty.h"
#include "ir/astNode.h"
#include "ir/expression.h"
#include "util/ustring.h"
#include "compiler/lowering/util.h"

#include "ir/ts/tsEnumDeclaration.h"

namespace ark::es2panda::compiler {

const char *g_enumGetvalueMethodName = "getValue";

ir::CallExpression *CreateCallExpression(ir::Expression *id, checker::ETSChecker *checker)
{
    auto *const callee = checker->AllocNode<ir::Identifier>(g_enumGetvalueMethodName, checker->Allocator());
    callee->SetReference();
    ir::Expression *const accessor =
        checker->AllocNode<ir::MemberExpression>(id, callee, ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    id->SetParent(accessor);
    callee->SetParent(accessor);

    auto *callExpression = checker->AllocNode<ir::CallExpression>(
        accessor, ArenaVector<ir::Expression *>(checker->Allocator()->Adapter()), nullptr, false);
    accessor->SetParent(callExpression);
    return callExpression;
}

ir::Expression *CreateTestExpression(ir::Identifier *id, CompilerContext *ctx, ir::CallExpression *callExpression)
{
    auto *const scope = NearestScope(id);
    auto *checker = ctx->Checker()->AsETSChecker();
    ASSERT(scope != nullptr);
    auto expressionCtx = varbinder::LexicalScope<varbinder::Scope>::Enter(checker->VarBinder(), scope);
    if (callExpression == nullptr) {
        callExpression = CreateCallExpression(id, checker);
    }

    auto *right = checker->AllocNode<ir::NumberLiteral>(util::StringView("0"));
    ir::Expression *testExpr =
        checker->AllocNode<ir::BinaryExpression>(callExpression, right, lexer::TokenType::PUNCTUATOR_NOT_EQUAL);
    callExpression->SetParent(testExpr);
    right->SetParent(testExpr);

    InitScopesPhaseETS::RunExternalNode(testExpr, ctx->VarBinder());
    checker->VarBinder()->AsETSBinder()->ResolveReferencesForScope(testExpr, scope);
    testExpr->Check(checker);

    return testExpr;
}

ir::Expression *CheckBinaryBranch(ir::Expression *ast, CompilerContext *ctx)
{
    auto updateExpression = [ctx](ir::Expression *object, ir::Expression *ast) -> ir::Expression * {
        if (object->IsIdentifier() && object->AsIdentifier()->Variable() != nullptr) {
            auto *checker = ctx->Checker()->AsETSChecker();
            auto *type = checker->GetTypeOfVariable(object->AsIdentifier()->Variable());
            ASSERT(type != nullptr);

            if (!type->IsETSEnumType()) {
                return ast;
            }

            auto *callExpr = CreateCallExpression(ast, checker);

            return callExpr;
        }
        return ast;
    };
    if (ast->IsMemberExpression()) {
        // i.e. we have following:
        //   'enum Color {Red, Blue, Green, Yellow}'
        //
        // so for such 'imcomplete' accessors like:
        //   'Color.Red'
        //
        // we'd need to replace it with
        //   'color.Red.getValue()'
        if (auto *object = ast->AsMemberExpression()->Object(); object != nullptr) {
            return updateExpression(object, ast);
        }
    } else if (ast->IsIdentifier()) {
        return updateExpression(ast->AsIdentifier(), ast);
    }
    return ast;
}

ir::AstNode *UpdateMemberExpression(ir::AstNode *ast, CompilerContext *ctx)
{
    [[maybe_unused]] auto *checker = ctx->Checker()->AsETSChecker();
    auto *const scope = NearestScope(ast);
    ASSERT(scope != nullptr);
    auto expressionCtx = varbinder::LexicalScope<varbinder::Scope>::Enter(checker->VarBinder(), scope);

    ast->AsBinaryExpression()->SetLeft(CheckBinaryBranch(
        ast->AsBinaryExpression()->Left()->Clone(checker->Allocator(), nullptr)->AsExpression(), ctx));
    ast->AsBinaryExpression()->SetRight(CheckBinaryBranch(
        ast->AsBinaryExpression()->Right()->Clone(checker->Allocator(), nullptr)->AsExpression(), ctx));

    InitScopesPhaseETS::RunExternalNode(ast, ctx->VarBinder());
    checker->VarBinder()->AsETSBinder()->ResolveReferencesForScope(ast, scope);
    ast->Check(checker);
    return ast;
}

bool EnumLoweringPostPhase::Perform(public_lib::Context *ctx, parser::Program *program)
{
    if (program->Extension() != ScriptExtension::ETS) {
        return true;
    }

    [[maybe_unused]] checker::ETSChecker *checker = ctx->checker->AsETSChecker();

    for (auto &[_, extPrograms] : program->ExternalSources()) {
        (void)_;
        for (auto *extProg : extPrograms) {
            Perform(ctx, extProg);
        }
    }

    program->Ast()->TransformChildrenRecursively([checker, ctx](ir::AstNode *ast) -> ir::AstNode * {
        if (ast->IsBinaryExpression()) {
            if (ast->AsBinaryExpression()->IsPostBitSet(
                    ir::PostProcessingBits::ENUM_LOWERING_POST_PROCESSING_REQUIRED)) {
                return UpdateMemberExpression(ast, ctx->compilerContext);
            }
            return ast;
        }
        if (ast->IsIfStatement() || ast->IsWhileStatement() || ast->IsDoWhileStatement() ||
            ast->IsForUpdateStatement() || ast->IsConditionalExpression()) {
            ir::Expression *test = nullptr;
            ir::Expression *testExpr = nullptr;

            if (ast->IsIfStatement()) {
                test = ast->AsIfStatement()->Test();
            } else if (ast->IsWhileStatement()) {
                test = ast->AsWhileStatement()->Test();
            } else if (ast->IsDoWhileStatement()) {
                test = ast->AsDoWhileStatement()->Test();
            } else if (ast->IsForUpdateStatement()) {
                test = ast->AsForUpdateStatement()->Test();
            } else if (ast->IsConditionalExpression()) {
                test = ast->AsConditionalExpression()->Test();
            }

            if (test == nullptr) {
                return ast;
            }
            if (test->IsIdentifier()) {
                // we got simple variable test expression, test against non-zero value
                if (test->AsIdentifier()->Variable() == nullptr) {
                    return ast;
                }
                auto *type = checker->GetTypeOfVariable(test->AsIdentifier()->Variable());
                ASSERT(type != nullptr);

                if (!type->IsETSEnumType()) {
                    return ast;
                }
                // ok now we need  to replace 'if (v)' to 'if (v.getValue() != 0)'
                // NOTE: what about string as enum constant?
                testExpr = CreateTestExpression(test->AsIdentifier(), ctx->compilerContext, nullptr);

            } else if (test->IsCallExpression()) {
                // simple call expression with default non-zero test, i.e.
                //
                //   'if (v.getValue())'
                //
                // this wll always be treated as 'true' since getValue() returns the EnumConst
                // object,but not the enum  value
                //
                // need  to checkif we're calling to getValue() for enum constant
                // and convert it into binary expression with '!= 0' test, i.e.
                //
                //   'if (v.getValue() != 0)'
                //
                // same for loop expressions.
                //
                if (ir::Expression *callee = test->AsCallExpression()->Callee();
                    (callee != nullptr) && (callee->IsMemberExpression())) {
                    if ((callee->AsMemberExpression()->Object() != nullptr) &&
                        callee->AsMemberExpression()->Object()->IsIdentifier()) {
                        auto *id = callee->AsMemberExpression()->Object()->AsIdentifier();
                        if (id->Variable() == nullptr) {
                            return ast;
                        }
                        auto *type = checker->GetTypeOfVariable(id->Variable());
                        ASSERT(type != nullptr);
                        if (!type->IsETSEnumType()) {
                            return ast;
                        }

                        if (ir::Expression *prop = callee->AsMemberExpression()->Property(); prop != nullptr) {
                            if (prop->IsIdentifier() && (prop->AsIdentifier()->Name() == g_enumGetvalueMethodName)) {
                                //  now we need to wrap it to the binary expression .. != 0
                                testExpr = CreateTestExpression(prop->AsIdentifier(), ctx->compilerContext,
                                                                test->AsCallExpression());
                            }
                        }
                    }
                }
            }
            if (testExpr == nullptr) {
                return ast;
            }
            if (ast->IsIfStatement()) {
                ast->AsIfStatement()->SetTest(testExpr);
            } else if (ast->IsWhileStatement()) {
                ast->AsWhileStatement()->SetTest(testExpr);
            } else if (ast->IsDoWhileStatement()) {
                ast->AsDoWhileStatement()->SetTest(testExpr);
            } else if (ast->IsForUpdateStatement()) {
                ast->AsForUpdateStatement()->SetTest(testExpr);
            } else if (ast->IsConditionalExpression()) {
                ast->AsConditionalExpression()->SetTest(testExpr);
            }
            testExpr->SetParent(ast);
            return ast;
        }
        return ast;
    });

    return true;
}

}  // namespace ark::es2panda::compiler
