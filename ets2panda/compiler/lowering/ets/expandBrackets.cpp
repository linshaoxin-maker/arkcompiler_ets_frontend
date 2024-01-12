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

#include "expandBrackets.h"

#include "checker/ETSchecker.h"
#include "compiler/lowering/util.h"
#include "compiler/lowering/scopesInit/scopesInitPhase.h"
#include "ir/statements/blockStatement.h"
#include "ir/expressions/memberExpression.h"
#include "parser/ETSparser.h"
#include "varbinder/ETSBinder.h"

namespace panda::es2panda::compiler {

bool ExpandBracketsPhase::Perform(public_lib::Context *ctx, parser::Program *program)
{
    auto *const checker = ctx->checker->AsETSChecker();
    auto *const allocator = checker->Allocator();
    auto *const parser = ctx->parser->AsETSParser();

    program->Ast()->TransformChildrenRecursively([ctx, parser, checker, allocator](ir::AstNode *ast) -> ir::AstNode * {
        if (!ast->IsETSNewArrayInstanceExpression()) {
            return ast;
        }
        auto *new_expression = ast->AsETSNewArrayInstanceExpression();
        auto *dimension = new_expression->Dimension();
        auto *dim_type = dimension->TsType();
        if (auto *unboxed = checker->ETSBuiltinTypeAsPrimitiveType(dim_type); unboxed != nullptr) {
            dim_type = unboxed;
        }
        if (!dim_type->HasTypeFlag(checker::TypeFlag::ETS_FLOATING_POINT)) {
            return ast;
        }

        auto *casted_dimension =
            parser->CreateFormattedExpression("@@E1 as int", parser::DEFAULT_SOURCE_FILE, dimension);
        casted_dimension->Check(checker);
        casted_dimension->SetParent(dimension->Parent());
        new_expression->SetDimension(casted_dimension);

        auto *const scope = NearestScope(new_expression);
        auto expression_ctx = varbinder::LexicalScope<varbinder::Scope>::Enter(checker->VarBinder(), scope);
        auto *ident = Gensym(allocator);
        auto *expr_type = checker->AllocNode<ir::OpaqueTypeNode>(dim_type);
        auto *sequence_expr = parser->CreateFormattedExpression(
            "let @@I1 = (@@E2) as @@T3;"
            "if (!isSafeInteger(@@I4)) {"
            "  throw new TypeError(\"Index fractional part should not be different from 0.0\");"
            "};"
            "(@@E5);",
            parser::DEFAULT_SOURCE_FILE, ident, dimension, expr_type, ident->Clone(allocator), new_expression);
        sequence_expr->SetParent(new_expression->Parent());
        ScopesInitPhaseETS::RunExternalNode(sequence_expr, ctx->compiler_context->VarBinder());
        checker->VarBinder()->AsETSBinder()->ResolveReferencesForScope(sequence_expr, scope);
        sequence_expr->Check(checker);

        return sequence_expr;
    });
    program->Ast()->TransformChildrenRecursively([ctx, parser, checker, allocator](ir::AstNode *ast) -> ir::AstNode * {
        if (!ast->IsAssignmentExpression()) {
            return ast;
        }
        auto *assignment = ast->AsAssignmentExpression();
        if (!assignment->Left()->IsMemberExpression()) {
            return ast;
        }
        auto *member_expression = assignment->Left()->AsMemberExpression();
        if (!member_expression->IsComputed()) {
            return ast;
        }
        auto *index = member_expression->Property();
        auto *index_type = index->TsType();
        if (auto *unboxed = checker->ETSBuiltinTypeAsPrimitiveType(index_type); unboxed != nullptr) {
            index_type = unboxed;
        }
        if (!index_type->HasTypeFlag(checker::TypeFlag::ETS_FLOATING_POINT)) {
            return ast;
        }
        auto *const scope = NearestScope(assignment);
        auto expression_ctx = varbinder::LexicalScope<varbinder::Scope>::Enter(checker->VarBinder(), scope);
        auto *ident = Gensym(allocator);
        auto *object = member_expression->Object();
        auto *expr_type = checker->AllocNode<ir::OpaqueTypeNode>(index_type);
        auto *sequence_expr = parser->CreateFormattedExpression(
            "let @@I1 = (@@E2) as @@T3;"
            "if (!isSafeInteger(@@I4)) {"
            "  throw new TypeError(\"Index fractional part should not be different from 0.0\");"
            "};"
            "@@E5[@@I6 as int] = (@@E7);",
            parser::DEFAULT_SOURCE_FILE, ident, index, expr_type, ident->Clone(allocator), object,
            ident->Clone(allocator), assignment->Right());
        sequence_expr->SetParent(assignment->Parent());
        ScopesInitPhaseETS::RunExternalNode(sequence_expr, ctx->compiler_context->VarBinder());
        checker->VarBinder()->AsETSBinder()->ResolveReferencesForScope(sequence_expr, scope);
        sequence_expr->Check(checker);
        return sequence_expr;
    });
    program->Ast()->TransformChildrenRecursively([ctx, parser, checker, allocator](ir::AstNode *ast) -> ir::AstNode * {
        if (!ast->IsMemberExpression()) {
            return ast;
        }
        auto *member_expression = ast->AsMemberExpression();
        if (!member_expression->IsComputed()) {
            return ast;
        }
        auto *index = member_expression->Property();
        auto *index_type = index->TsType();
        if (auto *unboxed = checker->ETSBuiltinTypeAsPrimitiveType(index_type); unboxed != nullptr) {
            index_type = unboxed;
        }
        if (!index_type->HasTypeFlag(checker::TypeFlag::ETS_FLOATING_POINT)) {
            return ast;
        }

        auto *const scope = NearestScope(member_expression);
        auto expression_ctx = varbinder::LexicalScope<varbinder::Scope>::Enter(checker->VarBinder(), scope);
        auto *ident = Gensym(allocator);
        auto *object = member_expression->Object();
        auto *expr_type = checker->AllocNode<ir::OpaqueTypeNode>(index_type);
        auto *sequence_expr = parser->CreateFormattedExpression(
            "let @@I1 = (@@E2) as @@T3;"
            "if (!isSafeInteger(@@I4)) {"
            "  throw new TypeError(\"Index fractional part should not be different from 0.0\");"
            "};"
            "@@E5[@@I6 as int];",
            parser::DEFAULT_SOURCE_FILE, ident, index, expr_type, ident->Clone(allocator), object,
            ident->Clone(allocator));
        sequence_expr->SetParent(member_expression->Parent());
        ScopesInitPhaseETS::RunExternalNode(sequence_expr, ctx->compiler_context->VarBinder());
        checker->VarBinder()->AsETSBinder()->ResolveReferencesForScope(sequence_expr, scope);
        sequence_expr->Check(checker);
        return sequence_expr;
    });
    return true;
}

}  // namespace panda::es2panda::compiler
