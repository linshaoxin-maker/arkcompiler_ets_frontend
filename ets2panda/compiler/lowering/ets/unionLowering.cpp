/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "unionLowering.h"
#include "varbinder/variableFlags.h"
#include "varbinder/ETSBinder.h"
#include "checker/ETSchecker.h"
#include "checker/ets/conversion.h"
#include "checker/ets/boxingConverter.h"
#include "checker/ets/unboxingConverter.h"
#include "compiler/lowering/util.h"
#include "compiler/lowering/scopesInit/scopesInitPhase.h"
#include "ir/base/classDefinition.h"
#include "ir/base/classProperty.h"
#include "ir/astNode.h"
#include "ir/expression.h"
#include "ir/opaqueTypeNode.h"
#include "ir/expressions/literals/nullLiteral.h"
#include "ir/expressions/literals/undefinedLiteral.h"
#include "ir/expressions/binaryExpression.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/memberExpression.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/classDeclaration.h"
#include "ir/statements/variableDeclaration.h"
#include "ir/ts/tsAsExpression.h"
#include "type_helper.h"
#include "public/public.h"

namespace ark::es2panda::compiler {
static ir::ClassDefinition *GetUnionFieldClass(checker::ETSChecker *checker, varbinder::VarBinder *varbinder)
{
    // Create the name for the synthetic class node
    util::UString unionFieldClassName(util::StringView(panda_file::GetDummyClassName()), checker->Allocator());
    varbinder::Variable *foundVar = nullptr;
    if ((foundVar = checker->Scope()->FindLocal(unionFieldClassName.View(),
                                                varbinder::ResolveBindingOptions::BINDINGS)) != nullptr) {
        return foundVar->Declaration()->Node()->AsClassDeclaration()->Definition();
    }
    auto *ident = checker->AllocNode<ir::Identifier>(unionFieldClassName.View(), checker->Allocator());
    auto [decl, var] = varbinder->NewVarDecl<varbinder::ClassDecl>(ident->Start(), ident->Name());
    ident->SetVariable(var);

    auto classCtx = varbinder::LexicalScope<varbinder::ClassScope>(varbinder);
    auto *classDef =
        checker->AllocNode<ir::ClassDefinition>(checker->Allocator(), ident, ir::ClassDefinitionModifiers::GLOBAL,
                                                ir::ModifierFlags::FINAL, Language(Language::Id::ETS));
    classDef->SetScope(classCtx.GetScope());
    auto *classDecl = checker->AllocNode<ir::ClassDeclaration>(classDef, checker->Allocator());
    classDef->Scope()->BindNode(classDecl);
    classDef->SetTsType(checker->GlobalETSObjectType());
    decl->BindNode(classDecl);
    var->SetScope(classDef->Scope());

    varbinder->AsETSBinder()->BuildClassDefinition(classDef);
    return classDef;
}

static varbinder::LocalVariable *CreateUnionFieldClassProperty(checker::ETSChecker *checker,
                                                               varbinder::VarBinder *varbinder,
                                                               checker::Type *fieldType,
                                                               const util::StringView &propName)
{
    auto *const allocator = checker->Allocator();
    auto *const dummyClass = GetUnionFieldClass(checker, varbinder);
    auto *classScope = dummyClass->Scope()->AsClassScope();

    // Enter the union filed class instance field scope
    auto fieldCtx = varbinder::LexicalScope<varbinder::LocalScope>::Enter(varbinder, classScope->InstanceFieldScope());

    if (auto *var = classScope->FindLocal(propName, varbinder::ResolveBindingOptions::VARIABLES); var != nullptr) {
        return var->AsLocalVariable();
    }

    // Create field name for synthetic class
    auto *fieldIdent = checker->AllocNode<ir::Identifier>(propName, allocator);

    // Create the synthetic class property node
    auto *field =
        checker->AllocNode<ir::ClassProperty>(fieldIdent, nullptr, nullptr, ir::ModifierFlags::NONE, allocator, false);

    // Add the declaration to the scope
    auto [decl, var] = varbinder->NewVarDecl<varbinder::LetDecl>(fieldIdent->Start(), fieldIdent->Name());
    var->AddFlag(varbinder::VariableFlags::PROPERTY);
    var->SetTsType(fieldType);
    fieldIdent->SetVariable(var);
    field->SetTsType(fieldType);
    decl->BindNode(field);

    ArenaVector<ir::AstNode *> fieldDecl {allocator->Adapter()};
    fieldDecl.push_back(field);
    dummyClass->AddProperties(std::move(fieldDecl));
    return var->AsLocalVariable();
}

static void HandleUnionPropertyAccess(checker::ETSChecker *checker, varbinder::VarBinder *vbind,
                                      ir::MemberExpression *expr)
{
    if (expr->PropVar() != nullptr) {
        return;
    }
    [[maybe_unused]] auto parent = expr->Parent();
    ASSERT(!(parent->IsCallExpression() && parent->AsCallExpression()->Callee() == expr &&
             parent->AsCallExpression()->Signature()->HasSignatureFlag(checker::SignatureFlags::TYPE)));
    expr->SetPropVar(
        CreateUnionFieldClassProperty(checker, vbind, expr->TsType(), expr->Property()->AsIdentifier()->Name()));
    ASSERT(expr->PropVar() != nullptr);
}

static std::optional<ir::Expression *> CheckNeedCast(ir::Expression *expr)
{
    if (expr->TsType() == nullptr || !expr->TsType()->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE)) {
        return {};
    }

    if (expr->IsTSAsExpression() && expr->AsTSAsExpression()->Expr()->TsType() != nullptr &&
        expr->AsTSAsExpression()->Expr()->TsType()->IsETSUnionType()) {
        return expr->AsTSAsExpression()->Expr();
    }

    // if (target->HasTypeFlag(TypeFlag::PRIMITIVE)) {
    //     if (!relation->ApplyUnboxing()) {
    //         return relation->Result(false);
    //     }
    //     relation->GetNode()->AddAstNodeFlags(ir::AstNodeFlags::UNION_CAST_PRIMITIVE);
    // }

    if (expr->HasAstNodeFlags(ir::AstNodeFlags::UNION_CAST_PRIMITIVE)) {
        ir::AstDumper d (expr);
        std::cerr << d.Str();
        return expr;
    }

    return {};
}

static void GenerateCastToPrimitive(std::stringstream &ss, checker::Type *nodeType,
                                    std::vector<ir::AstNode *> &newStmts, ir::Expression *expr)
{
    /*
     * For given union cast to primitive expression:
     *
     *  (<expr> as Numeric).[any_numeric_type]Value() or
     *  (<expr> as Char/Boolean).unboxed()
     *
     */
    auto addNode = [&newStmts](ir::AstNode *node) -> int {
        newStmts.emplace_back(node);
        return newStmts.size();
    };

    expr->SetBoxingUnboxingFlags(ir::BoxingUnboxingFlags::NONE);
    expr->SetTsType(nullptr);
    ss << "(@@E" << addNode(expr) << " as ";

    if (nodeType->HasTypeFlag(checker::TypeFlag::ETS_NUMERIC)) {
        ASSERT(nodeType->HasTypeFlag(checker::TypeFlag::CHAR));
        ASSERT(nodeType->HasTypeFlag(checker::TypeFlag::ETS_BOOLEAN));

        ss << "Numeric)." << nodeType->ToString() << "Value()";
    } else if (nodeType->HasTypeFlag(checker::TypeFlag::ETS_BOOLEAN)) {
        ss << "Boolean).unboxed()";
    } else if (nodeType->HasTypeFlag(checker::TypeFlag::CHAR)) {
        ss << "Char).unboxed()";
    } else {
        UNREACHABLE();
    }
}

static ir::Expression *HandleUnionCastToPrimitive(public_lib::Context *ctx, ir::Expression *expr)
{
    auto exprNode = CheckNeedCast(expr);
    if (!exprNode.has_value()) {
        return expr;
    }

    auto *const checker = ctx->checker->AsETSChecker();
    auto *const parser = ctx->parser->AsETSParser();
    auto *const varbinder = ctx->checker->VarBinder()->AsETSBinder();

    std::stringstream ss;
    std::vector<ir::AstNode *> newStmts;

    GenerateCastToPrimitive(ss, expr->TsType(), newStmts, exprNode.value());

    auto *loweringResult = parser->CreateFormattedExpression(ss.str(), newStmts);

    if ((expr->GetBoxingUnboxingFlags() & ir::BoxingUnboxingFlags::BOXING_FLAG) != 0U) {
        loweringResult->SetBoxingUnboxingFlags(expr->GetBoxingUnboxingFlags());
    }

    auto *parent = expr->Parent();

    loweringResult->SetParent(parent);

    auto scopeCtx = varbinder::LexicalScope<varbinder::Scope>::Enter(varbinder, NearestScope(expr));
    InitScopesPhaseETS::RunExternalNode(loweringResult, varbinder);

    varbinder->ResolveReferencesForScope(loweringResult, NearestScope(loweringResult));
    checker::SavedCheckerContext scc {checker, checker::CheckerStatus::IGNORE_VISIBILITY};
    loweringResult->Check(checker);
    return loweringResult;
}

bool UnionLowering::Perform(public_lib::Context *ctx, parser::Program *program)
{
    for (auto &[_, ext_programs] : program->ExternalSources()) {
        (void)_;
        for (auto *extProg : ext_programs) {
            Perform(ctx, extProg);
        }
    }

    checker::ETSChecker *checker = ctx->checker->AsETSChecker();

    program->Ast()->TransformChildrenRecursively(
        [checker, ctx](ir::AstNode *ast) -> ir::AstNode * {
            if (ast->IsMemberExpression() && ast->AsMemberExpression()->Object()->TsType() != nullptr) {
                auto *objType =
                    checker->GetApparentType(checker->GetNonNullishType(ast->AsMemberExpression()->Object()->TsType()));
                if (objType->IsETSUnionType()) {
                    HandleUnionPropertyAccess(checker, checker->VarBinder(), ast->AsMemberExpression());
                    return ast;
                }
            }

            if (ast->IsExpression()) {
                return HandleUnionCastToPrimitive(ctx, ast->AsExpression());
            }

            return ast;
        },
        Name());

    return true;
}

bool UnionLowering::Postcondition(public_lib::Context *ctx, const parser::Program *program)
{
    bool current = !program->Ast()->IsAnyChild([checker = ctx->checker->AsETSChecker()](ir::AstNode *ast) {
        if (!ast->IsMemberExpression() || ast->AsMemberExpression()->Object()->TsType() == nullptr) {
            return false;
        }
        auto *objType =
            checker->GetApparentType(checker->GetNonNullishType(ast->AsMemberExpression()->Object()->TsType()));
        auto *parent = ast->Parent();
        if (!(parent->IsCallExpression() &&
              parent->AsCallExpression()->Signature()->HasSignatureFlag(checker::SignatureFlags::TYPE))) {
            return false;
        }
        return objType->IsETSUnionType() && ast->AsMemberExpression()->PropVar() == nullptr;
    });
    if (!current || ctx->config->options->CompilerOptions().compilationMode != CompilationMode::GEN_STD_LIB) {
        return current;
    }

    for (auto &[_, ext_programs] : program->ExternalSources()) {
        (void)_;
        for (auto *extProg : ext_programs) {
            if (!Postcondition(ctx, extProg)) {
                return false;
            }
        }
    }
    return true;
}

}  // namespace ark::es2panda::compiler
