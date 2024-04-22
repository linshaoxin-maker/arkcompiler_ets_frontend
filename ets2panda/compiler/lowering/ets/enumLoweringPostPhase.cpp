/*
 * Copyright (c) 2021 - 2024 Huawei Device Co., Ltd.
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

static ir::CallExpression *CreateCallExpression(ir::Expression *id, checker::ETSChecker *checker)
{
    auto *const callee = checker->AllocNode<ir::Identifier>("valueOf", checker->Allocator());
    callee->SetReference();
    ir::Expression *const accessor =
        checker->AllocNode<ir::MemberExpression>(id, callee, ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    id->SetParent(accessor);
    callee->SetParent(accessor);

    auto *callExpression = checker->AllocNode<ir::CallExpression>(
        accessor, ArenaVector<ir::Expression *>(checker->Allocator()->Adapter()), nullptr, false);

    callExpression->Check(checker);
    accessor->SetParent(callExpression);

    return callExpression;
}

ir::Expression *CheckBinaryBranch(ir::Expression *ast, CompilerContext *ctx)
{
    // clang-format off
    auto updateExpression = [ctx](ir::Expression *object, ir::Expression *node) -> ir::Expression* {
        // clang-format on
        if (object->IsIdentifier() && object->AsIdentifier()->Variable() != nullptr) {
            auto *checker = ctx->Checker()->AsETSChecker();
            auto *type = checker->GetTypeOfVariable(object->AsIdentifier()->Variable());
            ASSERT(type != nullptr);

            if (!type->IsETSEnumType()) {
                return node;
            }

            auto *callExpr = CreateCallExpression(node, checker);

            return callExpr;
        }
        return node;
    };

    if (ast->IsMemberExpression()) {
        // i.e. we have following:
        //   'enum Color {Red, Blue, Green, Yellow}'
        //
        // so for such 'imcomplete' accessors like:
        //   'Color.Red'
        //
        // we'd need to replace it with
        //   'color.Red.valueOf()'

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
    auto *checker = ctx->Checker()->AsETSChecker();
    ast->AsBinaryExpression()->SetLeft(CheckBinaryBranch(ast->AsBinaryExpression()->Left(), ctx));
    ast->AsBinaryExpression()->SetRight(CheckBinaryBranch(ast->AsBinaryExpression()->Right(), ctx));
    ast->AsExpression()->SetTsType(nullptr);  // force recheck
    ast->Check(checker);
    return ast;
}

ir::Expression *GetTestExpression(ir::AstNode *ast)
{
    ASSERT(ast != nullptr);
    ir::Expression *test = nullptr;

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
    } else if (ast->IsAssertStatement()) {
        test = ast->AsAssertStatement()->Test();
    } else {
        UNREACHABLE();
    }

    return test;
}

void SetTestExpression(ir::Expression *testExpr, ir::AstNode *ast)
{
    ASSERT(testExpr != nullptr);

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
    } else if (ast->IsAssertStatement()) {
        ast->AsAssertStatement()->SetTest(testExpr);
    } else {
        UNREACHABLE();
    }
    testExpr->SetParent(ast);
}

ir::AstNode *UpdateTestNode(ir::AstNode *ast, public_lib::Context *ctx)
{
    checker::ETSChecker *checker = ctx->checker->AsETSChecker();
    ir::Expression *testExpr = nullptr;
    ir::Expression *test = GetTestExpression(ast);
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
        // ok now we need  to replace 'if (v)' to 'if (v.valueOf())'
        testExpr = CreateCallExpression(test->AsIdentifier(), checker);
    }

    if (testExpr == nullptr) {
        return ast;
    }

    SetTestExpression(testExpr, ast);
    ast->Check(checker);
    return ast;
}

static ir::AstNode *UpdateBinaryExpression(ir::AstNode *ast, checker::ETSChecker *checker)
{
    auto parent = ast->Parent()->AsBinaryExpression();
    if (!parent->IsLogicalExtended()) {
        return ast;
    }
    bool left = false;
    if (parent->Left() == ast) {
        left = true;
    }
    auto callexpr = CreateCallExpression(ast->AsIdentifier(), checker);

    if (left) {
        parent->SetLeft(callexpr);
    } else {
        parent->SetRight(callexpr);
    }

    parent->SetTsType(nullptr);  // force recheck
    parent->Check(checker);
    return callexpr;
}

static ir::AstNode *UpdateUnaryExpression(ir::AstNode *ast, checker::ETSChecker *checker)
{
    auto parent = ast->Parent()->AsUnaryExpression();
    if (parent->OperatorType() != lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK) {
        return ast;
    }
    auto callexpr = CreateCallExpression(ast->AsIdentifier(), checker);
    parent->SetArgument(callexpr);

    parent->SetTsType(nullptr);  // force recheck
    parent->Check(checker);
    return callexpr;
}

static ir::AstNode *UpdateConditionalExpression(ir::AstNode *ast, checker::ETSChecker *checker)
{
    auto parent = ast->Parent()->AsConditionalExpression();
    bool test = false;
    bool consequent = false;

    if (parent->Test() == ast) {
        test = true;
    } else if (parent->Consequent() == ast) {
        consequent = true;
    }

    auto callexpr = CreateCallExpression(ast->AsIdentifier(), checker);

    if (test) {
        parent->SetTest(callexpr);
    } else if (consequent) {
        parent->SetConsequent(callexpr);
    } else {
        parent->SetAlternate(callexpr);
    }

    parent->SetTsType(nullptr);
    parent->Check(checker);
    return callexpr;
}

static ir::AstNode *UpdateAssignmentExpression(ir::AstNode *ast, checker::ETSChecker *checker)
{
    auto parent = ast->Parent()->AsAssignmentExpression();
    auto left = parent->Left();
    if (left == ast) {
        return ast;
    }
    if (left->TsType()->IsETSEnumType()) {
        return ast;
    }
    if (!(left->TsType()->HasTypeFlag(checker::TypeFlag::ETS_NUMERIC) ||
          left->TsType()->HasTypeFlag(checker::TypeFlag::STRING))) {
        return ast;
    }

    auto callexpr = CreateCallExpression(ast->AsIdentifier(), checker);
    parent->SetRight(callexpr);
    parent->SetTsType(nullptr);
    parent->Check(checker);
    return callexpr;
}

ir::AstNode *UpdateIdentifierNode(ir::AstNode *ast, public_lib::Context *ctx)
{
    checker::ETSChecker *checker = ctx->checker->AsETSChecker();
    if (ast->AsIdentifier()->Variable() == nullptr) {
        return ast;
    }

    auto *type = ast->AsIdentifier()->Variable()->TsType();
    if (type == nullptr || !type->IsETSEnumType()) {
        return ast;
    }

    // NOTE (psiket) Not sure how complete this is
    if (ast->Parent()->IsBinaryExpression()) {
        return UpdateBinaryExpression(ast, checker);
    }

    if (ast->Parent()->IsUnaryExpression()) {
        return UpdateUnaryExpression(ast, checker);
    }

    if (ast->Parent()->IsConditionalExpression()) {
        return UpdateConditionalExpression(ast, checker);
    }

    if (ast->Parent()->IsAssignmentExpression()) {
        return UpdateAssignmentExpression(ast, checker);
    }

    return ast;
}

bool EnumLoweringPostPhase::Perform(public_lib::Context *ctx, parser::Program *program)
{
    if (program->Extension() != ScriptExtension::ETS) {
        return true;
    }

    for (auto &[_, extPrograms] : program->ExternalSources()) {
        (void)_;
        for (auto *extProg : extPrograms) {
            Perform(ctx, extProg);
        }
    }

    // clang-format off
    program->Ast()->TransformChildrenRecursively([ctx](ir::AstNode *ast) -> ir::AstNode* {
        if (ast->IsBinaryExpression()) {
            // NOTE (psiket) Why is the ENUM_LOWERING_POST_PROCESSING_REQUIRED flag set for
            // the binary expression only?
            if (ast->AsBinaryExpression()->IsPostBitSet(
                ir::PostProcessingBits::ENUM_LOWERING_POST_PROCESSING_REQUIRED)) {
                return UpdateMemberExpression(ast, ctx->compilerContext);
            }
            return ast;
        }
        // clang-format on

        if (ast->IsIfStatement() || ast->IsWhileStatement() || ast->IsDoWhileStatement() ||
            ast->IsForUpdateStatement() || ast->IsConditionalExpression() || ast->IsAssertStatement()) {
            return UpdateTestNode(ast, ctx);
        }

        if (ast->IsIdentifier()) {
            return UpdateIdentifierNode(ast, ctx);
        }

        return ast;
    });
    return true;
}
}  // namespace ark::es2panda::compiler
