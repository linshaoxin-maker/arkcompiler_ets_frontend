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

#include "ASTVerifier.h"

#include "es2panda.h"
#include "varbinder/scope.h"
#include "ir/astNode.h"
#include "ir/base/classDefinition.h"
#include "ir/expressions/identifier.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/forInStatement.h"
#include "ir/statements/forOfStatement.h"
#include "ir/statements/forUpdateStatement.h"
#include "ir/statements/variableDeclaration.h"

namespace panda::es2panda::compiler {

bool ASTVerifier::IsCorrectProgram(const parser::Program *program)
{
    bool is_correct = true;
    error_messages_.clear();

    for (auto *statement : program->Ast()->Statements()) {
        is_correct &= HaveParents(statement);
    }
    is_correct &= HaveParents(program->GlobalClass());

    for (auto *statement : program->Ast()->Statements()) {
        is_correct &= HaveTypes(statement);
    }
    is_correct &= HaveTypes(program->GlobalClass());

    for (auto *statement : program->Ast()->Statements()) {
        is_correct &= HaveVariables(statement);
    }
    is_correct &= HaveVariables(program->GlobalClass());

    for (auto *statement : program->Ast()->Statements()) {
        is_correct &= HaveScopes(statement);
    }
    is_correct &= HaveScopes(program->GlobalClass());

    for (auto *statement : program->Ast()->Statements()) {
        is_correct &= AreForLoopsCorrectInitialized(statement);
    }
    is_correct &= AreForLoopsCorrectInitialized(program->GlobalClass());

#ifndef NDEBUG
    std::for_each(error_messages_.begin(), error_messages_.end(), [](auto const msg) { LOG(INFO, COMMON) << msg; });
#endif  // NDEBUG
    return is_correct;
}

bool ASTVerifier::HasParent(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    if (ast->Parent() == nullptr) {
        error_messages_.push_back("NULL_PARENT: " + ast->DumpJSON());
        return false;
    }

    return true;
}

bool ASTVerifier::HaveParents(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    bool has_parent = HasParent(ast);
    ast->IterateRecursively([this, &has_parent](ir::AstNode *child) { has_parent &= HasParent(child); });
    return has_parent;
}

bool ASTVerifier::HasType(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    if (ast->IsTyped() && static_cast<const ir::TypedAstNode *>(ast)->TsType() == nullptr) {
        error_messages_.push_back("NULL_TS_TYPE: " + ast->DumpJSON());
        return false;
    }
    return true;
}

bool ASTVerifier::HaveTypes(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    bool has_type = HasType(ast);
    ast->IterateRecursively([this, &has_type](ir::AstNode *child) { has_type &= HasType(child); });
    return has_type;
}

bool ASTVerifier::HasVariable(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    if (!ast->IsIdentifier() || ast->AsIdentifier()->Variable() != nullptr) {
        return true;
    }

    error_messages_.push_back("NULL_VARIABLE: " + ast->AsIdentifier()->DumpJSON());
    return false;
}

bool ASTVerifier::HaveVariables(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    bool has_variable = HasVariable(ast);
    ast->IterateRecursively([this, &has_variable](ir::AstNode *child) { has_variable &= HasVariable(child); });
    return has_variable;
}

bool ASTVerifier::HasScope(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    if (!ast->IsIdentifier()) {
        return true;  // we will check only Identifier
    }

    // we will check only local variables of identifiers
    auto const *variable = ast->AsIdentifier()->Variable();
    if (!HasVariable(ast) || (variable->IsLocalVariable() && variable->AsLocalVariable()->GetScope() == nullptr)) {
        error_messages_.push_back("NULL_SCOPE_LOCAL_VAR: " + ast->DumpJSON());
        return false;
    }
    // NOTE(tatiana): Add check that the scope enclose this identifier
    return true;
}

bool ASTVerifier::HaveScopes(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    bool has_scope = HasScope(ast);
    ast->IterateRecursively([this, &has_scope](ir::AstNode *child) { has_scope &= HasScope(child); });
    return has_scope;
}

bool ASTVerifier::IsForLoopCorrectInitialized(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    if (ast->IsForInStatement()) {
        auto const *left = ast->AsForInStatement()->Left();
        if (left == nullptr) {
            error_messages_.push_back("NULL FOR-IN-LEFT: " + ast->DumpJSON());
            return false;
        }

        if (!left->IsIdentifier() && !left->IsVariableDeclaration()) {
            error_messages_.push_back("INCORRECT FOR-IN-LEFT: " + ast->DumpJSON());
            return false;
        }
    }

    if (ast->IsForOfStatement()) {
        auto const *left = ast->AsForOfStatement()->Left();
        if (left == nullptr) {
            error_messages_.push_back("NULL FOR-OF-LEFT: " + ast->DumpJSON());
            return false;
        }

        if (!left->IsIdentifier() && !left->IsVariableDeclaration()) {
            error_messages_.push_back("INCORRECT FOR-OF-LEFT: " + ast->DumpJSON());
            return false;
        }
    }

    if (ast->IsForUpdateStatement()) {
        // The most important part of for-loop is the test.
        // But it also can be null. Then there must be break;(return) in the body.
        auto const *test = ast->AsForUpdateStatement()->Test();
        if (test == nullptr) {
            auto const *body = ast->AsForUpdateStatement()->Body();
            if (body == nullptr) {
                error_messages_.push_back("NULL FOR-TEST AND FOR-BODY: " + ast->DumpJSON());
                return false;
            }
            bool has_exit = body->IsBreakStatement() || body->IsReturnStatement();
            body->IterateRecursively([&has_exit](ir::AstNode *child) {
                has_exit |= child->IsBreakStatement() || child->IsReturnStatement();
            });
            if (!has_exit) {
                // an infinite loop
                error_messages_.push_back("WARNING: NULL FOR-TEST AND FOR-BODY doesn't exit: " + ast->DumpJSON());
            }
            return true;
        }

        if (test->IsExpression()) {
            auto const *var = test->AsExpression();
            if (var == nullptr) {
                error_messages_.push_back("NULL FOR VAR: " + ast->DumpJSON());
                return false;
            }
        }
    }
    return true;
}

bool ASTVerifier::AreForLoopsCorrectInitialized(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    bool is_for_initialized = IsForLoopCorrectInitialized(ast);
    ast->IterateRecursively(
        [this, &is_for_initialized](ir::AstNode *child) { is_for_initialized &= IsForLoopCorrectInitialized(child); });
    return is_for_initialized;
}

}  // namespace panda::es2panda::compiler
