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

ir::AstNode *CreateCallExpression(ir::AstNode *ast, CompilerContext *ctx)
{
    // only identifiers allowed here!
    ASSERT(ast->IsIfStatement() && ast->AsIfStatement()->Test()->IsIdentifier());

    auto *checker = ctx->Checker()->AsETSChecker();
    [[maybe_unused]] auto *id =
        checker->AllocNode<ir::Identifier>(ast->AsIfStatement()->Test()->AsIdentifier()->Name(),
                                           checker->Allocator());  // ast->AsIfStatement()->Test()->AsIdentifier();
    id->SetReference();
    auto *const callee = checker->AllocNode<ir::Identifier>(util::StringView("getValue"), checker->Allocator());
    callee->SetReference();

    ir::Expression *const accessor =
        checker->AllocNode<ir::MemberExpression>(id,  // ast->AsIfStatement()->Test()->AsIdentifier(),
                                                 callee, ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    id->SetParent(accessor);
    callee->SetParent(accessor);

    ir::CallExpression *callExpression = checker->AllocNode<ir::CallExpression>(
        accessor, ArenaVector<ir::Expression *>(checker->Allocator()->Adapter()), nullptr, false);
    accessor->SetParent(callExpression);

    auto *right = checker->AllocNode<ir::NumberLiteral>(util::StringView("0"));
    ir::Expression *testExpr =
        checker->AllocNode<ir::BinaryExpression>(callExpression, right, lexer::TokenType::PUNCTUATOR_NOT_EQUAL);
    // testExpr->SetTsType(testExpr->Check(checker));
    callExpression->SetParent(testExpr);
    right->SetParent(testExpr);

    auto *test = checker->AllocNode<ir::IfStatement>(testExpr, ast->AsIfStatement()->Consequent(),
                                                     ast->AsIfStatement()->Alternate());
    testExpr->SetParent(test);
    return test;
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
        } else if (ast->IsIfStatement()) {
            // so far let's put this only for test script
            if (program->SourceFile().GetFileName() == util::StringView("40.01")) {
                std::cout << "File: " << program->SourceFile().GetFileName()
                          << ": test statement: " << ast->AsIfStatement()->Test()->DumpJSON() << std::endl;
                auto *test = ast->AsIfStatement()->Test();
                if (test->IsIdentifier()) {
                    std::cout << "Found Identifier:  " << test->AsIdentifier()->Name() << std::endl;
                    // we got simple variable test expression, test against  non-zero value
                    ASSERT(test->AsIdentifier()->Variable() != nullptr);
                    auto *type = checker->GetTypeOfVariable(test->AsIdentifier()->Variable());
                    ASSERT(type != nullptr);
                    if (type->IsETSEnum2Type()) {
                        std::cout << "Found type: ETSEnum2Type: " << type << std::endl;
                        // ok now we need  to replace 'if (v)' to 'if (v.getValue() != 0)'
                        // NOTE: what about string as enum constant?
                        auto *parent = ast->Parent();
                        auto *const scope = NearestScope(test);
                        auto expressionCtx =
                            varbinder::LexicalScope<varbinder::Scope>::Enter(checker->VarBinder(), scope);
                        auto *node = CreateCallExpression(ast, ctx->compilerContext);
                        node->SetParent(parent);
                        std::cout << "Updated node: " << node->DumpJSON() << std::endl;

                        InitScopesPhaseETS::RunExternalNode(node, ctx->compilerContext->VarBinder());

                        checker->VarBinder()->AsETSBinder()->ResolveReferencesForScope(node, scope);
                        node->Check(checker);
                        return node;
                    } else {
                        // ..
                    }
                } else if (test->IsCallExpression()) {
                    // simple callexpression with default non-zero test
                    // need  to checkif we'recalling to getValue() fo enum constant
                }

                // check & update conditional expression with explicit cast to new type
                // e.g. for the following
                //
                // enum Color {Red, Blue, Green, Yellow};
                // function main():void {
                //   let v: Color = Color.Blue;
                //   if (v) {...}
                // }
                //
                // so the if (v) should be translated to the if (v.getValue() == true) ??
                //
                [[maybe_unused]] auto *consequent = ast->AsIfStatement()->Consequent();
                [[maybe_unused]] auto *alternate = ast->AsIfStatement()->Alternate();
                // std::cout << "File name: " << program->SourceFile().GetFileName() << std::endl;
                if (0 && program->SourceFile().GetFileName().Compare(util::StringView("40.01")) == 0) {
                    std::cout << program->SourceFile().GetFileName()
                              << ": conditional expression: " << ast->AsIfStatement()->DumpJSON() << std::endl;
                    if (0 && consequent != nullptr)
                        std::cout << "consequent: " << consequent->DumpJSON() << std::endl;
                    if (0 && alternate != nullptr)
                        std::cout << "alternate: " << alternate->DumpJSON() << std::endl;
                }
            }
        }
        return ast;
    });

    return true;
}

}  // namespace ark::es2panda::compiler
