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

namespace panda::es2panda::checker {
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

    if (unboxedType != nullptr && unboxedType->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
        FlagExpressionWithUnboxing(type, unboxedType, expr);
    }
    expr->SetTsType(unboxedType);
}

// NOTE: vpukhov. this entire function is isolated work-around until nullish type are not unions
Type *ETSChecker::CreateNullishType(Type *type, checker::TypeFlag nullishFlags, ArenaAllocator *allocator,
                                    TypeRelation *relation, GlobalTypesHolder *globalTypes)
{
    ASSERT((nullishFlags & ~TypeFlag::NULLISH) == 0);

    auto *const nullish = type->Instantiate(allocator, relation, globalTypes);

    // Doesnt work for primitive array types, because instantiated type is equal to original one

    if ((nullishFlags & TypeFlag::NULL_TYPE) != 0) {
        nullish->AddTypeFlag(checker::TypeFlag::NULL_TYPE);
    }
    if ((nullishFlags & TypeFlag::UNDEFINED) != 0) {
        nullish->AddTypeFlag(checker::TypeFlag::UNDEFINED);
        if (nullish->IsETSObjectType()) {
            nullish->AsETSObjectType()->SetAssemblerName(GlobalETSObjectType()->AssemblerName());
        }
    }
    ASSERT(!nullish->HasTypeFlag(TypeFlag::ETS_PRIMITIVE));
    return nullish;
}

void ETSChecker::CheckNonNullishType([[maybe_unused]] Type *type, [[maybe_unused]] lexer::SourcePosition lineInfo)
{
    // NOTE: vpukhov. enable check when type inference is implemented
    (void)type;
}

// NOTE: vpukhov. rewrite with union types
Type *ETSChecker::GetNonNullishType(Type *type) const
{
    if (type->IsETSArrayType()) {
        return type;  // give up
    }
    if (type->IsETSTypeParameter()) {
        return type->AsETSTypeParameter()->GetOriginal();
    }

    while (type->IsNullish()) {
        type = type->AsETSObjectType()->GetBaseType();
        ASSERT(type != nullptr);
    }
    return type;
}

// NOTE: vpukhov. rewrite with union types
const Type *ETSChecker::GetNonNullishType(const Type *type) const
{
    if (type->IsETSArrayType()) {
        return type;  // give up
    }
    if (type->IsETSTypeParameter()) {
        return type->AsETSTypeParameter()->GetOriginal();
    }

    while (type->IsNullish()) {
        type = type->AsETSObjectType()->GetBaseType();
        ASSERT(type != nullptr);
    }
    return type;
}

Type *ETSChecker::CreateOptionalResultType(Type *type)
{
    if (type->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE)) {
        type = PrimitiveTypeAsETSBuiltinType(type);
        ASSERT(type->IsETSObjectType());
        Relation()->GetNode()->AddBoxingUnboxingFlags(GetBoxingFlag(type));
    }

    return CreateNullishType(type, checker::TypeFlag::UNDEFINED, Allocator(), Relation(), GetGlobalTypesHolder());
}

// NOTE(vpukhov): #14595 could be implemented with relation
template <typename P>
static bool MatchConstitutentOrConstraint(P const &pred, const Type *type)
{
    if (pred(type)) {
        return true;
    }
    if (type->IsETSUnionType()) {
        for (auto const &ctype : type->AsETSUnionType()->ConstituentTypes()) {
            if (MatchConstitutentOrConstraint(pred, ctype)) {
                return true;
            }
        }
        return false;
    }
    if (type->IsETSTypeParameter()) {
        return MatchConstitutentOrConstraint(pred, type->AsETSTypeParameter()->GetConstraintType());
    }
    return false;
}

bool ETSChecker::MayHaveNullValue(const Type *type) const
{
    const auto pred = [](const Type *t) { return t->ContainsNull() || t->IsETSNullType(); };
    return MatchConstitutentOrConstraint(pred, type);
}

bool ETSChecker::MayHaveUndefinedValue(const Type *type) const
{
    const auto pred = [](const Type *t) { return t->ContainsUndefined() || t->IsETSUndefinedType(); };
    return MatchConstitutentOrConstraint(pred, type);
}

bool ETSChecker::MayHaveNulllikeValue(const Type *type) const
{
    const auto pred = [](const Type *t) { return t->IsNullishOrNullLike(); };
    return MatchConstitutentOrConstraint(pred, type);
}

bool ETSChecker::IsConstantExpression(ir::Expression *expr, Type *type)
{
    return (type->HasTypeFlag(TypeFlag::CONSTANT) && (expr->IsIdentifier() || expr->IsMemberExpression()));
}

Type *ETSChecker::GetNonConstantTypeFromPrimitiveType(Type *type)
{
    if (type->IsETSStringType()) {
        // NOTE: vpukhov. remove when nullish types are unions
        ASSERT(!type->IsNullish());
        return GlobalBuiltinETSStringType();
    }

    if (!type->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
        return type;
    }

    // NOTE: vpukhov. remove when nullish types are unions
    ASSERT(!type->IsNullish());

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
    if (!base->IsETSTypeParameter()) {
        return nullptr;
    }
    auto *constr = base->AsETSTypeParameter()->GetConstraintType();
    // Constraint is supertype of TypeArg AND TypeArg is supertype of Constraint
    return Relation()->IsIdenticalTo(substituted, constr) ? nullptr : constr;
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

bool ETSChecker::IsReferenceType(const Type *type)
{
    return type->HasTypeFlag(checker::TypeFlag::ETS_ARRAY_OR_OBJECT) || type->IsETSNullLike() ||
           type->IsETSStringType() || type->IsETSTypeParameter() || type->IsETSUnionType() || type->IsETSBigIntType();
}

Type *ETSChecker::GetTypeFromTypeAliasReference(varbinder::Variable *var)
{
    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    auto *const aliasTypeNode = var->Declaration()->Node()->AsTSTypeAliasDeclaration();
    TypeStackElement tse(this, aliasTypeNode, "Circular type alias reference", aliasTypeNode->Start());
    aliasTypeNode->Check(this);
    auto *const aliasedType = GetTypeFromTypeAnnotation(aliasTypeNode->TypeAnnotation());

    var->SetTsType(aliasedType);
    return aliasedType;
}

Type *ETSChecker::GetTypeFromInterfaceReference(varbinder::Variable *var)
{
    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    auto *interfaceType = BuildInterfaceProperties(var->Declaration()->Node()->AsTSInterfaceDeclaration());
    var->SetTsType(interfaceType);
    return interfaceType;
}

Type *ETSChecker::GetTypeFromClassReference(varbinder::Variable *var)
{
    if (var->TsType() != nullptr) {
        return var->TsType();
    }

    auto *classType = BuildClassProperties(var->Declaration()->Node()->AsClassDefinition());
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

checker::Type *ETSChecker::CheckArrayElements(ir::Identifier *ident, ir::ArrayExpression *init)
{
    ArenaVector<ir::Expression *> elements = init->AsArrayExpression()->Elements();
    checker::Type *annotationType = nullptr;
    if (elements.empty()) {
        annotationType = Allocator()->New<ETSArrayType>(GlobalETSObjectType());
    } else {
        auto type = elements[0]->Check(this);
        auto const primType = ETSBuiltinTypeAsPrimitiveType(type);
        for (auto element : elements) {
            auto const eType = element->Check(this);
            auto const primEType = ETSBuiltinTypeAsPrimitiveType(eType);
            if (primEType != nullptr && primType != nullptr && primEType->HasTypeFlag(TypeFlag::ETS_NUMERIC) &&
                primType->HasTypeFlag(TypeFlag::ETS_NUMERIC)) {
                type = GlobalDoubleType();
            } else if (IsTypeIdenticalTo(type, eType)) {
                continue;
            } else {
                // NOTE: Create union type when implemented here
                ThrowTypeError({"Union type is not implemented yet!"}, ident->Start());
            }
        }
        annotationType = Allocator()->New<ETSArrayType>(type);
    }
    return annotationType;
}

Type *ETSChecker::GetReferencedTypeBase(ir::Expression *name)
{
    if (name->IsTSQualifiedName()) {
        auto *qualified = name->AsTSQualifiedName();
        return qualified->Check(this);
    }

    ASSERT(name->IsIdentifier() && name->AsIdentifier()->Variable() != nullptr);

    // NOTE: kbaladurin. forbid usage imported entities as types without declarations
    auto *importData = VarBinder()->AsETSBinder()->DynamicImportDataForVar(name->AsIdentifier()->Variable());
    if (importData != nullptr && importData->import->IsPureDynamic()) {
        return GlobalBuiltinDynamicType(importData->import->Language());
    }

    auto *refVar = name->AsIdentifier()->Variable()->AsLocalVariable();

    switch (refVar->Declaration()->Node()->Type()) {
        case ir::AstNodeType::TS_INTERFACE_DECLARATION: {
            return GetTypeFromInterfaceReference(refVar);
        }
        case ir::AstNodeType::CLASS_DECLARATION:
        case ir::AstNodeType::STRUCT_DECLARATION:
        case ir::AstNodeType::CLASS_DEFINITION: {
            return GetTypeFromClassReference(refVar);
        }
        case ir::AstNodeType::TS_ENUM_DECLARATION: {
            return GetTypeFromEnumReference(refVar);
        }
        case ir::AstNodeType::TS_TYPE_PARAMETER: {
            return GetTypeFromTypeParameterReference(refVar, name->Start());
        }
        case ir::AstNodeType::TS_TYPE_ALIAS_DECLARATION: {
            return GetTypeFromTypeAliasReference(refVar);
        }
        default: {
            UNREACHABLE();
        }
    }
}

Type *ETSChecker::GetReferencedTypeFromBase([[maybe_unused]] Type *baseType, [[maybe_unused]] ir::Expression *name)
{
    return nullptr;
}

Type *ETSChecker::GetTypeFromTypeAnnotation(ir::TypeNode *const typeAnnotation)
{
    auto *type = typeAnnotation->GetType(this);

    if (!typeAnnotation->IsNullAssignable() && !typeAnnotation->IsUndefinedAssignable()) {
        return type;
    }

    if (!IsReferenceType(type)) {
        ThrowTypeError("Non reference types cannot be nullish.", typeAnnotation->Start());
    }

    if (type->IsNullish()) {
        return type;
    }

    TypeFlag nullishFlags {0};
    if (typeAnnotation->IsNullAssignable()) {
        nullishFlags |= TypeFlag::NULL_TYPE;
    }
    if (typeAnnotation->IsUndefinedAssignable()) {
        nullishFlags |= TypeFlag::UNDEFINED;
    }
    return CreateNullishType(type, nullishFlags, Allocator(), Relation(), GetGlobalTypesHolder());
}
}  // namespace panda::es2panda::checker
