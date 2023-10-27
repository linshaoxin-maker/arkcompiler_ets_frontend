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
#include <algorithm>
#include <iterator>

#include "es2panda.h"
#include "varbinder/variableFlags.h"
#include "varbinder/scope.h"
#include "ir/astNode.h"
#include "ir/base/catchClause.h"
#include "ir/base/classDefinition.h"
#include "ir/base/classStaticBlock.h"
#include "ir/base/methodDefinition.h"
#include "ir/base/scriptFunction.h"
#include "ir/ets/etsFunctionType.h"
#include "ir/ets/etsNewClassInstanceExpression.h"
#include "ir/ets/etsPackageDeclaration.h"
#include "ir/ets/etsParameterExpression.h"
#include "ir/ets/etsTypeReference.h"
#include "ir/ets/etsTypeReferencePart.h"
#include "ir/expressions/callExpression.h"
#include "ir/expressions/sequenceExpression.h"
#include "ir/expressions/functionExpression.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/memberExpression.h"
#include "ir/expressions/literals/numberLiteral.h"
#include "ir/expressions/literals/stringLiteral.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/classDeclaration.h"
#include "ir/statements/expressionStatement.h"
#include "ir/statements/throwStatement.h"
#include "ir/statements/tryStatement.h"
#include "ir/statements/variableDeclaration.h"
#include "ir/statements/variableDeclarator.h"
#include "ir/ts/tsClassImplements.h"
#include "ir/ts/tsTypeParameter.h"
#include "ir/ts/tsTypeParameterDeclaration.h"
#include "ir/ts/tsTypeParameterInstantiation.h"
#include "util/ustring.h"
#include "utils/arena_containers.h"

namespace panda::es2panda::compiler {

template <typename Func>
ASTVerifier::CheckFunction RecursiveCheck(const Func &func)
{
    return [func](const ir::AstNode *ast) -> bool {
        bool has_parent = func(ast);
        ast->IterateRecursively([func, &has_parent](ir::AstNode *child) { has_parent &= func(child); });
        return has_parent;
    };
}

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define ADD_CHECK(Name)                                                                        \
    {                                                                                          \
        const auto check = [this](const ir::AstNode *ast) -> bool { return this->Name(ast); }; \
        checks_.emplace_back(NamedCheck {#Name, check});                                       \
        all_checks_.insert(#Name);                                                             \
        checks_.emplace_back(NamedCheck {#Name "Recursive", RecursiveCheck(check)});           \
        all_checks_.insert(#Name "Recursive");                                                 \
    }
// NOLINTEND(cppcoreguidelines-macro-usage)

ASTVerifier::ASTVerifier(ArenaAllocator *allocator, util::StringView source_code)
    : allocator_ {allocator},
      named_errors_ {allocator_->Adapter()},
      encountered_errors_ {allocator_->Adapter()},
      checks_ {allocator_->Adapter()},
      all_checks_(allocator_->Adapter())
{
    if (!source_code.Empty()) {
        index_.emplace(source_code);
    }

    ADD_CHECK(HasParent);
    ADD_CHECK(HasType);
    ADD_CHECK(HasVariable);
    ADD_CHECK(HasScope);
}

bool ASTVerifier::VerifyFull(const ir::AstNode *ast)
{
    return Verify(ast, all_checks_);
}

bool ASTVerifier::Verify(const ir::AstNode *ast, const CheckSet &check_set)
{
    bool is_correct = true;
    auto check_and_report = [&is_correct, this](util::StringView name, const CheckFunction &check,
                                                const ir::AstNode *node) {
        if (node == nullptr) {
            return;
        }

        is_correct &= check(node);
        if (!is_correct) {
            for (const auto &error : encountered_errors_) {
                named_errors_.emplace_back(NamedError {name, error});
            }
            encountered_errors_.clear();
        }
    };

    const auto contains_checks =
        std::includes(all_checks_.begin(), all_checks_.end(), check_set.begin(), check_set.end());
    if (!contains_checks) {
        auto invalid_checks = CheckSet {allocator_->Adapter()};
        for (const auto &check : check_set) {
            if (all_checks_.find(check) == all_checks_.end()) {
                invalid_checks.insert(check);
            }
        }
        for (const auto &check : invalid_checks) {
            const auto &message = check.Mutf8() + " check is not found";
            named_errors_.emplace_back(NamedError {"Check", Error {message, lexer::SourceLocation {}}});
        }
    }

    for (const auto &[name, check] : checks_) {
        if (check_set.find(name) != check_set.end()) {
            check_and_report(name, check, ast);
        }
    }

    return is_correct;
}

std::string ToStringHelper(const varbinder::ScopeType type)
{
    switch (type) {
        case varbinder::ScopeType::CATCH: {
            return "CATCH";
        }
        case varbinder::ScopeType::CATCH_PARAM: {
            return "CATCH_PARAM";
        }
        case varbinder::ScopeType::CLASS: {
            return "CLASS";
        }
        case varbinder::ScopeType::FUNCTION: {
            return "FUNCTION";
        }
        case varbinder::ScopeType::FUNCTION_PARAM: {
            return "FUNCTION_PARAM";
        }
        case varbinder::ScopeType::GLOBAL: {
            return "GLOBAL";
        }
        case varbinder::ScopeType::LOCAL: {
            return "LOCAL";
        }
        case varbinder::ScopeType::LOOP: {
            return "LOOP";
        }
        case varbinder::ScopeType::LOOP_DECL: {
            return "LOOP_DECL";
        }
        case varbinder::ScopeType::MODULE: {
            return "MODULE";
        }
        case varbinder::ScopeType::PARAM: {
            return "PARAM";
        }
        default: {
            return "MUST BE UNREACHABLE";
        }
    }
}

std::string ToStringHelper(const util::StringView &name)
{
    return name == nullptr ? "<null>" : name.Mutf8();
}

std::string ToStringHelper(const varbinder::Scope *scope)
{
    if (scope == nullptr) {
        return "<null>";
    }

    switch (scope->Type()) {
        case varbinder::ScopeType::FUNCTION: {
            return "FUNC_SCOPE " + ToStringHelper(scope->AsFunctionScope()->Name());
        }
        case varbinder::ScopeType::LOCAL: {
            return "LOCAL_SCOPE ";
        }
        case varbinder::ScopeType::CATCH: {
            return "CATCH_SCOPE ";
        }
        default: {
            return "MUST BE UNREACHABLE";
        }
    }
}

std::string ToStringHelper(const varbinder::Variable *var)
{
    if (var == nullptr) {
        return "<null>";
    }

    switch (var->Type()) {
        case varbinder::VariableType::LOCAL: {
            return "LOCAL_VAR " + ToStringHelper(var->Name());
        }
        case varbinder::VariableType::MODULE: {
            return "MODULE_VAR " + ToStringHelper(var->Name());
        }
        case varbinder::VariableType::GLOBAL: {
            return "GLOBAL_VAR " + ToStringHelper(var->Name());
        }
        case varbinder::VariableType::ENUM: {
            return "ENUM_VAR " + ToStringHelper(var->Name());
        }
        default: {
            return "MUST BE UNREACHABLE";
        }
    }
}

template <typename T>
std::string ToStringParamsHelper(const ir::AstNode *parent, const ArenaVector<T *> &params)
{
    std::string name;
    if (parent != nullptr) {
        name = ToStringHelper(parent) + " ";
    }

    name += "(";
    for (auto const *param : params) {
        name += ToStringHelper(param);
    }

    return name + ")";
}

std::string ToStringHelper(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return "<null>";
    }

    switch (ast->Type()) {
        case ir::AstNodeType::IDENTIFIER: {
            return "ID " + ToStringHelper(ast->AsIdentifier()->Name());
        }
        case ir::AstNodeType::CLASS_DEFINITION: {
            return "CLS_DEF " + ToStringHelper(ast->AsClassDefinition()->Ident());
        }
        case ir::AstNodeType::CLASS_DECLARATION: {
            return "CLS_DECL " + ToStringHelper(ast->AsClassDeclaration()->Definition());
        }
        case ir::AstNodeType::BLOCK_STATEMENT: {
            return "BLOCK " + ToStringHelper(ast->AsBlockStatement()->Scope());
        }
        case ir::AstNodeType::SCRIPT_FUNCTION: {
            auto const *sf = ast->AsScriptFunction();
            return "SCRIPT_FUN " + ToStringHelper(sf->Scope()) + "::" + ToStringHelper(sf->Id());
        }
        case ir::AstNodeType::FUNCTION_EXPRESSION: {
            return "FUN_EXPR " + ToStringHelper(ast->AsFunctionExpression()->Function());
        }
        case ir::AstNodeType::METHOD_DEFINITION: {
            return "METHOD_DEF " + ToStringHelper(ast->AsMethodDefinition()->Value());
        }
        case ir::AstNodeType::ETS_TYPE_REFERENCE_PART: {
            return "TYPE_REF_PART " + ToStringHelper(ast->AsETSTypeReferencePart()->Name());
        }
        case ir::AstNodeType::ETS_TYPE_REFERENCE: {
            return "TYPE_REF " + ToStringHelper(ast->AsETSTypeReference()->Part());
        }
        case ir::AstNodeType::VARIABLE_DECLARATOR: {
            return "VAR_DECLARATOR " + ToStringHelper(ast->AsVariableDeclarator()->Id());
        }
        case ir::AstNodeType::VARIABLE_DECLARATION: {
            if (ast->AsVariableDeclaration()->Declarators().empty()) {
                return "VAR_DECLARATION <null>";
            }
            return "VAR_DECLARATION " + ToStringHelper(ast->AsVariableDeclaration()->Declarators().at(0));
        }
        case ir::AstNodeType::CALL_EXPRESSION: {
            return "CALL_EXPR " + ToStringHelper(ast->AsCallExpression()->Callee()) + "(...)";
        }
        case ir::AstNodeType::EXPRESSION_STATEMENT: {
            return "EXPR_STMT " + ToStringHelper(ast->AsExpressionStatement()->GetExpression());
        }
        case ir::AstNodeType::MEMBER_EXPRESSION: {
            auto const *me = ast->AsMemberExpression();
            return "MEMBER_EXPR " + ToStringHelper(me->Object()) + "." + ToStringHelper(me->Property());
        }
        case ir::AstNodeType::CLASS_STATIC_BLOCK: {
            return "CLS_STATIC_BLOCK " + ToStringHelper(ast->AsClassStaticBlock()->Function());
        }
        case ir::AstNodeType::ETS_PACKAGE_DECLARATION: {
            return "PKG_DECL ";
        }
        case ir::AstNodeType::TS_TYPE_PARAMETER_DECLARATION: {
            auto params = ast->AsTSTypeParameterDeclaration()->Params();
            return "PARAM_DECL " + ToStringParamsHelper<ir::TSTypeParameter>(ast->Parent(), params);
        }
        case ir::AstNodeType::TS_TYPE_PARAMETER: {
            return "TYPE_PARAM " + ToStringHelper(ast->AsTSTypeParameter()->Name());
        }
        case ir::AstNodeType::TS_TYPE_PARAMETER_INSTANTIATION: {
            return "PARAM_INSTANTIATION " +
                   ToStringParamsHelper<ir::TypeNode>(ast->Parent(), ast->AsTSTypeParameterInstantiation()->Params());
        }
        case ir::AstNodeType::THROW_STATEMENT: {
            return "THROW_STMT " + ToStringHelper(ast->AsThrowStatement()->Argument());
        }
        case ir::AstNodeType::ETS_NEW_CLASS_INSTANCE_EXPRESSION: {
            return "NEW_CLS_INSTANCE " + ToStringHelper(ast->AsETSNewClassInstanceExpression()->GetTypeRef());
        }
        case ir::AstNodeType::STRING_LITERAL: {
            return "STR_LITERAL " + ToStringHelper(ast->AsStringLiteral()->Str());
        }
        case ir::AstNodeType::TRY_STATEMENT: {
            return "TRY_STMT " + ToStringHelper(ast->AsTryStatement()->Block());
        }
        case ir::AstNodeType::CATCH_CLAUSE: {
            return "CATCH_CLAUSE ";
        }
        case ir::AstNodeType::NUMBER_LITERAL: {
            return "NUMBER_LITERAL " + ToStringHelper(ast->AsNumberLiteral()->Str());
        }
        case ir::AstNodeType::ETS_PARAMETER_EXPRESSION: {
            return "ETS_PARAM_EXPR " + ToStringHelper(ast->AsETSParameterExpression()->Ident());
        }
        case ir::AstNodeType::TS_INTERFACE_DECLARATION: {
            return "TS_INTERFACE_DECL " + ToStringHelper(ast->AsTSInterfaceDeclaration()->Id());
        }
        case ir::AstNodeType::TS_INTERFACE_BODY: {
            return "TS_INTERFACE_BODY ";
        }
        case ir::AstNodeType::ETS_FUNCTION_TYPE: {
            return "ETS_FUNC_TYPE " +
                   ToStringParamsHelper<ir::Expression>(ast->Parent(), ast->AsETSFunctionType()->Params());
        }
        case ir::AstNodeType::TS_CLASS_IMPLEMENTS: {
            return "TS_CLASS_IMPL " + ToStringHelper(ast->AsTSClassImplements()->Expr());
        }
        default: {
            return "MUST BE UNREACHABLE";
        }
    }
}

bool ASTVerifier::HasParent(const ir::AstNode *ast)
{
    if (ast->Parent() == nullptr) {
        AddError("NULL_PARENT: " + ToStringHelper(ast), ast->Start());
        return false;
    }

    return true;
}

bool ASTVerifier::HasType(const ir::AstNode *ast)
{
    if (ast->IsTyped() && static_cast<const ir::TypedAstNode *>(ast)->TsType() == nullptr) {
        AddError("NULL_TS_TYPE: " + ToStringHelper(ast), ast->Start());
        return false;
    }
    return true;
}

bool ASTVerifier::HasVariable(const ir::AstNode *ast)
{
    if (!ast->IsIdentifier() || ast->AsIdentifier()->Variable() != nullptr) {
        return true;
    }

    const auto *id = ast->AsIdentifier();
    AddError("NULL_VARIABLE: " + ToStringHelper(id), id->Start());
    return false;
}

bool ASTVerifier::HasScope(const ir::AstNode *ast)
{
    if (!ast->IsIdentifier()) {
        return true;  // we will check only Identifier
    }
    // we will check only local variables of identifiers
    if (HasVariable(ast) && ast->AsIdentifier()->Variable()->IsLocalVariable() &&
        ast->AsIdentifier()->Variable()->AsLocalVariable()->GetScope() == nullptr) {
        const auto *id = ast->AsIdentifier();
        AddError("NULL_SCOPE_LOCAL_VAR: " + ToStringHelper(ast), id->Start());
        return false;
    }
    // NOTE(tatiana): Add check that the scope enclose this identifier
    return true;
}

}  // namespace panda::es2panda::compiler
