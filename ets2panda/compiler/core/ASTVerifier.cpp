/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "checker/types/typeFlag.h"
#include "ir/astNode.h"
#include "ir/base/classDefinition.h"
#include "ir/base/classElement.h"
#include "ir/statement.h"
#include "ir/base/classStaticBlock.h"
#include "ir/base/methodDefinition.h"
#include "ir/base/scriptFunction.h"
#include "ir/ets/etsNewClassInstanceExpression.h"
#include "ir/ets/etsScript.h"
#include "ir/ets/etsImportDeclaration.h"
#include "ir/expressions/sequenceExpression.h"
#include "ir/module/importSpecifier.h"
#include "ir/module/importNamespaceSpecifier.h"
#include "ir/module/importDefaultSpecifier.h"
#include "ir/expressions/callExpression.h"
#include "ir/expressions/binaryExpression.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/memberExpression.h"
#include "ir/statements/forInStatement.h"
#include "ir/statements/forOfStatement.h"
#include "ir/statements/forUpdateStatement.h"
#include "ir/statements/variableDeclarator.h"
#include "ir/statements/expressionStatement.h"
#include "ir/statements/throwStatement.h"
#include "ir/ts/tsTypeParameter.h"
#include "lexer/token/tokenType.h"
#include "util/ustring.h"
#include "utils/arena_containers.h"
#include "varbinder/scope.h"

constexpr auto RECURSIVE_SUFFIX = "ForAll";

namespace panda::es2panda::compiler {

static bool IsNumericType(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    if (!ast->IsTyped()) {
        return false;
    }

    auto typed_ast = static_cast<const ir::TypedAstNode *>(ast);

    if (typed_ast->TsType() == nullptr) {
        return false;
    }

    return typed_ast->TsType()->HasTypeFlag(checker::TypeFlag::ETS_NUMERIC) ||
           typed_ast->TsType()->HasTypeFlag(checker::TypeFlag::NUMBER_LITERAL) ||
           typed_ast->TsType()->HasTypeFlag(checker::TypeFlag::BIGINT_LITERAL);
}

static bool IsStringType(const ir::AstNode *ast)
{
    if (ast == nullptr) {
        return false;
    }

    if (!ast->IsTyped()) {
        return false;
    }

    auto typed_ast = static_cast<const ir::TypedAstNode *>(ast);

    if (typed_ast->TsType() == nullptr) {
        return false;
    }

    return typed_ast->TsType()->HasTypeFlag(checker::TypeFlag::STRING_LIKE);
}

template <typename T>
static bool IsContainedIn(const T *child, const T *parent)
{
    if (child == nullptr || parent == nullptr) {
        return false;
    }

    std::unordered_set<const T *> saved_nodes;
    while (child != nullptr && child != parent) {
        saved_nodes.emplace(child);
        child = child->Parent();
        if (saved_nodes.find(child) != saved_nodes.end()) {
            return false;
        }
    }
    return child == parent;
}
bool IsVisibleInternalNode(const ir::AstNode *ast, const ir::AstNode *obj_type_decl_node)
{
    auto *current_top_statement = (static_cast<const ir::ETSScript *>(ast->GetTopStatement()));
    auto *current_program = current_top_statement->Program();
    if (current_program == nullptr) {
        return false;
    }
    util::StringView package_name_current = current_program->GetPackageName();
    auto *object_top_statement = (static_cast<const ir::ETSScript *>(obj_type_decl_node->GetTopStatement()));
    auto *object_program = object_top_statement->Program();
    if (object_program == nullptr) {
        return false;
    }
    util::StringView package_name_object = object_program->GetPackageName();
    return current_top_statement == object_top_statement ||
           (package_name_current == package_name_object && !package_name_current.Empty());
}

static bool ValidateVariableAccess(const varbinder::LocalVariable *prop_var, const ir::MemberExpression *ast)
{
    const auto *prop_var_decl = prop_var->Declaration();
    if (prop_var_decl == nullptr) {
        return false;
    }
    const auto *prop_var_decl_node = prop_var_decl->Node();
    if (prop_var_decl_node == nullptr) {
        return false;
    }
    auto *obj_type = ast->ObjType();
    if (obj_type == nullptr) {
        return false;
    }
    const auto *obj_type_decl_node = obj_type->GetDeclNode();
    if (obj_type_decl_node == nullptr) {
        return false;
    }
    const auto *prop_var_decl_node_parent = prop_var_decl_node->Parent();
    if (prop_var_decl_node_parent != nullptr && prop_var_decl_node_parent->IsClassDefinition() &&
        obj_type_decl_node->IsClassDefinition()) {
        // Check if the variable is used where it is declared
        if (IsContainedIn<const ir::AstNode>(ast, prop_var_decl_node_parent->AsClassDefinition())) {
            return true;
        }
        if (prop_var_decl_node->IsPrivate()) {
            return false;
        }
        if (prop_var_decl_node->IsProtected()) {
            // Check if the variable is inherited and is used in class in which it is inherited
            auto ret = obj_type->IsPropertyInherited(prop_var);
            return ret && IsContainedIn<const ir::AstNode>(ast, obj_type_decl_node->AsClassDefinition());
        }
        if (prop_var_decl_node->IsInternal()) {
            return IsVisibleInternalNode(ast, obj_type_decl_node);
        }
        return true;
    }
    return false;
}

static bool ValidateMethodAccess(const ir::MemberExpression *member_expression, const ir::CallExpression *ast)
{
    auto *member_obj_type = member_expression->ObjType();
    if (member_obj_type == nullptr) {
        return false;
    }
    if (member_obj_type->HasObjectFlag(checker::ETSObjectFlags::RESOLVED_SUPER) &&
        member_obj_type->SuperType() != nullptr &&
        member_obj_type->SuperType()->HasObjectFlag(checker::ETSObjectFlags::BUILTIN_TYPE |
                                                    checker::ETSObjectFlags::GLOBAL)) {
        return true;
    }
    const auto *member_obj_type_decl_node = member_obj_type->GetDeclNode();
    if (member_obj_type_decl_node == nullptr) {
        return false;
    }
    auto *signature = ast->Signature();
    if (signature == nullptr) {
        return false;
    }
    auto *owner_sign = signature->Owner();
    if (owner_sign == nullptr) {
        return false;
    }
    auto *owner_sign_decl_node = owner_sign->GetDeclNode();
    if (owner_sign_decl_node != nullptr && owner_sign_decl_node->IsClassDefinition() &&
        member_obj_type_decl_node->IsClassDefinition()) {
        // Check if the method is used where it is declared
        if (IsContainedIn<const ir::AstNode>(ast, owner_sign_decl_node->AsClassDefinition())) {
            return true;
        }
        if (signature->HasSignatureFlag(checker::SignatureFlags::PRIVATE)) {
            return false;
        }
        if (signature->HasSignatureFlag(checker::SignatureFlags::PROTECTED)) {
            // Check if the method is inherited and is used in class in which it is inherited
            auto ret = member_obj_type->IsSignatureInherited(signature);
            return ret && IsContainedIn<const ir::AstNode>(ast, member_obj_type_decl_node->AsClassDefinition());
        }
        if (signature->HasSignatureFlag(checker::SignatureFlags::INTERNAL)) {
            return IsVisibleInternalNode(ast, member_obj_type_decl_node);
        }
        return true;
    }
    return false;
}

class NodeHasParent {
public:
    explicit NodeHasParent([[maybe_unused]] ArenaAllocator &allocator) {}

    ASTVerifier::CheckResult operator()(ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast)
    {
        const auto is_ets_script = ast->IsETSScript();
        const auto has_parent = ast->Parent() != nullptr;
        if (!is_ets_script && !has_parent) {
            ctx.AddInvariantError("NodeHasParent", "NULL_PARENT", *ast);
            return ASTVerifier::CheckResult::FAILED;
        }
        if (ast->IsProgram()) {
            return ASTVerifier::CheckResult::SUCCESS;
        }
        return ASTVerifier::CheckResult::SUCCESS;
    }
};

class IdentifierHasVariable {
public:
    explicit IdentifierHasVariable([[maybe_unused]] ArenaAllocator &allocator) {}

    ASTVerifier::CheckResult operator()(ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast)
    {
        if (!ast->IsIdentifier()) {
            return ASTVerifier::CheckResult::SUCCESS;
        }
        if (ast->AsIdentifier()->Variable() != nullptr) {
            return ASTVerifier::CheckResult::SUCCESS;
        }

        const auto *id = ast->AsIdentifier();
        ctx.AddInvariantError("IdentifierHasVariable", "NULL_VARIABLE", *id);
        return ASTVerifier::CheckResult::FAILED;
    }

private:
};

class NodeHasType {
public:
    explicit NodeHasType([[maybe_unused]] ArenaAllocator &allocator) {}

    ASTVerifier::CheckResult operator()(ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast)
    {
        if (ast->IsTyped()) {
            if (ast->IsClassDefinition() && ast->AsClassDefinition()->Ident()->Name() == "ETSGLOBAL") {
                return ASTVerifier::CheckResult::SKIP_SUBTREE;
            }
            const auto *typed = static_cast<const ir::TypedAstNode *>(ast);
            if (typed->TsType() == nullptr) {
                ctx.AddInvariantError("NodeHasType", "NULL_TS_TYPE", *ast);
                return ASTVerifier::CheckResult::FAILED;
            }
        }
        return ASTVerifier::CheckResult::SUCCESS;
    }

private:
};

class VariableHasScope {
public:
    explicit VariableHasScope(ArenaAllocator &allocator) : allocator_ {allocator} {}

    ASTVerifier::CheckResult operator()(ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast)
    {
        if (!ast->IsIdentifier()) {
            return ASTVerifier::CheckResult::SUCCESS;  // we will check invariant of Identifier only
        }

        // we will check invariant for only local variables of identifiers
        if (const auto maybe_var = GetLocalScopeVariable(allocator_, ctx, ast); maybe_var.has_value()) {
            const auto var = *maybe_var;
            const auto scope = var->GetScope();
            if (scope == nullptr) {
                ctx.AddInvariantError("VariableHasScope", "NULL_SCOPE_LOCAL_VAR", *ast);
                return ASTVerifier::CheckResult::FAILED;
            }
            return ScopeEncloseVariable(ctx, var) ? ASTVerifier::CheckResult::SUCCESS
                                                  : ASTVerifier::CheckResult::FAILED;
        }
        return ASTVerifier::CheckResult::SUCCESS;
    }

    static std::optional<varbinder::LocalVariable *> GetLocalScopeVariable(ArenaAllocator &allocator,
                                                                           ASTVerifier::ErrorContext &ctx,
                                                                           const ir::AstNode *ast)
    {
        if (!ast->IsIdentifier()) {
            return std::nullopt;
        }

        auto invariant_has_variable = IdentifierHasVariable {allocator};
        const auto variable = ast->AsIdentifier()->Variable();
        if ((invariant_has_variable(ctx, ast) == ASTVerifier::CheckResult::SUCCESS) && variable->IsLocalVariable()) {
            const auto local_var = variable->AsLocalVariable();
            if (local_var->HasFlag(varbinder::VariableFlags::LOCAL)) {
                return local_var;
            }
        }
        return std::nullopt;
    }

    bool ScopeEncloseVariable(ASTVerifier::ErrorContext &ctx, const varbinder::LocalVariable *var)
    {
        ASSERT(var);

        const auto scope = var->GetScope();
        if (scope == nullptr || var->Declaration() == nullptr) {
            return true;
        }
        const auto node = var->Declaration()->Node();
        if (node == nullptr) {
            return true;
        }
        const auto name = "VariableHasScope";
        bool is_ok = true;
        if (scope->Bindings().count(var->Name()) == 0) {
            ctx.AddInvariantError(name, "SCOPE_DO_NOT_ENCLOSE_LOCAL_VAR", *node);
            is_ok = false;
        }
        const auto scope_node = scope->Node();
        auto var_node = node;
        if (!IsContainedIn(var_node, scope_node) || scope_node == nullptr) {
            ctx.AddInvariantError(name, "SCOPE_NODE_DONT_DOMINATE_VAR_NODE", *node);
            is_ok = false;
        }
        const auto &decls = scope->Decls();
        const auto decl_dominate = std::count(decls.begin(), decls.end(), var->Declaration());
        if (decl_dominate == 0) {
            ctx.AddInvariantError(name, "SCOPE_DECL_DONT_DOMINATE_VAR_DECL", *node);
            is_ok = false;
        }
        return is_ok;
    }

private:
    ArenaAllocator &allocator_;
};

class EveryChildHasValidParent {
public:
    explicit EveryChildHasValidParent([[maybe_unused]] ArenaAllocator &allocator) {}

    ASTVerifier::CheckResult operator()(ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast)
    {
        auto result = ASTVerifier::CheckResult::SUCCESS;
        if (ast->IsETSScript()) {
            return result;
        }
        ast->Iterate([&](const ir::AstNode *node) {
            if (ast != node->Parent()) {
                ctx.AddInvariantError("EveryChildHasValidParent", "INCORRECT_PARENT_REF", *node);
                result = ASTVerifier::CheckResult::FAILED;
            }
        });
        return result;
    }

private:
};

class VariableHasEnclosingScope {
public:
    explicit VariableHasEnclosingScope(ArenaAllocator &allocator) : allocator_ {allocator} {}

    ASTVerifier::CheckResult operator()(ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast)
    {
        const auto maybe_var = VariableHasScope::GetLocalScopeVariable(allocator_, ctx, ast);
        if (!maybe_var) {
            return ASTVerifier::CheckResult::SUCCESS;
        }
        const auto var = *maybe_var;
        const auto scope = var->GetScope();
        const auto name = "VariableHasEnclosingScope";
        if (scope == nullptr) {
            // already checked
            return ASTVerifier::CheckResult::SUCCESS;
        }
        const auto enclose_scope = scope->EnclosingVariableScope();
        if (enclose_scope == nullptr) {
            ctx.AddInvariantError(name, "NO_ENCLOSING_VAR_SCOPE", *ast);
            return ASTVerifier::CheckResult::FAILED;
        }
        const auto node = scope->Node();
        auto result = ASTVerifier::CheckResult::SUCCESS;
        if (!IsContainedIn(ast, node)) {
            result = ASTVerifier::CheckResult::FAILED;
            ctx.AddInvariantError(name, "VARIABLE_NOT_ENCLOSE_SCOPE", *ast);
        }
        if (!IsContainedIn<varbinder::Scope>(scope, enclose_scope)) {
            result = ASTVerifier::CheckResult::FAILED;
            ctx.AddInvariantError(name, "VARIABLE_NOT_ENCLOSE_SCOPE", *ast);
        }
        return result;
    }

private:
    ArenaAllocator &allocator_;
};

class SequenceExpressionHasLastType {
public:
    explicit SequenceExpressionHasLastType([[maybe_unused]] ArenaAllocator &allocator) {}

    ASTVerifier::CheckResult operator()(ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast)
    {
        if (!ast->IsSequenceExpression()) {
            return ASTVerifier::CheckResult::SUCCESS;
        }
        const auto *expr = ast->AsSequenceExpression();
        const auto *last = expr->Sequence().back();
        const auto name = "SequenceExpressionHasLastType";
        if (expr->TsType() == nullptr) {
            ctx.AddInvariantError(name, "Sequence expression type is null", *expr);
            return ASTVerifier::CheckResult::FAILED;
        }
        if (last->TsType() == nullptr) {
            ctx.AddInvariantError(name, "Sequence expression last type is null", *last);
            return ASTVerifier::CheckResult::FAILED;
        }
        if (expr->TsType() != last->TsType()) {
            ctx.AddInvariantError(name, "Sequence expression type and last expression type are not the same", *expr);
            return ASTVerifier::CheckResult::FAILED;
        }
        return ASTVerifier::CheckResult::SUCCESS;
    }

private:
};

class ForLoopCorrectlyInitialized {
public:
    explicit ForLoopCorrectlyInitialized([[maybe_unused]] ArenaAllocator &allocator) {}

    ASTVerifier::CheckResult operator()(ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast)
    {
        const auto name = "ForLoopCorrectlyInitialized";
        if (ast->IsForInStatement()) {
            auto const *left = ast->AsForInStatement()->Left();
            if (left == nullptr) {
                ctx.AddInvariantError(name, "NULL FOR-IN-LEFT", *ast);
                return ASTVerifier::CheckResult::FAILED;
            }

            if (!left->IsIdentifier() && !left->IsVariableDeclaration()) {
                ctx.AddInvariantError(name, "INCORRECT FOR-IN-LEFT", *ast);
                return ASTVerifier::CheckResult::FAILED;
            }
        }

        if (ast->IsForOfStatement()) {
            auto const *left = ast->AsForOfStatement()->Left();
            if (left == nullptr) {
                ctx.AddInvariantError(name, "NULL FOR-OF-LEFT", *ast);
                return ASTVerifier::CheckResult::FAILED;
            }

            if (!left->IsIdentifier() && !left->IsVariableDeclaration()) {
                ctx.AddInvariantError(name, "INCORRECT FOR-OF-LEFT", *ast);
                return ASTVerifier::CheckResult::FAILED;
            }
        }

        if (ast->IsForUpdateStatement()) {
            // The most important part of for-loop is the test.
            // But it also can be null. Then there must be break;(return) in the body.
            auto const *test = ast->AsForUpdateStatement()->Test();
            if (test == nullptr) {
                auto const *body = ast->AsForUpdateStatement()->Body();
                if (body == nullptr) {
                    ctx.AddInvariantError(name, "NULL FOR-TEST AND FOR-BODY", *ast);
                    return ASTVerifier::CheckResult::FAILED;
                }
                bool has_exit = body->IsBreakStatement() || body->IsReturnStatement();
                body->IterateRecursively([&has_exit](ir::AstNode *child) {
                    has_exit |= child->IsBreakStatement() || child->IsReturnStatement();
                });
                if (!has_exit) {
                    // an infinite loop
                    ctx.AddInvariantError(name, "WARNING: NULL FOR-TEST AND FOR-BODY doesn't exit", *ast);
                }
                return ASTVerifier::CheckResult::SUCCESS;
            }

            if (!test->IsExpression()) {
                ctx.AddInvariantError(name, "NULL FOR VAR", *ast);
                return ASTVerifier::CheckResult::FAILED;
            }
        }
        return ASTVerifier::CheckResult::SUCCESS;
    }

private:
};

class ModifierAccessValid {
public:
    explicit ModifierAccessValid([[maybe_unused]] ArenaAllocator &allocator) {}

    ASTVerifier::CheckResult operator()(ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast)
    {
        const auto name = "ModifierAccessValid";
        if (ast->IsMemberExpression()) {
            const auto *prop_var = ast->AsMemberExpression()->PropVar();
            if (prop_var != nullptr && prop_var->HasFlag(varbinder::VariableFlags::PROPERTY) &&
                !ValidateVariableAccess(prop_var, ast->AsMemberExpression())) {
                ctx.AddInvariantError(name, "PROPERTY_NOT_VISIBLE_HERE", *ast);
                return ASTVerifier::CheckResult::FAILED;
            }
        }
        if (ast->IsCallExpression()) {
            const auto *call_expr = ast->AsCallExpression();
            const auto *callee = call_expr->Callee();
            if (callee != nullptr && callee->IsMemberExpression()) {
                const auto *callee_member = callee->AsMemberExpression();
                const auto *prop_var_callee = callee_member->PropVar();
                if (prop_var_callee != nullptr && prop_var_callee->HasFlag(varbinder::VariableFlags::METHOD) &&
                    !ValidateMethodAccess(callee_member, ast->AsCallExpression())) {
                    ctx.AddInvariantError(name, "PROPERTY_NOT_VISIBLE_HERE", *callee);
                    return ASTVerifier::CheckResult::FAILED;
                }
            }
        }
        return ASTVerifier::CheckResult::SUCCESS;
    }

private:
};

class ImportExportAccessValid {
public:
    explicit ImportExportAccessValid([[maybe_unused]] ArenaAllocator &allocator) {}

    ASTVerifier::CheckResult operator()(ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast)
    {
        ASTVerifier::InvariantSet imported_variables {};
        if (ast->IsETSImportDeclaration()) {
            const auto import_decl = ast->AsETSImportDeclaration()->Specifiers();
            const auto name = [](ir::AstNode *const specifier) {
                if (specifier->IsImportNamespaceSpecifier()) {
                    return specifier->AsImportNamespaceSpecifier()->Local()->Name();
                }
                if (specifier->IsImportSpecifier()) {
                    return specifier->AsImportSpecifier()->Local()->Name();
                }
                return specifier->AsImportDefaultSpecifier()->Local()->Name();
            };
            for (const auto import : import_decl) {
                imported_variables.emplace(name(import));
            }
        }
        const auto name = "ImportExportAccessValid";
        if (ast->IsCallExpression()) {
            const auto *call_expr = ast->AsCallExpression();
            const auto *callee = call_expr->Callee();
            if (callee != nullptr && callee->IsIdentifier() &&
                !HandleImportExportIdentifier(imported_variables, callee->AsIdentifier(), call_expr)) {
                ctx.AddInvariantError(name, "PROPERTY_NOT_VISIBLE_HERE(NOT_EXPORTED)", *callee);
                return ASTVerifier::CheckResult::FAILED;
            }
        }
        if (ast->IsIdentifier() && !HandleImportExportIdentifier(imported_variables, ast->AsIdentifier(), nullptr)) {
            ctx.AddInvariantError(name, "PROPERTY_NOT_VISIBLE_HERE(NOT_EXPORTED)", *ast);
            return ASTVerifier::CheckResult::FAILED;
        }
        return ASTVerifier::CheckResult::SUCCESS;
    }

private:
    bool ValidateExport(const varbinder::Variable *var)
    {
        const auto *decl = var->Declaration();
        if (decl == nullptr) {
            return false;
        }
        const auto *node = decl->Node();
        if (node == nullptr) {
            return false;
        }
        return node->IsExported();
    }

    bool InvariantImportExportMethod(const ASTVerifier::InvariantSet &imported_variables,
                                     const varbinder::Variable *var_callee, const ir::AstNode *call_expr,
                                     util::StringView name)
    {
        auto *signature = call_expr->AsCallExpression()->Signature();
        if (signature->Owner() == nullptr) {
            // NOTE(vpukhov): Add a synthetic owner for dynamic signatures
            ASSERT(call_expr->AsCallExpression()->Callee()->TsType()->HasTypeFlag(checker::TypeFlag::ETS_DYNAMIC_FLAG));
            return true;
        }

        if (signature != nullptr && var_callee->Declaration() != nullptr &&
            var_callee->Declaration()->Node() != nullptr &&
            !IsContainedIn(var_callee->Declaration()->Node(), signature->Owner()->GetDeclNode()) &&
            var_callee->Declaration()->Node() != signature->Owner()->GetDeclNode()) {
            if (imported_variables.find(name.Mutf8()) != imported_variables.end() ||
                imported_variables.find("") != imported_variables.end()) {
                return ValidateExport(var_callee);
            }
            return false;
        }
        return true;
    }

    bool InvariantImportExportVariable(const ASTVerifier::InvariantSet &imported_variables,
                                       const varbinder::Variable *var, const ir::Identifier *ident,
                                       util::StringView name)
    {
        if (!var->HasFlag(varbinder::VariableFlags::LOCAL) && !var->HasFlag(varbinder::VariableFlags::VAR) &&
            var->HasFlag(varbinder::VariableFlags::INITIALIZED) && var->Declaration() != nullptr &&
            var->Declaration()->Node() != nullptr && !var->Declaration()->Node()->IsMethodDefinition() &&
            !var->Declaration()->Node()->IsClassProperty()) {
            auto var_parent = var->Declaration()->Node()->Parent();
            if (var_parent != nullptr && !IsContainedIn(ident->Parent(), var_parent) && ident->Parent() != var_parent) {
                if (var->GetScope() != nullptr && var->GetScope()->Parent() != nullptr &&
                    var->GetScope()->Parent()->IsGlobalScope() &&
                    ident->GetTopStatement() == var_parent->GetTopStatement()) {
                    return true;
                }
                if (imported_variables.find(name.Mutf8()) != imported_variables.end() ||
                    imported_variables.find("") != imported_variables.end()) {
                    return ValidateExport(var);
                }
                return false;
            }
        }
        return true;
    }

    bool HandleImportExportIdentifier(ASTVerifier::InvariantSet &imported_variables, const ir::Identifier *ident,
                                      const ir::AstNode *call_expr)
    {
        if (ident->IsReference()) {
            const auto *var = ident->Variable();
            if (var != nullptr) {
                if (var->HasFlag(varbinder::VariableFlags::METHOD) && call_expr != nullptr) {
                    return InvariantImportExportMethod(imported_variables, var, call_expr, ident->Name());
                }
                return InvariantImportExportVariable(imported_variables, var, ident, ident->Name());
            }
        }
        return true;
    }
};

class ArithmeticOperationValid {
public:
    explicit ArithmeticOperationValid([[maybe_unused]] ArenaAllocator &allocator) {}

    ASTVerifier::CheckResult operator()([[maybe_unused]] ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast)
    {
        if (ast->IsBinaryExpression() && ast->AsBinaryExpression()->IsArithmetic()) {
            if (ast->AsBinaryExpression()->OperatorType() == lexer::TokenType::PUNCTUATOR_PLUS &&
                IsStringType(ast->AsBinaryExpression()->Left()) && IsStringType(ast->AsBinaryExpression()->Right())) {
                return ASTVerifier::CheckResult::SUCCESS;
            }
            auto result = ASTVerifier::CheckResult::SUCCESS;
            ast->Iterate([&result](ir::AstNode *child) {
                if (!IsNumericType(child)) {
                    result = ASTVerifier::CheckResult::FAILED;
                }
            });
            return result;
        }

        return ASTVerifier::CheckResult::SUCCESS;
    }

private:
};

template <typename Func>
static ASTVerifier::InvariantCheck RecursiveInvariant(const Func &func)
{
    return [func](ASTVerifier::ErrorContext &ctx, const ir::AstNode *ast) -> ASTVerifier::CheckResult {
        std::function<void(const ir::AstNode *)> aux;
        auto result = ASTVerifier::CheckResult::SUCCESS;
        aux = [&ctx, &func, &aux, &result](const ir::AstNode *child) -> void {
            if (result == ASTVerifier::CheckResult::FAILED) {
                return;
            }
            const auto new_result = func(ctx, child);
            if (new_result == ASTVerifier::CheckResult::SKIP_SUBTREE) {
                return;
            }
            result = new_result;
            child->Iterate(aux);
        };
        aux(ast);
        return result;
    };
}

void ASTVerifier::AddInvariant(const std::string &name, const InvariantCheck &invariant)
{
    invariants_checks_[name] = invariant;
    invariants_names_.insert(name);
    invariants_checks_[name + RECURSIVE_SUFFIX] = RecursiveInvariant(invariant);
    invariants_names_.insert(name + RECURSIVE_SUFFIX);
}

ASTVerifier::ASTVerifier(ArenaAllocator *allocator)
{
    AddInvariant("NodeHasParent", *allocator->New<NodeHasParent>(*allocator));
    AddInvariant("NodeHasType", *allocator->New<NodeHasType>(*allocator));
    AddInvariant("IdentifierHasVariable", *allocator->New<IdentifierHasVariable>(*allocator));
    AddInvariant("VariableHasScope", *allocator->New<VariableHasScope>(*allocator));
    AddInvariant("EveryChildHasValidParent", *allocator->New<EveryChildHasValidParent>(*allocator));
    AddInvariant("VariableHasEnclosingScope", *allocator->New<VariableHasEnclosingScope>(*allocator));
    AddInvariant("ForLoopCorrectlyInitialized", *allocator->New<ForLoopCorrectlyInitialized>(*allocator));
    AddInvariant("ModifierAccessValid", *allocator->New<ModifierAccessValid>(*allocator));
    AddInvariant("ImportExportAccessValid", *allocator->New<ImportExportAccessValid>(*allocator));
    AddInvariant("ArithmeticOperationValid", *allocator->New<ArithmeticOperationValid>(*allocator));
    AddInvariant("SequenceExpressionHasLastType", *allocator->New<SequenceExpressionHasLastType>(*allocator));
}

std::tuple<ASTVerifier::Errors, ASTVerifier::Errors> ASTVerifier::VerifyFull(
    const std::unordered_set<std::string> &warnings, const std::unordered_set<std::string> &asserts,
    const ir::AstNode *ast)
{
    auto recursive_checks = InvariantSet {};
    std::copy_if(invariants_names_.begin(), invariants_names_.end(),
                 std::inserter(recursive_checks, recursive_checks.end()),
                 [](const std::string &s) { return s.find(RECURSIVE_SUFFIX) != s.npos; });
    return Verify(warnings, asserts, ast, recursive_checks);
}

std::tuple<ASTVerifier::Errors, ASTVerifier::Errors> ASTVerifier::Verify(
    const std::unordered_set<std::string> &warnings, const std::unordered_set<std::string> &asserts,
    const ir::AstNode *ast, const InvariantSet &invariant_set)
{
    ErrorContext warning_ctx {};
    AssertsContext assert_ctx {};

    const auto contains_invariants =
        std::includes(invariants_names_.begin(), invariants_names_.end(), invariant_set.begin(), invariant_set.end());

    if (!contains_invariants) {
        auto invalid_invariants = InvariantSet {};
        for (const auto &invariant : invariant_set) {
            if (invariants_names_.find(invariant) == invariants_names_.end()) {
                invalid_invariants.insert(invariant.data());
            }
        }
        for (const auto &invariant : invalid_invariants) {
            assert_ctx.AddError(std::string {"invariant was not found: "} + invariant);
        }
    }

    for (const auto &invariant_name : invariant_set) {
        if (warnings.count(invariant_name) > 0) {
            invariants_checks_[invariant_name](warning_ctx, ast);
        } else if (asserts.count(invariant_name) > 0) {
            invariants_checks_[invariant_name](assert_ctx, ast);
        }
    }

    return std::make_tuple(warning_ctx.GetErrors(), assert_ctx.GetErrors());
}

}  // namespace panda::es2panda::compiler
