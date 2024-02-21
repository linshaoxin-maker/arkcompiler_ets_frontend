/**
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

#include "compiler/lowering/scopesInit/scopesInitPhase.h"
#include "varbinder/variableFlags.h"
#include "checker/checker.h"
#include "checker/checkerContext.h"
#include "checker/ets/narrowingWideningConverter.h"
#include "checker/types/globalTypesHolder.h"
#include "checker/types/ets/etsObjectType.h"
#include "ir/astNode.h"
#include "lexer/token/tokenType.h"
#include "ir/base/catchClause.h"
#include "ir/expression.h"
#include "ir/typeNode.h"
#include "ir/base/scriptFunction.h"
#include "ir/base/classProperty.h"
#include "ir/base/methodDefinition.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/classDeclaration.h"
#include "ir/statements/variableDeclarator.h"
#include "ir/statements/switchCaseStatement.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/arrayExpression.h"
#include "ir/expressions/objectExpression.h"
#include "ir/expressions/callExpression.h"
#include "ir/expressions/memberExpression.h"
#include "ir/expressions/literals/booleanLiteral.h"
#include "ir/expressions/literals/charLiteral.h"
#include "ir/expressions/binaryExpression.h"
#include "ir/expressions/assignmentExpression.h"
#include "ir/expressions/arrowFunctionExpression.h"
#include "ir/expressions/literals/numberLiteral.h"
#include "ir/expressions/literals/undefinedLiteral.h"
#include "ir/expressions/literals/nullLiteral.h"
#include "ir/statements/labelledStatement.h"
#include "ir/statements/tryStatement.h"
#include "ir/ets/etsFunctionType.h"
#include "ir/ets/etsNewClassInstanceExpression.h"
#include "ir/ets/etsParameterExpression.h"
#include "ir/ts/tsAsExpression.h"
#include "ir/ts/tsTypeAliasDeclaration.h"
#include "ir/ts/tsEnumMember.h"
#include "ir/ts/tsTypeParameter.h"
#include "ir/ets/etsTypeReference.h"
#include "ir/ets/etsTypeReferencePart.h"
#include "ir/ets/etsPrimitiveType.h"
#include "ir/ts/tsQualifiedName.h"
#include "varbinder/variable.h"
#include "varbinder/scope.h"
#include "varbinder/declaration.h"
#include "parser/ETSparser.h"
#include "parser/program/program.h"
#include "checker/ETSchecker.h"
#include "varbinder/ETSBinder.h"
#include "checker/ets/typeRelationContext.h"
#include "checker/ets/boxingConverter.h"
#include "checker/ets/unboxingConverter.h"
#include "checker/types/ets/types.h"
#include "util/helpers.h"

namespace ark::es2panda::checker {
void ETSChecker::CheckTruthinessOfType(ir::Expression *expr)
{
    checker::Type *type = expr->Check(this);
    auto *unboxedType = ETSBuiltinTypeAsConditionalType(type);

    if (unboxedType == nullptr) {
        ThrowTypeError("Condition must be of possible condition type", expr->Start());
    }

    if (unboxedType == GlobalBuiltinVoidType() || unboxedType->IsETSVoidType()) {
        ThrowTypeError("An expression of type 'void' cannot be tested for truthiness", expr->Start());
    }

    if (!unboxedType->IsConditionalExprType()) {
        ThrowTypeError("Condition must be of possible condition type", expr->Start());
    }

    if (unboxedType->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
        FlagExpressionWithUnboxing(type, unboxedType, expr);
    }
    expr->SetTsType(unboxedType);
}

void ETSChecker::CheckNonNullish(ir::Expression const *expr)
{
    if (expr->TsType()->PossiblyETSNullish()) {
        ThrowTypeError("Value is possibly nullish.", expr->Start());
    }
}

Type *ETSChecker::GetNonNullishType(Type *type)
{
    if (type->DefinitelyNotETSNullish()) {
        return type;
    }
    if (type->IsETSTypeParameter()) {
        return Allocator()->New<ETSNonNullishType>(type->AsETSTypeParameter());
    }
    ArenaVector<Type *> copied(Allocator()->Adapter());
    for (auto const &t : type->AsETSUnionType()->ConstituentTypes()) {
        if (t->IsETSNullType() || t->IsETSUndefinedType()) {
            continue;
        }
        copied.push_back(GetNonNullishType(t));
    }
    return copied.empty() ? GetGlobalTypesHolder()->GlobalBuiltinNeverType() : CreateETSUnionType(std::move(copied));
}

// NOTE(vpukhov): can be implemented with relation if etscompiler will support it
template <bool VISIT_NONNULLISH, typename P>
static bool MatchConstituentOrConstraint(P const &pred, const Type *type)
{
    auto const traverse = [](P const &p, const Type *t) {
        return MatchConstituentOrConstraint<VISIT_NONNULLISH, P>(p, t);
    };
    if (pred(type)) {
        return true;
    }
    if (type->IsETSUnionType()) {
        for (auto const &ctype : type->AsETSUnionType()->ConstituentTypes()) {
            if (traverse(pred, ctype)) {
                return true;
            }
        }
        return false;
    }
    if (type->IsETSTypeParameter()) {
        return traverse(pred, type->AsETSTypeParameter()->GetConstraintType());
    }
    if constexpr (VISIT_NONNULLISH) {
        if (type->IsETSNonNullishType()) {
            auto tparam = type->AsETSNonNullishType()->GetUnderlying();
            return traverse(pred, tparam->GetConstraintType());
        }
    }
    return false;
}

bool Type::PossiblyETSNull() const
{
    const auto pred = [](const Type *t) { return t->IsETSNullType(); };
    return MatchConstituentOrConstraint<false>(pred, this);
}

bool Type::PossiblyETSUndefined() const
{
    const auto pred = [](const Type *t) { return t->IsETSUndefinedType(); };
    return MatchConstituentOrConstraint<false>(pred, this);
}

bool Type::PossiblyETSNullish() const
{
    const auto pred = [](const Type *t) { return t->IsETSNullType() || t->IsETSUndefinedType(); };
    return MatchConstituentOrConstraint<false>(pred, this);
}

bool Type::DefinitelyETSNullish() const
{
    const auto pred = [](const Type *t) {
        return !(t->IsTypeParameter() || t->IsETSUnionType() || t->IsETSNullType() || t->IsETSUndefinedType());
    };
    return !MatchConstituentOrConstraint<false>(pred, this);
}

bool Type::DefinitelyNotETSNullish() const
{
    return !PossiblyETSNullish();
}

bool Type::PossiblyETSString() const
{
    const auto pred = [](const Type *t) {
        return t->IsETSStringType() || (t->IsETSObjectType() && t->AsETSObjectType()->IsGlobalETSObjectType());
    };
    return MatchConstituentOrConstraint<true>(pred, this);
}

bool Type::IsETSReferenceType() const
{
    return HasTypeFlag(checker::TypeFlag::ETS_ARRAY_OR_OBJECT) || IsETSNullType() || IsETSUndefinedType() ||
           IsETSStringType() || IsETSTypeParameter() || IsETSUnionType() || IsETSNonNullishType() || IsETSBigIntType();
}

bool Type::IsETSUnboxableObject() const
{
    return IsETSObjectType() && AsETSObjectType()->HasObjectFlag(ETSObjectFlags::UNBOXABLE_TYPE);
}

bool ETSChecker::IsConstantExpression(ir::Expression *expr, Type *type)
{
    return (type->HasTypeFlag(TypeFlag::CONSTANT) && (expr->IsIdentifier() || expr->IsMemberExpression()));
}

Type *ETSChecker::GetNonConstantTypeFromPrimitiveType(Type *type)
{
    if (type->IsETSStringType()) {
        return GlobalBuiltinETSStringType();
    }

    if (!type->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
        return type;
    }

    if (type->HasTypeFlag(TypeFlag::LONG)) {
        return GlobalLongType();
    }

    if (type->HasTypeFlag(TypeFlag::BYTE)) {
        return GlobalByteType();
    }

    if (type->HasTypeFlag(TypeFlag::SHORT)) {
        return GlobalShortType();
    }

    if (type->HasTypeFlag(TypeFlag::CHAR)) {
        return GlobalCharType();
    }

    if (type->HasTypeFlag(TypeFlag::INT)) {
        return GlobalIntType();
    }

    if (type->HasTypeFlag(TypeFlag::FLOAT)) {
        return GlobalFloatType();
    }

    if (type->HasTypeFlag(TypeFlag::DOUBLE)) {
        return GlobalDoubleType();
    }

    if (type->IsETSBooleanType()) {
        return GlobalETSBooleanType();
    }
    return type;
}

Type *ETSChecker::GetTypeOfVariable(varbinder::Variable *const var)
{
    if (IsVariableGetterSetter(var)) {
        return GetTypeForGetterSetter(var);
    }

    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    // NOTE: kbaladurin. forbid usage of imported entities as types without declarations
    if (VarBinder()->AsETSBinder()->IsDynamicModuleVariable(var)) {
        return GetTypeForDynamicModuleVariable(var);
    }

    varbinder::Decl *decl = var->Declaration();

    // Before computing the given variables type, we have to make a new checker context frame so that the checking is
    // done in the proper context, and have to enter the scope where the given variable is declared, so reference
    // resolution works properly
    checker::SavedCheckerContext savedContext(this, CheckerStatus::NO_OPTS);
    checker::ScopeContext scopeCtx(this, var->GetScope());
    SetUpContextForNodeHierarchy(decl->Node()->Parent());

    switch (decl->Type()) {
        case varbinder::DeclType::CLASS: {
            auto *classDef = decl->Node()->AsClassDefinition();
            BuildClassProperties(classDef);
            return classDef->TsType();
        }
        case varbinder::DeclType::ENUM_LITERAL:
        case varbinder::DeclType::CONST:
        case varbinder::DeclType::LET:
        case varbinder::DeclType::VAR: {
            auto *declNode = decl->Node();

            if (decl->Node()->IsIdentifier()) {
                declNode = declNode->Parent();
            }

            return declNode->Check(this);
        }
        case varbinder::DeclType::FUNC: {
            return decl->Node()->Check(this);
        }
        case varbinder::DeclType::IMPORT: {
            return decl->Node()->Check(this);
        }
        case varbinder::DeclType::TYPE_ALIAS: {
            return GetTypeFromTypeAliasReference(var);
        }
        case varbinder::DeclType::INTERFACE: {
            return BuildInterfaceProperties(decl->Node()->AsTSInterfaceDeclaration());
        }
        default: {
            UNREACHABLE();
        }
    }

    return var->TsType();
}

Type *ETSChecker::GetTypeForGetterSetter(varbinder::Variable *const var)
{
    auto *propType = var->TsType()->AsETSFunctionType();
    if (propType->HasTypeFlag(checker::TypeFlag::GETTER)) {
        return propType->FindGetter()->ReturnType();
    }
    return propType->FindSetter()->Params()[0]->TsType();
}

Type *ETSChecker::GetTypeForDynamicModuleVariable(varbinder::Variable *const var)
{
    auto *importData = VarBinder()->AsETSBinder()->DynamicImportDataForVar(var);
    if (importData->import->IsPureDynamic()) {
        return GlobalBuiltinDynamicType(importData->import->Language());
    }
    return nullptr;
}

void ETSChecker::SetUpContextForNodeHierarchy(ir::AstNode *iter)
{
    while (iter != nullptr) {
        if (iter->IsMethodDefinition()) {
            auto *methodDef = iter->AsMethodDefinition();
            ASSERT(methodDef->TsType());
            Context().SetContainingSignature(methodDef->Function()->Signature());
        }

        if (iter->IsClassDefinition()) {
            auto *classDef = iter->AsClassDefinition();
            ETSObjectType *containingClass {};

            if (classDef->TsType() == nullptr) {
                containingClass = BuildClassProperties(classDef);
            } else {
                containingClass = classDef->TsType()->AsETSObjectType();
            }

            ASSERT(classDef->TsType());
            Context().SetContainingClass(containingClass);
        }

        iter = iter->Parent();
    }
}

// Determine if unchecked cast is needed and yield guaranteed source type
Type *ETSChecker::GuaranteedTypeForUncheckedCast(Type *base, Type *substituted)
{
    // Apparent type acts as effective representation for type.
    //  For T extends SomeClass|undefined
    //  Apparent(Int|T|null) is Int|SomeClass|undefined|null
    auto *appBase = GetApparentType(base);
    auto *appSubst = GetApparentType(substituted);
    // Base is supertype of Substituted AND Substituted is supertype of Base
    return Relation()->IsIdenticalTo(appSubst, appBase) ? nullptr : appBase;
}

// Determine if substituted property access requires cast from erased type
Type *ETSChecker::GuaranteedTypeForUncheckedPropertyAccess(varbinder::Variable *const prop)
{
    if (IsVariableStatic(prop)) {
        return nullptr;
    }
    if (IsVariableGetterSetter(prop)) {
        auto *method = prop->TsType()->AsETSFunctionType();
        if (!method->HasTypeFlag(checker::TypeFlag::GETTER)) {
            return nullptr;
        }
        return GuaranteedTypeForUncheckedCallReturn(method->FindGetter());
    }
    // NOTE(vpukhov): mark ETSDynamicType properties
    if (prop->Declaration() == nullptr || prop->Declaration()->Node() == nullptr) {
        return nullptr;
    }

    auto *baseProp = prop->Declaration()->Node()->AsClassProperty()->Id()->Variable();
    if (baseProp == prop) {
        return nullptr;
    }
    return GuaranteedTypeForUncheckedCast(GetTypeOfVariable(baseProp), GetTypeOfVariable(prop));
}

// Determine if substituted method cast requires cast from erased type
Type *ETSChecker::GuaranteedTypeForUncheckedCallReturn(Signature *sig)
{
    if (sig->HasSignatureFlag(checker::SignatureFlags::THIS_RETURN_TYPE)) {
        return sig->ReturnType();
    }
    auto *baseSig = sig->Function()->Signature();
    if (baseSig == sig) {
        return nullptr;
    }
    return GuaranteedTypeForUncheckedCast(baseSig->ReturnType(), sig->ReturnType());
}

void ETSChecker::CheckEtsFunctionType(ir::Identifier *const ident, ir::Identifier const *const id,
                                      ir::TypeNode const *const annotation)
{
    if (annotation == nullptr) {
        ThrowTypeError(
            {"Cannot infer type for ", id->Name(), " because method reference needs an explicit target type"},
            id->Start());
    }

    const auto *const targetType = GetTypeOfVariable(id->Variable());
    ASSERT(targetType != nullptr);

    if (!targetType->IsETSObjectType() || !targetType->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::FUNCTIONAL)) {
        ThrowError(ident);
    }
}

bool ETSChecker::IsTypeBuiltinType(const Type *type) const
{
    if (!type->IsETSObjectType()) {
        return false;
    }

    switch (type->AsETSObjectType()->BuiltInKind()) {
        case ETSObjectFlags::BUILTIN_BOOLEAN:
        case ETSObjectFlags::BUILTIN_BYTE:
        case ETSObjectFlags::BUILTIN_SHORT:
        case ETSObjectFlags::BUILTIN_CHAR:
        case ETSObjectFlags::BUILTIN_INT:
        case ETSObjectFlags::BUILTIN_LONG:
        case ETSObjectFlags::BUILTIN_FLOAT:
        case ETSObjectFlags::BUILTIN_DOUBLE: {
            return true;
        }
        default:
            return false;
    }
}

Type *ETSChecker::GetTypeFromTypeAliasReference(varbinder::Variable *var)
{
    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    auto *const aliasTypeNode = var->Declaration()->Node()->AsTSTypeAliasDeclaration();
    TypeStackElement tse(this, aliasTypeNode, "Circular type alias reference", aliasTypeNode->Start());
    aliasTypeNode->Check(this);
    auto *const aliasedType = aliasTypeNode->TypeAnnotation()->GetType(this);

    var->SetTsType(aliasedType);
    return aliasedType;
}

Type *ETSChecker::GetTypeFromInterfaceReference(varbinder::Variable *var)
{
    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    auto *interfaceType = BuildBasicInterfaceProperties(var->Declaration()->Node()->AsTSInterfaceDeclaration());
    var->SetTsType(interfaceType);
    return interfaceType;
}

Type *ETSChecker::GetTypeFromClassReference(varbinder::Variable *var)
{
    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    auto *classType = BuildBasicClassProperties(var->Declaration()->Node()->AsClassDefinition());
    var->SetTsType(classType);
    return classType;
}

Type *ETSChecker::GetTypeFromEnumReference([[maybe_unused]] varbinder::Variable *var)
{
    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    auto const *const enumDecl = var->Declaration()->Node()->AsTSEnumDeclaration();
    if (auto *const itemInit = enumDecl->Members().front()->AsTSEnumMember()->Init(); itemInit->IsNumberLiteral()) {
        return CreateETSEnumType(enumDecl);
    } else if (itemInit->IsStringLiteral()) {  // NOLINT(readability-else-after-return)
        return CreateETSStringEnumType(enumDecl);
    } else {  // NOLINT(readability-else-after-return)
        ThrowTypeError("Invalid enumeration value type.", enumDecl->Start());
    }
}

Type *ETSChecker::GetTypeFromTypeParameterReference(varbinder::LocalVariable *var, const lexer::SourcePosition &pos)
{
    ASSERT(var->Declaration()->Node()->IsTSTypeParameter());
    if ((var->Declaration()->Node()->AsTSTypeParameter()->Parent()->Parent()->IsClassDefinition() ||
         var->Declaration()->Node()->AsTSTypeParameter()->Parent()->Parent()->IsTSInterfaceDeclaration()) &&
        HasStatus(CheckerStatus::IN_STATIC_CONTEXT)) {
        ThrowTypeError({"Cannot make a static reference to the non-static type ", var->Name()}, pos);
    }

    return var->TsType();
}

void ETSChecker::CheckUnboxedTypesAssignable(TypeRelation *relation, Type *source, Type *target)
{
    auto *unboxedSourceType = relation->GetChecker()->AsETSChecker()->ETSBuiltinTypeAsPrimitiveType(source);
    auto *unboxedTargetType = relation->GetChecker()->AsETSChecker()->ETSBuiltinTypeAsPrimitiveType(target);
    if (unboxedSourceType == nullptr || unboxedTargetType == nullptr) {
        return;
    }
    relation->IsAssignableTo(unboxedSourceType, unboxedTargetType);
    if (relation->IsTrue()) {
        relation->GetNode()->AddBoxingUnboxingFlags(
            relation->GetChecker()->AsETSChecker()->GetUnboxingFlag(unboxedSourceType));
    }
}

void ETSChecker::CheckBoxedSourceTypeAssignable(TypeRelation *relation, Type *source, Type *target)
{
    ASSERT(relation != nullptr);
    checker::SavedTypeRelationFlagsContext savedTypeRelationFlagCtx(
        relation, (relation->ApplyWidening() ? TypeRelationFlag::WIDENING : TypeRelationFlag::NONE) |
                      (relation->ApplyNarrowing() ? TypeRelationFlag::NARROWING : TypeRelationFlag::NONE));
    auto *boxedSourceType = relation->GetChecker()->AsETSChecker()->PrimitiveTypeAsETSBuiltinType(source);
    if (boxedSourceType == nullptr) {
        return;
    }
    ASSERT(target != nullptr);
    // Do not box primitive in case of cast to dynamic types
    if (target->IsETSDynamicType()) {
        return;
    }
    relation->IsAssignableTo(boxedSourceType, target);
    if (relation->IsTrue() && !relation->OnlyCheckBoxingUnboxing()) {
        AddBoxingFlagToPrimitiveType(relation, boxedSourceType);
    } else {
        auto unboxedTargetType = ETSBuiltinTypeAsPrimitiveType(target);
        if (unboxedTargetType == nullptr) {
            return;
        }
        NarrowingWideningConverter(this, relation, unboxedTargetType, source);
        if (relation->IsTrue()) {
            AddBoxingFlagToPrimitiveType(relation, target);
        }
    }
}

void ETSChecker::CheckUnboxedSourceTypeWithWideningAssignable(TypeRelation *relation, Type *source, Type *target)
{
    auto *unboxedSourceType = relation->GetChecker()->AsETSChecker()->ETSBuiltinTypeAsPrimitiveType(source);
    if (unboxedSourceType == nullptr) {
        return;
    }
    relation->IsAssignableTo(unboxedSourceType, target);
    if (!relation->IsTrue() && relation->ApplyWidening()) {
        relation->GetChecker()->AsETSChecker()->CheckUnboxedTypeWidenable(relation, target, unboxedSourceType);
    }
    if (!relation->OnlyCheckBoxingUnboxing()) {
        relation->GetNode()->AddBoxingUnboxingFlags(
            relation->GetChecker()->AsETSChecker()->GetUnboxingFlag(unboxedSourceType));
    }
}

static ir::AstNode *DerefETSTypeReference(ir::AstNode *node)
{
    ASSERT(node->IsETSTypeReference());
    do {
        auto *name = node->AsETSTypeReference()->Part()->Name();
        ASSERT(name->IsIdentifier());
        auto *var = name->AsIdentifier()->Variable();
        ASSERT(var != nullptr);
        auto *declNode = var->Declaration()->Node();
        if (!declNode->IsTSTypeAliasDeclaration()) {
            return declNode;
        }
        node = declNode->AsTSTypeAliasDeclaration()->TypeAnnotation();
    } while (node->IsETSTypeReference());
    return node;
}

bool ETSChecker::CheckLambdaAssignable(ir::Expression *param, ir::ScriptFunction *lambda)
{
    ASSERT(param->IsETSParameterExpression());
    ir::AstNode *typeAnn = param->AsETSParameterExpression()->Ident()->TypeAnnotation();
    if (typeAnn->IsETSTypeReference()) {
        typeAnn = DerefETSTypeReference(typeAnn);
    }
    if (!typeAnn->IsETSFunctionType()) {
        return false;
    }
    ir::ETSFunctionType *calleeType = typeAnn->AsETSFunctionType();
    return lambda->Params().size() == calleeType->Params().size();
}

void ETSChecker::InferTypesForLambda(ir::ScriptFunction *lambda, ir::ETSFunctionType *calleeType)
{
    for (size_t i = 0; i < calleeType->Params().size(); ++i) {
        const auto *const calleeParam = calleeType->Params()[i]->AsETSParameterExpression()->Ident();
        auto *const lambdaParam = lambda->Params()[i]->AsETSParameterExpression()->Ident();
        if (lambdaParam->TypeAnnotation() == nullptr) {
            auto *const typeAnnotation = calleeParam->TypeAnnotation()->Clone(Allocator(), lambdaParam);
            lambdaParam->SetTsTypeAnnotation(typeAnnotation);
            typeAnnotation->SetParent(lambdaParam);
        }
    }
    if (lambda->ReturnTypeAnnotation() == nullptr) {
        lambda->SetReturnTypeAnnotation(calleeType->ReturnType());
    }
}

bool ETSChecker::TypeInference(Signature *signature, const ArenaVector<ir::Expression *> &arguments,
                               TypeRelationFlag flags)
{
    bool invocable = true;
    auto const argumentCount = arguments.size();
    auto const parameterCount = signature->Params().size();
    auto const count = std::min(parameterCount, argumentCount);

    for (size_t index = 0U; index < count; ++index) {
        auto const &argument = arguments[index];
        if (!argument->IsArrowFunctionExpression()) {
            continue;
        }

        if (index == arguments.size() - 1 && (flags & TypeRelationFlag::NO_CHECK_TRAILING_LAMBDA) != 0) {
            continue;
        }

        auto *const arrowFuncExpr = argument->AsArrowFunctionExpression();
        ir::ScriptFunction *const lambda = arrowFuncExpr->Function();
        if (!NeedTypeInference(lambda)) {
            continue;
        }

        auto const *const param = signature->Function()->Params()[index]->AsETSParameterExpression()->Ident();
        ir::AstNode *typeAnn = param->TypeAnnotation();

        if (typeAnn->IsETSTypeReference()) {
            typeAnn = DerefETSTypeReference(typeAnn);
        }

        ASSERT(typeAnn->IsETSFunctionType());
        InferTypesForLambda(lambda, typeAnn->AsETSFunctionType());
        Type *const argType = arrowFuncExpr->Check(this);
        const Type *targetType = TryGettingFunctionTypeFromInvokeFunction(signature->Params()[index]->TsType());
        const std::initializer_list<TypeErrorMessageElement> msg = {
            "Type '", argType, "' is not compatible with type '", targetType, "' at index ", index + 1};

        checker::InvocationContext invokationCtx(Relation(), arguments[index], argType,
                                                 signature->Params()[index]->TsType(), arrowFuncExpr->Start(), msg,
                                                 flags);

        invocable &= invokationCtx.IsInvocable();
    }
    return invocable;
}
}  // namespace ark::es2panda::checker
