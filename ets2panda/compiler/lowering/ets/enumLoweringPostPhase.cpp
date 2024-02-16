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

ir::CallExpression *CreateCallExpression(ir::Identifier *id, checker::ETSChecker *checker)
{
    auto *const callee = checker->AllocNode<ir::Identifier>(util::StringView("getValue"), checker->Allocator());
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

ir::Expression *CreateTestExpression(ir::Identifier *id, checker::ETSChecker *checker, CompilerContext *ctx)
{
    auto *const scope = NearestScope(id);
    ASSERT(scope != nullptr);
    auto expressionCtx = varbinder::LexicalScope<varbinder::Scope>::Enter(checker->VarBinder(), scope);
    auto *callExpression = CreateCallExpression(id, checker);
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

ir::AstNode *CreateCallExpression_if(ir::AstNode *ast, CompilerContext *ctx)
{
    // only identifiers allowed here!
    ASSERT(ast->IsIfStatement() && ast->AsIfStatement()->Test()->IsIdentifier());

    auto *checker = ctx->Checker()->AsETSChecker();
    auto *id = ast->AsIfStatement()->Test()->AsIdentifier();
    // auto *id =
    //     checker->AllocNode<ir::Identifier>(ast->AsIfStatement()->Test()->AsIdentifier()->Name(),
    //     checker->Allocator());
    // id->SetReference();

    auto *testExpr = CreateTestExpression(id, checker, ctx);

    // auto *test = checker->AllocNode<ir::IfStatement>(testExpr, ast->AsIfStatement()->Consequent(),
    //                                                  ast->AsIfStatement()->Alternate());
    ast->AsIfStatement()->SetTest(testExpr);
    testExpr->SetParent(ast);
    return ast;
}

ir::AstNode *CreateCallExpression_while(ir::AstNode *ast, CompilerContext *ctx)
{
    // only identifiers allowed here!
    ASSERT(ast->IsWhileStatement() && ast->AsWhileStatement()->Test()->IsIdentifier());

    auto *checker = ctx->Checker()->AsETSChecker();
    auto *id = ast->AsWhileStatement()->Test()->AsIdentifier();
    // auto *id = checker->AllocNode<ir::Identifier>(
    //     ast->AsWhileStatement()->Test()->AsIdentifier()->Name(), checker->Allocator());
    // id->SetReference();

    auto *testExpr = CreateTestExpression(id, checker, ctx);

    // auto *expr = checker->AllocNode<ir::WhileStatement>(testExpr, ast->AsWhileStatement()->Body());
    ast->AsWhileStatement()->SetTest(testExpr);
    testExpr->SetParent(ast);
    return ast;
}

ir::AstNode *CreateCallExpression_do_while(ir::AstNode *ast, CompilerContext *ctx)
{
    // only identifiers allowed here!
    ASSERT(ast->IsDoWhileStatement() && ast->AsDoWhileStatement()->Test()->IsIdentifier());

    auto *checker = ctx->Checker()->AsETSChecker();
    auto *id = ast->AsDoWhileStatement()->Test()->AsIdentifier();
    // auto *id = checker->AllocNode<ir::Identifier>(
    //     ast->AsDoWhileStatement()->Test()->AsIdentifier()->Name(), checker->Allocator());
    // id->SetReference();

    auto *testExpr = CreateTestExpression(id, checker, ctx);

    // auto *expr = checker->AllocNode<ir::DoWhileStatement>(ast->AsDoWhileStatement()->Body(), testExpr);
    ast->AsDoWhileStatement()->SetTest(testExpr);
    testExpr->SetParent(ast);
    return ast;
}

ir::AstNode *CreateCallExpression_for_update(ir::AstNode *ast, CompilerContext *ctx)
{
    // only identifiers allowed here!
    ASSERT(ast->IsForUpdateStatement() && ast->AsForUpdateStatement()->Test()->IsIdentifier());

    auto *checker = ctx->Checker()->AsETSChecker();
    auto *id = ast->AsForUpdateStatement()->Test()->AsIdentifier();
    // auto *id = checker->AllocNode<ir::Identifier>(ast->AsForUpdateStatement()->Test()->AsIdentifier()->Name(),
    //                                               checker->Allocator());
    // id->SetReference();

    auto *testExpr = CreateTestExpression(id, checker, ctx);

    // auto *expr = checker->AllocNode<ir::ForUpdateStatement>(ast->AsForUpdateStatement()->Init(), testExpr,
    //                                                         ast->AsForUpdateStatement()->Update(),
    //                                                         ast->AsForUpdateStatement()->Body());
    // testExpr->SetParent(expr);
    ast->AsForUpdateStatement()->SetTest(testExpr);
    testExpr->SetParent(ast);
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

    program->Ast()->TransformChildrenRecursively([checker, ctx, program](ir::AstNode *ast) -> ir::AstNode * {
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
            // auto *rightExpr = AllocNode<ir::BinaryExpression>(left, rightExpr, operatorType);
            [[maybe_unused]] auto *left = ast->AsBinaryExpression()->Left();
            [[maybe_unused]] auto *right = ast->AsBinaryExpression()->Right();

        } else if (ast->IsIfStatement() || ast->IsWhileStatement() || ast->IsDoWhileStatement() ||
                   ast->IsForUpdateStatement()) {
            // so far let's put this only for test script
            const ir::Expression *test = nullptr;
            if (ast->IsIfStatement())
                test = ast->AsIfStatement()->Test();
            if (ast->IsWhileStatement())
                test = ast->AsWhileStatement()->Test();
            if (ast->IsDoWhileStatement())
                test = ast->AsDoWhileStatement()->Test();
            if (ast->IsForUpdateStatement())
                test = ast->AsForUpdateStatement()->Test();

            if (0) {
                std::cout << "File: " << program->SourceFile().GetFileName() << ": test statement: " << test->DumpJSON()
                          << std::endl;
            }
            if (test->IsIdentifier()) {
                if (0) {
                    std::cout << "Found Identifier:  " << test->AsIdentifier()->Name() << std::endl;
                }
                // we got simple variable test expression, test against  non-zero value
                // ASSERT(test->AsIdentifier()->Variable() != nullptr);
                if (test->AsIdentifier()->Variable() == nullptr) {
                    return ast;
                }
                auto *type = checker->GetTypeOfVariable(test->AsIdentifier()->Variable());
                ASSERT(type != nullptr);
                if (type->IsETSEnum2Type()) {
                    if (0) {
                        std::cout << "Found type: ETSEnum2Type: " << type << std::endl;
                    }
                    // ok now we need  to replace 'if (v)' to 'if (v.getValue() != 0)'
                    // NOTE: what about string as enum constant?
                    ir::AstNode *node = nullptr;
                    if (ast->IsIfStatement()) {
                        node = CreateCallExpression_if(ast, ctx->compilerContext);
                    } else if (ast->IsWhileStatement()) {
                        node = CreateCallExpression_while(ast, ctx->compilerContext);
                    } else if (ast->IsDoWhileStatement()) {
                        node = CreateCallExpression_do_while(ast, ctx->compilerContext);
                    } else if (ast->IsForUpdateStatement()) {
                        node = CreateCallExpression_for_update(ast, ctx->compilerContext);
                    }

                    if (node == nullptr) {
                        std::cout << "ERRPR: can't create proper substitution!" << std::endl;
                        return ast;
                    }
                    if (0)
                        std::cout << "Updated node: " << node->DumpJSON() << std::endl;

                    return node;
                } else {
                    // ..
                }
            } else if (test->IsCallExpression()) {
                // simple call expression with default non-zero test, i.e.
                //
                //   if (v.getValue())
                //
                // this iwll always be treated as 'true' sine getValue() returns the EnumConst
                // object,but not the enum  value
                //
                // need  to checkif we're calling to getValue() for enum constant
                // and convert it intobinary expression with '!= 0' test, i.e.
                //
                //   if (v.getValue() != 0)
            }
        }
        return ast;
    });

    return true;
}

}  // namespace ark::es2panda::compiler
