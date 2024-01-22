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
#include "compiler/core/ASTVerifier.h"
#include "varbinder/variableFlags.h"
#include "varbinder/ETSBinder.h"
#include "checker/ETSchecker.h"
#include "checker/ets/conversion.h"
#include "checker/ets/boxingConverter.h"
#include "checker/ets/unboxingConverter.h"
#include "compiler/core/compilerContext.h"
#include "compiler/lowering/util.h"
#include "compiler/lowering/scopesInit/scopesInitPhase.h"
#include "ir/base/classDefinition.h"
#include "ir/base/classProperty.h"
#include "ir/astNode.h"
#include "ir/expression.h"
#include "ir/opaqueTypeNode.h"
#include "ir/expressions/literals/booleanLiteral.h"
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
                                                ir::ModifierFlags::NONE, Language(Language::Id::ETS));
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

static ir::TSAsExpression *GenAsExpression(checker::ETSChecker *checker, checker::Type *const opaqueType,
                                           ir::Expression *const node, ir::AstNode *const parent)
{
    auto *const typeNode = checker->AllocNode<ir::OpaqueTypeNode>(opaqueType);
    auto *const asExpression = checker->AllocNode<ir::TSAsExpression>(node, typeNode, false);
    asExpression->SetParent(parent);
    asExpression->Check(checker);
    return asExpression;
}

/*
 *  Function that generates conversion from (union) to (primitive) type as to `as` expressions:
 *      (union) as (prim) => ((union) as (ref)) as (prim),
 *      where (ref) is some unboxable type from union constituent types.
 *  Finally, `(union) as (prim)` expression replaces union_node that came above.
 */
static ir::TSAsExpression *UnionCastToPrimitive(checker::ETSChecker *checker, checker::ETSObjectType *unboxableRef,
                                                checker::Type *unboxedPrim, ir::Expression *unionNode)
{
    auto *const unionAsRefExpression = GenAsExpression(checker, unboxableRef, unionNode, nullptr);
    auto *const refAsPrimExpressoin = GenAsExpression(checker, unboxedPrim, unionAsRefExpression, unionNode->Parent());
    unionAsRefExpression->SetParent(refAsPrimExpressoin);
    return refAsPrimExpressoin;
}

static ir::TSAsExpression *HandleUnionCastToPrimitive(checker::ETSChecker *checker, ir::TSAsExpression *expr)
{
    auto *const unionType = expr->Expr()->TsType()->AsETSUnionType();
    if (unionType->IsNumericUnion()) {
        return expr;
    }
    auto *sourceType = unionType->FindExactOrBoxedType(checker, expr->TsType());
    if (sourceType == nullptr) {
        sourceType =
            checker->GetNonConstantTypeFromPrimitiveType(unionType->AsETSUnionType()->FindTypeIsCastableToSomeType(
                expr->Expr(), checker->Relation(), expr->TsType()));
    }
    expr->Expr()->SetBoxingUnboxingFlags(ir::BoxingUnboxingFlags::NONE);
    bool canCastToUnboxable =
        sourceType != nullptr && (sourceType->IsETSUnboxableObject() || sourceType->IsETSPrimitiveType());
    if (canCastToUnboxable) {
        if (expr->TsType()->IsETSPrimitiveType()) {
            auto *const asPrimSource =
                UnionCastToPrimitive(checker, checker->MaybePromotedBuiltinType(sourceType)->AsETSObjectType(),
                                     checker->MaybePrimitiveBuiltinType(sourceType), expr->Expr());
            expr->SetExpr(asPrimSource);
        }
        return expr;
    }
    auto *const unboxableUnionType = unionType->FindUnboxableType();
    auto *const unboxableType =
        checker->MaybePromotedBuiltinType(unboxableUnionType != nullptr ? unboxableUnionType : expr->TsType());
    auto *const unboxedType = checker->MaybePrimitiveBuiltinType(unboxableType);
    expr->SetExpr(UnionCastToPrimitive(checker, unboxableType->AsETSObjectType(), unboxedType, expr->Expr()));
    return expr;
}

ir::BinaryExpression *HandleBinaryExpressionWithUnion(public_lib::Context *ctx, ir::BinaryExpression *binExpr)
{
    checker::ETSChecker *checker = ctx->checker->AsETSChecker();
    if (binExpr->Left()->TsType()->IsETSUnionType() && binExpr->Left()->TsType()->AsETSUnionType()->IsNumericUnion()) {
        auto *lub = binExpr->Left()->TsType()->AsETSUnionType()->GetAssemblerLUB();
        ASSERT(lub->IsETSPrimitiveType());
        binExpr->SetLeft(GenAsExpression(checker, lub, binExpr->Left(), binExpr->Left()->Parent()));
        binExpr->SetOperationType(lub);
        binExpr->SetTsType(binExpr->TsType()->IsETSUnionType() ? lub : binExpr->TsType());
    }
    if (binExpr->Right()->TsType()->IsETSUnionType() &&
        binExpr->Right()->TsType()->AsETSUnionType()->IsNumericUnion()) {
        auto *lub = binExpr->Right()->TsType()->AsETSUnionType()->GetAssemblerLUB();
        ASSERT(lub->IsETSPrimitiveType());
        binExpr->SetRight(GenAsExpression(checker, lub, binExpr->Right(), binExpr->Right()->Parent()));
        binExpr->SetOperationType(lub);
        binExpr->SetTsType(binExpr->TsType()->IsETSUnionType() ? lub : binExpr->TsType());
    }
    if (!binExpr->OperationType()->IsETSUnionType()) {
        return binExpr;
    }
    if (binExpr->OperatorType() != lexer::TokenType::PUNCTUATOR_EQUAL &&
        binExpr->OperatorType() != lexer::TokenType::PUNCTUATOR_NOT_EQUAL) {
        ctx->checker->ThrowTypeError(
            "Bad operand type, non-primitive unions are not allowed in binary expressions except equality.",
            binExpr->Start());
    }
    return binExpr;
}

// As soon as we have primitive unions (unions with only primitive types/constants) and
// literal types (aka reduced unions) for compatibility we need to process the instanceof properly
ir::Expression *HandleInstanceofWithPrimitive(public_lib::Context *ctx, ir::BinaryExpression *binExpr)
{
    ASSERT(binExpr->OperatorType() == lexer::TokenType::KEYW_INSTANCEOF);
    auto *const checker = ctx->checker->AsETSChecker();
    auto *const relation = checker->Relation();
    auto *const parser = ctx->parser->AsETSParser();
    auto *const lhsType = binExpr->Left()->TsType();
    auto *const rhsType = binExpr->Right()->TsType();
    auto *const maybePrimRhsType = checker->MaybePrimitiveBuiltinType(rhsType);
    if (lhsType->IsConstantType()) {
        bool isIdentical =
            relation->IsIdenticalTo(checker->GetNonConstantTypeFromPrimitiveType(lhsType), maybePrimRhsType);
        auto *const newExpr = checker->AllocNode<ir::BooleanLiteral>(isIdentical);
        newExpr->SetParent(binExpr->Parent());
        return newExpr;
    }
    if (lhsType->IsETSPrimitiveType()) {
        checker->ThrowTypeError(
            "Bad operand type, the left type of instanceof expression must be reduced union or reference type.",
            binExpr->Left()->Start());
    }
    ASSERT(lhsType->IsETSUnionType() && !lhsType->AsETSUnionType()->IsReferenceUnion());
    auto *const unionType = lhsType->AsETSUnionType();
    ir::Expression *newBinExpr = checker->AllocNode<ir::BooleanLiteral>(false);
    for (auto *ct : unionType->ConstituentTypes()) {
        ASSERT(ct->IsETSPrimitiveType());
        auto *const nonConstCt = checker->GetNonConstantTypeFromPrimitiveType(ct);
        if (!relation->IsIdenticalTo(nonConstCt, maybePrimRhsType)) {
            continue;
        }
        auto *const unionEqullity = parser->CreateFormattedExpression(
            "@@E1 == " + ct->ToString(), binExpr->Left()->Clone(checker->Allocator(), newBinExpr));
        unionEqullity->Check(checker);
        newBinExpr = parser->CreateFormattedExpression(
            "@@E1 || @@E2", newBinExpr, HandleBinaryExpressionWithUnion(ctx, unionEqullity->AsBinaryExpression()));
    }
    newBinExpr->SetParent(binExpr->Parent());
    InitScopesPhaseETS::RunExternalNode(newBinExpr, ctx->compilerContext->VarBinder());
    newBinExpr->Check(checker);
    return newBinExpr;
}

static bool CastToPrimitiveLoweringAppliable(const ir::AstNode *astNode)
{
    if (!astNode->IsTSAsExpression()) {
        return false;
    }
    auto *const asExpr = astNode->AsTSAsExpression();
    auto *const exprType = asExpr->Expr()->TsType();
    return exprType != nullptr && exprType->IsETSUnionType() && asExpr->TsType()->IsETSPrimitiveType();
}

static bool BinaryLoweringAppliable(const ir::AstNode *astNode)
{
    if (!astNode->IsBinaryExpression()) {
        return false;
    }
    auto *binary = astNode->AsBinaryExpression();
    if (binary->OperatorType() == lexer::TokenType::PUNCTUATOR_NULLISH_COALESCING) {
        return false;
    }
    auto *const lhsType = binary->Left()->TsType();
    auto *const rhsType = binary->Right()->TsType();
    if (lhsType == nullptr || rhsType == nullptr) {
        return false;
    }
    if (lhsType->IsETSReferenceType() && rhsType->IsETSReferenceType()) {
        return false;
    }
    if (!lhsType->IsETSUnionType() && !rhsType->IsETSUnionType()) {
        return false;
    }
    return binary->OperationType() != nullptr && binary->OperationType()->IsETSUnionType();
}

static bool InstanceofLoweringAppliable(const ir::AstNode *astNode)
{
    if (!astNode->IsBinaryExpression()) {
        return false;
    }
    auto *binary = astNode->AsBinaryExpression();
    if (binary->OperatorType() != lexer::TokenType::KEYW_INSTANCEOF) {
        return false;
    }
    auto *const lhsType = binary->Left()->TsType();
    auto *const rhsType = binary->Right()->TsType();
    if (lhsType == nullptr || rhsType == nullptr) {
        return false;
    }
    if (lhsType->IsETSReferenceType() && rhsType->IsETSReferenceType()) {
        return false;
    }
    if (rhsType->IsETSUnionType()) {
        return false;
    }
    return lhsType->IsETSPrimitiveType() || lhsType->IsETSUnionType();
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
        [ctx, checker](ir::AstNode *ast) -> ir::AstNode * {
            if (ast->IsMemberExpression() && ast->AsMemberExpression()->Object()->TsType() != nullptr) {
                auto *objType =
                    checker->GetApparentType(checker->GetNonNullishType(ast->AsMemberExpression()->Object()->TsType()));
                if (objType->IsETSUnionType()) {
                    HandleUnionPropertyAccess(checker, checker->VarBinder(), ast->AsMemberExpression());
                    return ast;
                }
            }

            if (CastToPrimitiveLoweringAppliable(ast)) {
                return HandleUnionCastToPrimitive(checker, ast->AsTSAsExpression());
            }

            if (BinaryLoweringAppliable(ast)) {
                return HandleBinaryExpressionWithUnion(ctx, ast->AsBinaryExpression());
            }

            if (InstanceofLoweringAppliable(ast)) {
                return HandleInstanceofWithPrimitive(ctx, ast->AsBinaryExpression());
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
    if (!current || ctx->compilerContext->Options()->compilationMode != CompilationMode::GEN_STD_LIB) {
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
