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

const char *ENUM_GETVALUE_METHOD_NAME = "getValue";

ir::CallExpression *CreateCallExpression(ir::Expression *id, checker::ETSChecker *checker)
{
    auto *const callee = checker->AllocNode<ir::Identifier>(ENUM_GETVALUE_METHOD_NAME, checker->Allocator());
    callee->SetReference();
    ir::Expression *const accessor =
        checker->AllocNode<ir::MemberExpression>(id, callee, ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    id->SetParent(accessor);
    callee->SetParent(accessor);

    ir::CallExpression *callExpression = checker->AllocNode<ir::CallExpression>(
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

ir::Expression *checkBinaryBranch(ir::Expression *ast, CompilerContext *ctx)
{
    // auto updateExpression = [ctx](ir::Expression *id) -> ir::Expression * {
    //
    // };
    if (ast->IsMemberExpression()) {
        // i.e. we have following:
        //   enum Color {Red, Blue, Green, Yellow};
        //
        // so for such 'imcomplete' accessors like
        //   Color.Red
        //
        // we'd need to replace it with
        //   color.Red.getValue()
        if (auto *object = ast->AsMemberExpression()->Object(); object != nullptr) {
            // ..
            if (object->IsIdentifier() && object->AsIdentifier()->Variable() != nullptr) {
                // ..
                auto *checker = ctx->Checker()->AsETSChecker();
                auto *type = checker->GetTypeOfVariable(object->AsIdentifier()->Variable());
                ASSERT(type != nullptr);

                if (!type->IsETSEnum2Type()) {
                    std::cout << "[DEBUG] Not a ETSEnum2Type object!" << std::endl;
                    return ast;
                }
                std::cout << "[DEBUG] Found ETSEnum2Type identifier!" << std::endl;

                // auto *const scope = NearestScope(ast);
                // ASSERT(scope != nullptr);
                // auto expressionCtx = varbinder::LexicalScope<varbinder::Scope>::Enter(checker->VarBinder(), scope);

                // auto *parent = ast->Parent();
                auto *callExpr =
                    CreateCallExpression(ast->AsMemberExpression()->Clone(checker->Allocator(), nullptr), checker);
                // callExpr->SetParent(parent);
                //
                // InitScopesPhaseETS::RunExternalNode(callExpr, ctx->VarBinder());
                // checker->VarBinder()->AsETSBinder()->ResolveReferencesForScope(callExpr, scope);
                // callExpr->Check(checker);
                // ASSERT(callExpr->Variable() != nullptr);

                return callExpr;
            }
        }
    } else if (ast->IsIdentifier()) {
    } else {
        std::cout << "[DEBUG] Neither MemberExpression nor identifier!" << std::endl;
    }
    return ast;
}

ir::AstNode *UpdateMemberExpression(ir::AstNode *ast, CompilerContext *ctx)
{
    [[maybe_unused]] auto *checker = ctx->Checker()->AsETSChecker();
    auto *const scope = NearestScope(ast);
    ASSERT(scope != nullptr);
    auto expressionCtx = varbinder::LexicalScope<varbinder::Scope>::Enter(checker->VarBinder(), scope);

    ast->AsBinaryExpression()->SetLeft(checkBinaryBranch(
        ast->AsBinaryExpression()->Left()->Clone(checker->Allocator(), nullptr)->AsExpression(), ctx));
    ast->AsBinaryExpression()->SetRight(checkBinaryBranch(
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

    for (auto &[_, ext_programs] : program->ExternalSources()) {
        (void)_;
        for (auto *ext_prog : ext_programs) {
            Perform(ctx, ext_prog);
        }
    }

    program->Ast()->TransformChildrenRecursively([checker, ctx](ir::AstNode *ast) -> ir::AstNode * {
        if (ast->IsCallExpression()) {
            // check & update call expression with explicit cast to new type
            // e.g. for the following
            //
            // enum Color {Red, Blue, Green, Yellow};
            // function foo(a:Color) {..}
            // function main():void {
            //   foo(1);
            // }
            //
            // so the foo(1) should be translated to foo(Color.Blue)
        } else if (ast->IsBinaryExpression()) {
            if (ast->AsBinaryExpression()->IsPostBitSet(
                    ir::PostProcessingBits::ENUM_LOWERING_POST_PROCESSING_REQUIRED)) {
                return UpdateMemberExpression(ast, ctx->compilerContext);
            } else {
                // std::cout << __func__ << ":" << __LINE__ << ": [DEBUG] Skip this one, no need to update!" <<
                // std::endl;
                return ast;
            }
        } else if (ast->IsIfStatement() || ast->IsWhileStatement() || ast->IsDoWhileStatement() ||
                   ast->IsForUpdateStatement() || ast->IsConditionalExpression()) {
            // so far let's put this only for test script
            ir::Expression *test = nullptr;
            ir::Expression *testExpr = nullptr;

            if (ast->IsIfStatement())
                test = ast->AsIfStatement()->Test();
            if (ast->IsWhileStatement())
                test = ast->AsWhileStatement()->Test();
            if (ast->IsDoWhileStatement())
                test = ast->AsDoWhileStatement()->Test();
            if (ast->IsForUpdateStatement())
                test = ast->AsForUpdateStatement()->Test();
            if (ast->IsConditionalExpression())
                test = ast->AsConditionalExpression()->Test();

            if (test == nullptr) {
                return ast;
            }
            if (test->IsIdentifier()) {
                // we got simple variable test expression, test against  non-zero value
                if (test->AsIdentifier()->Variable() == nullptr) {
                    return ast;
                }
                auto *type = checker->GetTypeOfVariable(test->AsIdentifier()->Variable());
                ASSERT(type != nullptr);

                if (!type->IsETSEnum2Type()) {
                    return ast;
                }
                // ok now we need  to replace 'if (v)' to 'if (v.getValue() != 0)'
                // NOTE: what about string as enum constant?
                testExpr = CreateTestExpression(test->AsIdentifier(), ctx->compilerContext, nullptr);

            } else if (test->IsCallExpression()) {
                // simple call expression with default non-zero test, i.e.
                //
                //   if (v.getValue())
                //
                // this wll always be treated as 'true' since getValue() returns the EnumConst
                // object,but not the enum  value
                //
                // need  to checkif we're calling to getValue() for enum constant
                // and convert it into binary expression with '!= 0' test, i.e.
                //
                //   if (v.getValue() != 0)
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
                        if (!type->IsETSEnum2Type()) {
                            return ast;  // do not modify it,  it is not ETSEnum2Type
                        }

                        if (ir::Expression *prop = callee->AsMemberExpression()->Property(); prop != nullptr) {
                            if (prop->IsIdentifier() && (prop->AsIdentifier()->Name() == ENUM_GETVALUE_METHOD_NAME)) {
                                //  now we need tow rap it to the binary expression .. != 0
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
