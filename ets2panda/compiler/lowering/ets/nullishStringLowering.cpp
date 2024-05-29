/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "nullishStringLowering.h"

#include "checker/ETSchecker.h"
#include "compiler/lowering/util.h"
#include "compiler/lowering/scopesInit/scopesInitPhase.h"
#include "parser/ETSparser.h"
#include "varbinder/ETSBinder.h"
#include "varbinder/scope.h"

namespace ark::es2panda::compiler {

std::string_view NullishStringLowering::Name() const
{
    return "NullishStringLowering";
}

// NOLINTBEGIN(modernize-avoid-c-arrays)
static constexpr char const FORMAT_CHECK_NULLISH_EXPRESSION[] =
    "let @@I1 = (@@E2) as NullishType;"
    "if (@@I3 instanceof null) \"null\";"
    "if (@@I4 instanceof undefined) \"undefined\";"
    "(@@I5);";
// NOLINTEND(modernize-avoid-c-arrays)

ir::Expression *ReplaceNullishStringCtor(public_lib::Context *const ctx,
                                         ir::ETSNewClassInstanceExpression *newClassInstExpr)
{
    auto *checker = ctx->checker->AsETSChecker();
    auto *parser = ctx->parser->AsETSParser();

    // Skip missing signatures
    if (newClassInstExpr->GetSignature() == nullptr || newClassInstExpr->GetSignature()->InternalName() == nullptr) {
        return newClassInstExpr;
    }

    // Case for the constructor: new String(str: string)
    if (newClassInstExpr->GetSignature()->InternalName() == Signatures::BUILTIN_STRING_FROM_STRING_CTOR) {
        auto *arg = newClassInstExpr->GetArguments()[0];
        arg->SetParent(newClassInstExpr->Parent());
        return arg;
    }

    // Case for the constructor: new String(str: NullishType)
    if (newClassInstExpr->GetSignature()->InternalName() == Signatures::BUILTIN_STRING_FROM_NULLISH_CTOR) {
        auto *arg = newClassInstExpr->GetArguments()[0];

        // For the case when the constructor parameter is "null" or "undefined" literals
        if (arg->IsLiteral()) {
            auto *literal = arg->IsNullLiteral() ? checker->AllocNode<ir::StringLiteral>("null")
                                                 : checker->AllocNode<ir::StringLiteral>("undefined");
            literal->SetParent(newClassInstExpr->Parent());

            // Run checker
            literal->Check(checker);
            return literal;
        }

        // Enter to the old scope
        auto *scope = NearestScope(newClassInstExpr);
        auto exprCtx = varbinder::LexicalScope<varbinder::Scope>::Enter(checker->VarBinder(), scope);

        // Generate temporary variable
        auto const tmpIdentName = GenName(checker->Allocator());

        // Create BlockExpression
        auto *blockExpr = parser->CreateFormattedExpression(FORMAT_CHECK_NULLISH_EXPRESSION, tmpIdentName, arg,
                                                            tmpIdentName, tmpIdentName, tmpIdentName);

        blockExpr->SetParent(newClassInstExpr->Parent());

        // Run VarBinder for new BlockExpression
        InitScopesPhaseETS::RunExternalNode(blockExpr, checker->VarBinder());
        checker->VarBinder()->AsETSBinder()->ResolveReferencesForScope(blockExpr, NearestScope(blockExpr));

        // Run checker
        blockExpr->Check(checker);
        return blockExpr;
    }

    return newClassInstExpr;
}

bool NullishStringLowering::Perform(public_lib::Context *const ctx, parser::Program *const program)
{
    for (const auto &[_, ext_programs] : program->ExternalSources()) {
        (void)_;
        for (auto *const extProg : ext_programs) {
            Perform(ctx, extProg);
        }
    }

    program->Ast()->TransformChildrenRecursively(
        [ctx](ir::AstNode *ast) -> ir::AstNode * {
            if (ast->IsETSNewClassInstanceExpression()) {
                return ReplaceNullishStringCtor(ctx, ast->AsETSNewClassInstanceExpression());
            }

            return ast;
        },
        Name());

    return true;
}

}  // namespace ark::es2panda::compiler
