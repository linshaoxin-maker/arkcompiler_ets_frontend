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
varbinder::Variable *ETSChecker::FindVariableInFunctionScope(const util::StringView name)
{
    return Scope()->FindInFunctionScope(name, varbinder::ResolveBindingOptions::ALL).variable;
}

std::pair<const varbinder::Variable *, const ETSObjectType *> ETSChecker::FindVariableInClassOrEnclosing(
    const util::StringView name, const ETSObjectType *classType)
{
    const auto searchFlags = PropertySearchFlags::SEARCH_ALL | PropertySearchFlags::SEARCH_IN_BASE |
                             PropertySearchFlags::SEARCH_IN_INTERFACES;
    auto *resolved = classType->GetProperty(name, searchFlags);
    while (classType->EnclosingType() != nullptr && resolved == nullptr) {
        classType = classType->EnclosingType();
        resolved = classType->GetProperty(name, searchFlags);
    }

    return {resolved, classType};
}

varbinder::Variable *ETSChecker::FindVariableInGlobal(const ir::Identifier *const identifier)
{
    return Scope()->FindInGlobal(identifier->Name(), varbinder::ResolveBindingOptions::ALL).variable;
}

bool ETSChecker::IsVariableStatic(const varbinder::Variable *var)
{
    if (var->HasFlag(varbinder::VariableFlags::METHOD)) {
        return var->TsType()->AsETSFunctionType()->CallSignatures()[0]->HasSignatureFlag(SignatureFlags::STATIC);
    }
    return var->HasFlag(varbinder::VariableFlags::STATIC);
}

bool ETSChecker::IsVariableGetterSetter(const varbinder::Variable *var)
{
    return var->TsType() != nullptr && var->TsType()->HasTypeFlag(TypeFlag::GETTER_SETTER);
}

void ETSChecker::ThrowError(ir::Identifier *const ident)
{
    ThrowTypeError({"Unresolved reference ", ident->Name()}, ident->Start());
}

void ETSChecker::WrongContextErrorClassifyByType(ir::Identifier *ident, varbinder::Variable *const resolved)
{
    std::string identCategoryName;
    switch (static_cast<varbinder::VariableFlags>(
        resolved->Flags() &
        (varbinder::VariableFlags::CLASS_OR_INTERFACE_OR_ENUM | varbinder::VariableFlags::METHOD))) {
        case varbinder::VariableFlags::CLASS: {
            identCategoryName = "Class";
            break;
        }
        case varbinder::VariableFlags::METHOD: {
            identCategoryName = "Function";
            break;
        }
        case varbinder::VariableFlags::INTERFACE: {
            identCategoryName = "Interface";
            break;
        }
        case varbinder::VariableFlags::ENUM_LITERAL: {
            identCategoryName = "Enum";
            break;
        }
        default: {
            ThrowError(ident);
        }
    }
    ThrowTypeError({identCategoryName.c_str(), " name \"", ident->Name(), "\" used in the wrong context"},
                   ident->Start());
}

void ETSChecker::NotResolvedError(ir::Identifier *const ident, const varbinder::Variable *classVar,
                                  const ETSObjectType *classType)
{
    if (classVar == nullptr) {
        ThrowError(ident);
    }

    if (IsVariableStatic(classVar)) {
        ThrowTypeError(
            {"Static property '", ident->Name(), "' must be accessed through it's class '", classType->Name(), "'"},
            ident->Start());
    } else {
        ThrowTypeError({"Property '", ident->Name(), "' must be accessed through 'this'"}, ident->Start());
    }
}

std::pair<const ir::Identifier *, ir::TypeNode *> ETSChecker::GetTargetIdentifierAndType(ir::Identifier *const ident)
{
    if (ident->Parent()->IsClassProperty()) {
        const auto *const classProp = ident->Parent()->AsClassProperty();
        ASSERT(classProp->Value() && classProp->Value() == ident);
        return std::make_pair(classProp->Key()->AsIdentifier(), classProp->TypeAnnotation());
    }
    const auto *const variableDecl = ident->Parent()->AsVariableDeclarator();
    ASSERT(variableDecl->Init() && variableDecl->Init() == ident);
    return std::make_pair(variableDecl->Id()->AsIdentifier(), variableDecl->Id()->AsIdentifier()->TypeAnnotation());
}

void ETSChecker::ExtraCheckForResolvedError(ir::Identifier *const ident)
{
    const auto [class_var, class_type] = FindVariableInClassOrEnclosing(ident->Name(), Context().ContainingClass());
    auto *parentClass = FindAncestorGivenByType(ident, ir::AstNodeType::CLASS_DEFINITION);
    if (parentClass != nullptr && parentClass->AsClassDefinition()->IsLocal()) {
        if (parentClass != class_type->GetDeclNode()) {
            ThrowTypeError({"Property '", ident->Name(), "' of enclosing class '", class_type->Name(),
                            "' is not allowed to be captured from the local class '",
                            parentClass->AsClassDefinition()->Ident()->Name(), "'"},
                           ident->Start());
        }
    }
    NotResolvedError(ident, class_var, class_type);
}

bool ETSChecker::SaveCapturedVariableInLocalClass(varbinder::Variable *const var, ir::Identifier *ident)
{
    const auto &pos = ident->Start();

    if (!HasStatus(CheckerStatus::IN_LOCAL_CLASS)) {
        return false;
    }

    if (!var->HasFlag(varbinder::VariableFlags::LOCAL)) {
        return false;
    }

    LOG(DEBUG, ES2PANDA) << "Checking variable (line:" << pos.line << "): " << var->Name();
    auto *scopeIter = Scope();
    bool inStaticMethod = false;

    auto captureVariable = [this, var, ident, &scopeIter, &inStaticMethod, &pos]() {
        if (inStaticMethod) {
            ThrowTypeError({"Not allowed to capture variable '", var->Name(), "' in static method"}, pos);
        }
        if (scopeIter->Node()->AsClassDefinition()->CaptureVariable(var)) {
            LOG(DEBUG, ES2PANDA) << "  Captured in class:" << scopeIter->Node()->AsClassDefinition()->Ident()->Name();
        }

        auto *parent = ident->Parent();

        if (parent->IsVariableDeclarator()) {
            parent = parent->Parent()->Parent();
        }

        if (!(parent->IsUpdateExpression() ||
              (parent->IsAssignmentExpression() && parent->AsAssignmentExpression()->Left() == ident)) ||
            var->Declaration() == nullptr) {
            return;
        }

        if (var->Declaration()->IsParameterDecl()) {
            LOG(DEBUG, ES2PANDA) << "    - Modified parameter ";
            if (!var->HasFlag(varbinder::VariableFlags::BOXED)) {
                scopeIter->Node()->AsClassDefinition()->AddToLocalVariableIsNeeded(var);
            }
        } else {
            var->AddFlag(varbinder::VariableFlags::BOXED);
        }
    };

    while (scopeIter != var->GetScope()) {
        if (scopeIter->Node() != nullptr) {
            if (scopeIter->Node()->IsScriptFunction() && scopeIter->Node()->AsScriptFunction()->IsStatic()) {
                inStaticMethod = true;
            }

            if (scopeIter->Node()->IsClassDefinition()) {
                captureVariable();
                return true;
            }
        }
        scopeIter = scopeIter->Parent();
    }

    return false;
}

void ETSChecker::SaveCapturedVariable(varbinder::Variable *const var, ir::Identifier *ident)
{
    const auto &pos = ident->Start();

    if (SaveCapturedVariableInLocalClass(var, ident)) {
        return;
    }

    if (!HasStatus(CheckerStatus::IN_LAMBDA)) {
        return;
    }

    if (var->HasFlag(varbinder::VariableFlags::PROPERTY)) {
        Context().AddCapturedVar(var, pos);
        return;
    }

    if ((!var->HasFlag(varbinder::VariableFlags::LOCAL) && !var->HasFlag(varbinder::VariableFlags::METHOD)) ||
        (var->GetScope()->Node()->IsScriptFunction() && var->GetScope()->Node()->AsScriptFunction()->IsArrow())) {
        return;
    }

    const auto *scopeIter = Scope();
    while (scopeIter != var->GetScope()) {
        if (scopeIter->IsFunctionScope()) {
            Context().AddCapturedVar(var, pos);
            return;
        }
        scopeIter = scopeIter->Parent();
    }
}

Type *ETSChecker::ResolveIdentifier(ir::Identifier *const ident)
{
    if (ident->Variable() != nullptr) {
        auto *const resolved = ident->Variable();
        SaveCapturedVariable(resolved, ident);
        return GetTypeOfVariable(resolved);
    }

    auto *resolved = FindVariableInFunctionScope(ident->Name());
    if (resolved == nullptr) {
        // If the reference is not found already in the current class, then it is not bound to the class, so we have to
        // find the reference in the global class first, then in the global scope
        resolved = FindVariableInGlobal(ident);
    }

    // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
    ValidateResolvedIdentifier(ident, resolved);

    ValidatePropertyAccess(resolved, Context().ContainingClass(), ident->Start());
    SaveCapturedVariable(resolved, ident);

    ident->SetVariable(resolved);
    return GetTypeOfVariable(resolved);
}

std::tuple<Type *, bool> ETSChecker::ApplyBinaryOperatorPromotion(Type *left, Type *right, TypeFlag test,
                                                                  bool doPromotion)
{
    Type *unboxedL = ETSBuiltinTypeAsPrimitiveType(left);
    Type *unboxedR = ETSBuiltinTypeAsPrimitiveType(right);
    bool bothConst = false;

    if (unboxedL == nullptr || unboxedR == nullptr) {
        return {nullptr, false};
    }

    if (!unboxedL->HasTypeFlag(test) || !unboxedR->HasTypeFlag(test)) {
        return {nullptr, false};
    }

    if (unboxedL->HasTypeFlag(TypeFlag::CONSTANT) && unboxedR->HasTypeFlag(TypeFlag::CONSTANT)) {
        bothConst = true;
    }
    if (doPromotion) {
        if (unboxedL->HasTypeFlag(TypeFlag::ETS_NUMERIC) && unboxedR->HasTypeFlag(TypeFlag::ETS_NUMERIC)) {
            if (unboxedL->IsDoubleType() || unboxedR->IsDoubleType()) {
                return {GlobalDoubleType(), bothConst};
            }

            if (unboxedL->IsFloatType() || unboxedR->IsFloatType()) {
                return {GlobalFloatType(), bothConst};
            }

            if (unboxedL->IsLongType() || unboxedR->IsLongType()) {
                return {GlobalLongType(), bothConst};
            }

            if (unboxedL->IsCharType() && unboxedR->IsCharType()) {
                return {GlobalCharType(), bothConst};
            }

            return {GlobalIntType(), bothConst};
        }

        if (IsTypeIdenticalTo(unboxedL, unboxedR)) {
            return {unboxedL, bothConst};
        }
    }

    return {unboxedR, bothConst};
}

checker::Type *ETSChecker::ApplyConditionalOperatorPromotion(checker::ETSChecker *checker, checker::Type *unboxedL,
                                                             checker::Type *unboxedR)
{
    if ((unboxedL->HasTypeFlag(checker::TypeFlag::CONSTANT) && unboxedL->IsIntType()) ||
        (unboxedR->HasTypeFlag(checker::TypeFlag::CONSTANT) && unboxedR->IsIntType())) {
        int value = unboxedL->IsIntType() ? unboxedL->AsIntType()->GetValue() : unboxedR->AsIntType()->GetValue();
        checker::Type *otherType = !unboxedL->IsIntType() ? unboxedL : unboxedR;

        switch (checker::ETSChecker::ETSType(otherType)) {
            case checker::TypeFlag::BYTE:
            case checker::TypeFlag::CHAR: {
                if (value <= static_cast<int>(std::numeric_limits<char>::max()) &&
                    value >= static_cast<int>(std::numeric_limits<char>::min())) {
                    return checker->GetNonConstantTypeFromPrimitiveType(otherType);
                }
                break;
            }
            case checker::TypeFlag::SHORT: {
                if (value <= std::numeric_limits<int16_t>::max() && value >= std::numeric_limits<int16_t>::min()) {
                    return checker->GlobalShortType();
                }
                break;
            }
            default: {
                return otherType;
            }
        }
        return checker->GlobalIntType();
    }

    if (unboxedL->IsDoubleType() || unboxedR->IsDoubleType()) {
        return checker->GlobalDoubleType();
    }
    if (unboxedL->IsFloatType() || unboxedR->IsFloatType()) {
        return checker->GlobalFloatType();
    }
    if (unboxedL->IsLongType() || unboxedR->IsLongType()) {
        return checker->GlobalLongType();
    }
    if (unboxedL->IsIntType() || unboxedR->IsIntType() || unboxedL->IsCharType() || unboxedR->IsCharType()) {
        return checker->GlobalIntType();
    }
    if (unboxedL->IsShortType() || unboxedR->IsShortType()) {
        return checker->GlobalShortType();
    }
    if (unboxedL->IsByteType() || unboxedR->IsByteType()) {
        return checker->GlobalByteType();
    }

    UNREACHABLE();
}

Type *ETSChecker::ApplyUnaryOperatorPromotion(Type *type, const bool createConst, const bool doPromotion,
                                              const bool isCondExpr)
{
    Type *unboxedType = isCondExpr ? ETSBuiltinTypeAsConditionalType(type) : ETSBuiltinTypeAsPrimitiveType(type);

    if (unboxedType == nullptr) {
        return nullptr;
    }
    if (doPromotion) {
        switch (ETSType(unboxedType)) {
            case TypeFlag::BYTE:
            case TypeFlag::SHORT:
            case TypeFlag::CHAR: {
                if (!createConst) {
                    return GlobalIntType();
                }

                return CreateIntTypeFromType(unboxedType);
            }
            default: {
                break;
            }
        }
    }
    return unboxedType;
}

bool ETSChecker::IsNullLikeOrVoidExpression(const ir::Expression *expr) const
{
    return expr->TsType()->DefinitelyETSNullish() || expr->TsType()->IsETSVoidType();
}

std::tuple<bool, bool> ETSChecker::IsResolvedAndValue(const ir::Expression *expr, Type *type) const
{
    auto [isResolve, isValue] =
        IsNullLikeOrVoidExpression(expr) ? std::make_tuple(true, false) : type->ResolveConditionExpr();

    const Type *tsType = expr->TsType();
    if (tsType->DefinitelyNotETSNullish() && !type->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
        isResolve = true;
        isValue = true;
    }
    return std::make_tuple(isResolve, isValue);
}

Type *ETSChecker::HandleBooleanLogicalOperatorsExtended(Type *leftType, Type *rightType, ir::BinaryExpression *expr)
{
    ASSERT(leftType->IsConditionalExprType() && rightType->IsConditionalExprType());

    auto [resolveLeft, leftValue] = IsResolvedAndValue(expr->Left(), leftType);
    auto [resolveRight, rightValue] = IsResolvedAndValue(expr->Right(), rightType);

    if (!resolveLeft && !resolveRight) {
        if (IsTypeIdenticalTo(leftType, rightType)) {
            return leftType;
        }
        return CreateETSUnionType({leftType, rightType});
    }

    switch (expr->OperatorType()) {
        case lexer::TokenType::PUNCTUATOR_LOGICAL_OR: {
            if (leftValue) {
                expr->SetResult(expr->Left());
                return leftType->IsETSBooleanType() ? CreateETSBooleanType(true) : leftType;
            }

            expr->SetResult(expr->Right());
            return rightType->IsETSBooleanType() && resolveRight ? CreateETSBooleanType(rightValue) : rightType;
        }
        case lexer::TokenType::PUNCTUATOR_LOGICAL_AND: {
            if (leftValue) {
                expr->SetResult(expr->Right());
                return rightType->IsETSBooleanType() && resolveRight ? CreateETSBooleanType(rightValue) : rightType;
            }

            expr->SetResult(expr->Left());
            return leftType->IsETSBooleanType() ? CreateETSBooleanType(false) : leftType;
        }
        default: {
            break;
        }
    }

    UNREACHABLE();
}

Type *ETSChecker::HandleBooleanLogicalOperators(Type *leftType, Type *rightType, lexer::TokenType tokenType)
{
    using UType = typename ETSBooleanType::UType;
    ASSERT(leftType->IsETSBooleanType() && rightType->IsETSBooleanType());

    if (!leftType->HasTypeFlag(checker::TypeFlag::CONSTANT) || !rightType->HasTypeFlag(checker::TypeFlag::CONSTANT)) {
        return GlobalETSBooleanType();
    }

    UType leftValue = leftType->AsETSBooleanType()->GetValue();
    UType rightValue = rightType->AsETSBooleanType()->GetValue();

    switch (tokenType) {
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR: {
            return CreateETSBooleanType(leftValue ^ rightValue);
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND: {
            return CreateETSBooleanType((static_cast<uint8_t>(leftValue) & static_cast<uint8_t>(rightValue)) != 0);
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR: {
            return CreateETSBooleanType((static_cast<uint8_t>(leftValue) | static_cast<uint8_t>(rightValue)) != 0);
        }
        case lexer::TokenType::PUNCTUATOR_LOGICAL_OR: {
            return CreateETSBooleanType(leftValue || rightValue);
        }
        case lexer::TokenType::PUNCTUATOR_LOGICAL_AND: {
            return CreateETSBooleanType(leftValue && rightValue);
        }
        default: {
            break;
        }
    }

    UNREACHABLE();
    return nullptr;
}

void ETSChecker::ResolveReturnStatement(checker::Type *funcReturnType, checker::Type *argumentType,
                                        ir::ScriptFunction *containingFunc, ir::ReturnStatement *st)
{
    if (funcReturnType->IsETSReferenceType() || argumentType->IsETSReferenceType()) {
        // function return type should be of reference (object) type
        Relation()->SetFlags(checker::TypeRelationFlag::NONE);

        if (!argumentType->IsETSReferenceType()) {
            argumentType = PrimitiveTypeAsETSBuiltinType(argumentType);
            if (argumentType == nullptr) {
                ThrowTypeError("Invalid return statement expression", st->Argument()->Start());
            }
            st->Argument()->AddBoxingUnboxingFlags(GetBoxingFlag(argumentType));
        }

        if (!funcReturnType->IsETSReferenceType()) {
            funcReturnType = PrimitiveTypeAsETSBuiltinType(funcReturnType);
            if (funcReturnType == nullptr) {
                ThrowTypeError("Invalid return function expression", st->Start());
            }
        }
        funcReturnType = CreateETSUnionType({funcReturnType, argumentType});
        containingFunc->Signature()->SetReturnType(funcReturnType);
        containingFunc->Signature()->AddSignatureFlag(checker::SignatureFlags::INFERRED_RETURN_TYPE);
    } else if (funcReturnType->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE_RETURN) &&
               argumentType->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE_RETURN)) {
        // function return type is of primitive type (including enums):
        Relation()->SetFlags(checker::TypeRelationFlag::DIRECT_RETURN |
                             checker::TypeRelationFlag::IN_ASSIGNMENT_CONTEXT |
                             checker::TypeRelationFlag::ASSIGNMENT_CONTEXT);
        if (Relation()->IsAssignableTo(funcReturnType, argumentType)) {
            funcReturnType = argumentType;
            containingFunc->Signature()->SetReturnType(funcReturnType);
            containingFunc->Signature()->AddSignatureFlag(checker::SignatureFlags::INFERRED_RETURN_TYPE);
        } else if (!Relation()->IsAssignableTo(argumentType, funcReturnType)) {
            const Type *targetType = TryGettingFunctionTypeFromInvokeFunction(funcReturnType);
            const Type *sourceType = TryGettingFunctionTypeFromInvokeFunction(argumentType);

            Relation()->RaiseError(
                {"Function cannot have different primitive return types, found '", targetType, "', '", sourceType, "'"},
                st->Argument()->Start());
        }
    } else {
        ThrowTypeError("Invalid return statement type(s).", st->Start());
    }
}

checker::Type *ETSChecker::CheckArrayElements(ir::Identifier *ident, ir::ArrayExpression *init)
{
    ArenaVector<ir::Expression *> elements = init->AsArrayExpression()->Elements();
    checker::Type *annotationType = nullptr;
    if (elements.empty()) {
        // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
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
        // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
        annotationType = Allocator()->New<ETSArrayType>(type);
    }
    return annotationType;
}

checker::Type *ETSChecker::FixOptionalVariableType(varbinder::Variable *const bindingVar, ir::ModifierFlags flags)
{
    if ((flags & ir::ModifierFlags::OPTIONAL) != 0) {
        auto type = bindingVar->TsType();
        if (type->IsETSUnionType()) {
            auto constituentTypes = type->AsETSUnionType()->ConstituentTypes();
            constituentTypes.push_back(GlobalETSUndefinedType());
            type = CreateETSUnionType(std::move(constituentTypes));
        } else {
            type = CreateETSUnionType({GlobalETSUndefinedType(), type});
        }
        bindingVar->SetTsType(type);
    }
    return bindingVar->TsType();
}

checker::Type *ETSChecker::CheckVariableDeclaration(ir::Identifier *ident, ir::TypeNode *typeAnnotation,
                                                    ir::Expression *init, ir::ModifierFlags flags)
{
    const util::StringView &varName = ident->Name();
    ASSERT(ident->Variable());
    varbinder::Variable *const bindingVar = ident->Variable();
    checker::Type *annotationType = nullptr;

    const bool isConst = (flags & ir::ModifierFlags::CONST) != 0;

    if (typeAnnotation != nullptr) {
        annotationType = typeAnnotation->GetType(this);
        bindingVar->SetTsType(annotationType);
    }

    if (init == nullptr) {
        return FixOptionalVariableType(bindingVar, flags);
    }

    if (typeAnnotation == nullptr) {
        if (init->IsArrayExpression()) {
            annotationType = CheckArrayElements(ident, init->AsArrayExpression());
            bindingVar->SetTsType(annotationType);
        }

        if (init->IsObjectExpression()) {
            ThrowTypeError(
                {"Cannot infer type for ", ident->Name(), " because class composite needs an explicit target type"},
                ident->Start());
        }
    }

    if (init->IsMemberExpression() && init->AsMemberExpression()->Object()->IsObjectExpression()) {
        ThrowTypeError({"Class composite must be constructed separately before referring their members."},
                       ident->Start());
    }

    if ((init->IsMemberExpression()) && (annotationType != nullptr)) {
        SetArrayPreferredTypeForNestedMemberExpressions(init->AsMemberExpression(), annotationType);
    }

    if (init->IsArrayExpression() && annotationType->IsETSArrayType()) {
        if (annotationType->IsETSTupleType()) {
            ValidateTupleMinElementSize(init->AsArrayExpression(), annotationType->AsETSTupleType());
        }

        init->AsArrayExpression()->SetPreferredType(annotationType);
    }

    if (init->IsObjectExpression()) {
        init->AsObjectExpression()->SetPreferredType(annotationType);
    }

    if (typeAnnotation != nullptr && typeAnnotation->IsETSFunctionType() && init->IsArrowFunctionExpression()) {
        auto *const arrowFuncExpr = init->AsArrowFunctionExpression();
        ir::ScriptFunction *const lambda = arrowFuncExpr->Function();
        if (lambda->Params().size() == typeAnnotation->AsETSFunctionType()->Params().size() &&
            NeedTypeInference(lambda)) {
            InferTypesForLambda(lambda, typeAnnotation->AsETSFunctionType());
        }
    }
    checker::Type *initType = init->Check(this);

    if (initType == nullptr) {
        ThrowTypeError("Cannot get the expression type", init->Start());
    }

    if (typeAnnotation == nullptr &&
        (init->IsArrowFunctionExpression() ||
         (init->IsTSAsExpression() && init->AsTSAsExpression()->Expr()->IsArrowFunctionExpression()))) {
        if (init->IsArrowFunctionExpression()) {
            // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
            typeAnnotation = init->AsArrowFunctionExpression()->CreateTypeAnnotation(this);
        } else {
            typeAnnotation = init->AsTSAsExpression()->TypeAnnotation()->Clone(Allocator(), nullptr);
        }
        ident->SetTsTypeAnnotation(typeAnnotation);
        typeAnnotation->SetParent(ident);
        annotationType = typeAnnotation->GetType(this);
        bindingVar->SetTsType(annotationType);
    }

    if (annotationType != nullptr) {
        const Type *targetType = TryGettingFunctionTypeFromInvokeFunction(annotationType);
        const Type *sourceType = TryGettingFunctionTypeFromInvokeFunction(initType);

        AssignmentContext(Relation(), init, initType, annotationType, init->Start(),
                          {"Type '", sourceType, "' cannot be assigned to type '", targetType, "'"});
        if (isConst && initType->HasTypeFlag(TypeFlag::ETS_PRIMITIVE) &&
            annotationType->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
            bindingVar->SetTsType(init->TsType());
        }
        return FixOptionalVariableType(bindingVar, flags);
    }

    if (initType->IsETSObjectType() && initType->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::ENUM) &&
        !init->IsMemberExpression()) {
        ThrowTypeError({"Cannot assign type '", initType->AsETSObjectType()->Name(), "' for variable ", varName, "."},
                       init->Start());
    }

    isConst ? bindingVar->SetTsType(initType) : bindingVar->SetTsType(GetNonConstantTypeFromPrimitiveType(initType));

    return FixOptionalVariableType(bindingVar, flags);
}

void ETSChecker::SetArrayPreferredTypeForNestedMemberExpressions(ir::MemberExpression *expr, Type *annotationType)
{
    if ((expr == nullptr) || (annotationType == nullptr)) {
        return;
    }

    if (expr->Kind() != ir::MemberExpressionKind::ELEMENT_ACCESS) {
        return;
    }

    // Expand all member expressions
    Type *elementType = annotationType;
    ir::Expression *object = expr->Object();
    while ((object != nullptr) && (object->IsMemberExpression())) {
        ir::MemberExpression *memberExpr = object->AsMemberExpression();
        if (memberExpr->Kind() != ir::MemberExpressionKind::ELEMENT_ACCESS) {
            return;
        }

        object = memberExpr->Object();
        elementType = CreateETSArrayType(elementType);
    }

    // Set explicit target type for array
    if ((object != nullptr) && (object->IsArrayExpression())) {
        ir::ArrayExpression *array = object->AsArrayExpression();
        array->SetPreferredType(CreateETSArrayType(elementType));
    }
}

Type *ETSChecker::HandleTypeAlias(ir::Expression *const name, const ir::TSTypeParameterInstantiation *const typeParams)
{
    ASSERT(name->IsIdentifier() && name->AsIdentifier()->Variable() &&
           name->AsIdentifier()->Variable()->Declaration()->IsTypeAliasDecl());

    auto *const typeAliasNode =
        name->AsIdentifier()->Variable()->Declaration()->AsTypeAliasDecl()->Node()->AsTSTypeAliasDeclaration();

    // NOTE (mmartin): modify for default params
    if ((typeParams == nullptr) != (typeAliasNode->TypeParams() == nullptr)) {
        if (typeParams == nullptr) {
            ThrowTypeError("Type alias declaration is generic, but no type parameters were provided", name->Start());
        }

        ThrowTypeError("Type alias declaration is not generic, but type parameters were provided", typeParams->Start());
    }

    if (typeParams == nullptr) {
        // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
        return GetReferencedTypeBase(name);
    }

    for (auto *const origTypeParam : typeParams->Params()) {
        origTypeParam->Check(this);
    }

    // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
    Type *const aliasType = GetReferencedTypeBase(name);
    auto *const aliasSub = NewSubstitution();

    if (typeAliasNode->TypeParams()->Params().size() != typeParams->Params().size()) {
        ThrowTypeError("Wrong number of type parameters for generic type alias", typeParams->Start());
    }

    for (std::size_t idx = 0; idx < typeAliasNode->TypeParams()->Params().size(); ++idx) {
        auto *typeAliasType = typeAliasNode->TypeParams()->Params().at(idx)->Name()->Variable()->TsType();
        if (typeAliasType->IsETSTypeParameter()) {
            aliasSub->insert({typeAliasType->AsETSTypeParameter(), typeParams->Params().at(idx)->TsType()});
        }
    }

    // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
    ValidateGenericTypeAliasForClonedNode(typeAliasNode->AsTSTypeAliasDeclaration(), typeParams);

    return aliasType->Substitute(Relation(), aliasSub);
}

std::vector<util::StringView> ETSChecker::GetNameForSynteticObjectType(const util::StringView &source)
{
    const std::string str = source.Mutf8();
    std::istringstream ss {str};
    const char delimiter = '.';
    std::string token;

    std::vector<util::StringView> syntheticName {};

    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            util::UString sV(token, Allocator());
            syntheticName.emplace_back(sV.View());
        }
    }

    return syntheticName;
}

void ETSChecker::SetPropertiesForModuleObject(checker::ETSObjectType *moduleObjType, const util::StringView &importPath)
{
    auto *etsBinder = static_cast<varbinder::ETSBinder *>(VarBinder());

    auto extRecords = etsBinder->GetGlobalRecordTable()->Program()->ExternalSources();
    auto [name, isPackageModule] = etsBinder->GetModuleInfo(importPath);
    auto res = extRecords.find(name);
    ASSERT(res != extRecords.end());

    // Check imported properties before assigning them to module object
    res->second.front()->Ast()->Check(this);

    for (auto [_, var] : res->second.front()->GlobalClassScope()->StaticFieldScope()->Bindings()) {
        (void)_;
        if (var->AsLocalVariable()->Declaration()->Node()->IsExported()) {
            moduleObjType->AddProperty<checker::PropertyType::STATIC_FIELD>(var->AsLocalVariable());
        }
    }

    for (auto [_, var] : res->second.front()->GlobalClassScope()->StaticMethodScope()->Bindings()) {
        (void)_;
        if (var->AsLocalVariable()->Declaration()->Node()->IsExported()) {
            moduleObjType->AddProperty<checker::PropertyType::STATIC_METHOD>(var->AsLocalVariable());
        }
    }

    for (auto [_, var] : res->second.front()->GlobalClassScope()->InstanceDeclScope()->Bindings()) {
        (void)_;
        if (var->AsLocalVariable()->Declaration()->Node()->IsExported()) {
            moduleObjType->AddProperty<checker::PropertyType::STATIC_DECL>(var->AsLocalVariable());
        }
    }

    for (auto [_, var] : res->second.front()->GlobalClassScope()->TypeAliasScope()->Bindings()) {
        (void)_;
        if (var->AsLocalVariable()->Declaration()->Node()->IsExported()) {
            moduleObjType->AddProperty<checker::PropertyType::STATIC_DECL>(var->AsLocalVariable());
        }
    }
}

void ETSChecker::SetrModuleObjectTsType(ir::Identifier *local, checker::ETSObjectType *moduleObjType)
{
    auto *etsBinder = static_cast<varbinder::ETSBinder *>(VarBinder());

    for (auto [bindingName, var] : etsBinder->TopScope()->Bindings()) {
        if (bindingName.Is(local->Name().Mutf8())) {
            var->SetTsType(moduleObjType);
        }
    }
}

Type *ETSChecker::GetReferencedTypeFromBase([[maybe_unused]] Type *baseType, [[maybe_unused]] ir::Expression *name)
{
    return nullptr;
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
            // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
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

void ETSChecker::ConcatConstantString(util::UString &target, Type *type)
{
    switch (ETSType(type)) {
        case TypeFlag::ETS_OBJECT: {
            ASSERT(type->IsETSStringType());
            target.Append(type->AsETSStringType()->GetValue());
            break;
        }
        case TypeFlag::ETS_BOOLEAN: {
            ETSBooleanType::UType value = type->AsETSBooleanType()->GetValue();
            target.Append(value ? "true" : "false");
            break;
        }
        case TypeFlag::BYTE: {
            ByteType::UType value = type->AsByteType()->GetValue();
            target.Append(std::to_string(value));
            break;
        }
        case TypeFlag::CHAR: {
            CharType::UType value = type->AsCharType()->GetValue();
            std::string s(1, value);
            target.Append(s);
            break;
        }
        case TypeFlag::SHORT: {
            ShortType::UType value = type->AsShortType()->GetValue();
            target.Append(std::to_string(value));
            break;
        }
        case TypeFlag::INT: {
            IntType::UType value = type->AsIntType()->GetValue();
            target.Append(std::to_string(value));
            break;
        }
        case TypeFlag::LONG: {
            LongType::UType value = type->AsLongType()->GetValue();
            target.Append(std::to_string(value));
            break;
        }
        case TypeFlag::FLOAT: {
            FloatType::UType value = type->AsFloatType()->GetValue();
            target.Append(std::to_string(value));
            break;
        }
        case TypeFlag::DOUBLE: {
            DoubleType::UType value = type->AsDoubleType()->GetValue();
            target.Append(std::to_string(value));
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
}

Type *ETSChecker::HandleStringConcatenation(Type *leftType, Type *rightType)
{
    ASSERT(leftType->IsETSStringType() || rightType->IsETSStringType());

    if (!leftType->HasTypeFlag(checker::TypeFlag::CONSTANT) || !rightType->HasTypeFlag(checker::TypeFlag::CONSTANT)) {
        return GlobalETSStringLiteralType();
    }

    util::UString concatenated(Allocator());
    ConcatConstantString(concatenated, leftType);
    ConcatConstantString(concatenated, rightType);

    return CreateETSStringLiteralType(concatenated.View());
}

ETSFunctionType *ETSChecker::FindFunctionInVectorGivenByName(util::StringView name,
                                                             ArenaVector<ETSFunctionType *> &list)
{
    for (auto *it : list) {
        if (it->Name() == name) {
            return it;
        }
    }

    return nullptr;
}

bool ETSChecker::IsFunctionContainsSignature(ETSFunctionType *funcType, Signature *signature)
{
    for (auto *it : funcType->CallSignatures()) {
        Relation()->IsCompatibleTo(it, signature);
        if (Relation()->IsTrue()) {
            return true;
        }
    }

    return false;
}

void ETSChecker::CheckFunctionContainsClashingSignature(const ETSFunctionType *funcType, Signature *signature)
{
    for (auto *it : funcType->CallSignatures()) {
        SavedTypeRelationFlagsContext strfCtx(Relation(), TypeRelationFlag::NONE);
        Relation()->IsCompatibleTo(it, signature);
        if (Relation()->IsTrue() && it->Function()->Id()->Name() == signature->Function()->Id()->Name()) {
            std::stringstream ss;
            it->ToString(ss, nullptr, true);
            auto sigStr1 = ss.str();
            ss.str(std::string {});  // Clear buffer
            signature->ToString(ss, nullptr, true);
            auto sigStr2 = ss.str();
            ThrowTypeError({"Function '", it->Function()->Id()->Name(), sigStr1.c_str(),
                            "' is redeclared with different signature '", signature->Function()->Id()->Name(),
                            sigStr2.c_str(), "'"},
                           signature->Function()->ReturnTypeAnnotation()->Start());
        }
    }
}

void ETSChecker::MergeSignatures(ETSFunctionType *target, ETSFunctionType *source)
{
    for (auto *s : source->CallSignatures()) {
        if (IsFunctionContainsSignature(target, s)) {
            continue;
        }

        CheckFunctionContainsClashingSignature(target, s);
        target->AddCallSignature(s);
    }
}

void ETSChecker::MergeComputedAbstracts(ArenaVector<ETSFunctionType *> &merged, ArenaVector<ETSFunctionType *> &current)
{
    for (auto *curr : current) {
        auto name = curr->Name();
        auto *found = FindFunctionInVectorGivenByName(name, merged);
        if (found != nullptr) {
            MergeSignatures(found, curr);
            continue;
        }

        merged.push_back(curr);
    }
}

ir::AstNode *ETSChecker::FindAncestorGivenByType(ir::AstNode *node, ir::AstNodeType type, const ir::AstNode *endNode)
{
    auto *iter = node->Parent();

    while (iter != endNode) {
        if (iter->Type() == type) {
            return iter;
        }

        iter = iter->Parent();
    }

    return nullptr;
}

util::StringView ETSChecker::GetContainingObjectNameFromSignature(Signature *signature)
{
    ASSERT(signature->Function());
    auto *iter = signature->Function()->Parent();

    while (iter != nullptr) {
        if (iter->IsClassDefinition()) {
            return iter->AsClassDefinition()->Ident()->Name();
        }

        if (iter->IsTSInterfaceDeclaration()) {
            return iter->AsTSInterfaceDeclaration()->Id()->Name();
        }

        iter = iter->Parent();
    }

    UNREACHABLE();
    return {""};
}

const ir::AstNode *ETSChecker::FindJumpTarget(ir::AstNodeType nodeType, const ir::AstNode *node,
                                              const ir::Identifier *target)
{
    const auto *iter = node->Parent();

    while (iter != nullptr) {
        switch (iter->Type()) {
            case ir::AstNodeType::LABELLED_STATEMENT: {
                if (const auto *labelled = iter->AsLabelledStatement(); labelled->Ident()->Name() == target->Name()) {
                    return nodeType == ir::AstNodeType::CONTINUE_STATEMENT ? labelled->GetReferencedStatement()
                                                                           : labelled;
                }
                break;
            }
            case ir::AstNodeType::DO_WHILE_STATEMENT:
            case ir::AstNodeType::WHILE_STATEMENT:
            case ir::AstNodeType::FOR_UPDATE_STATEMENT:
            case ir::AstNodeType::FOR_OF_STATEMENT:
            case ir::AstNodeType::SWITCH_STATEMENT: {
                if (target == nullptr) {
                    return iter;
                }
                break;
            }
            default: {
                break;
            }
        }

        iter = iter->Parent();
    }

    UNREACHABLE();
    return nullptr;
}

varbinder::VariableFlags ETSChecker::GetAccessFlagFromNode(const ir::AstNode *node)
{
    if (node->IsPrivate()) {
        return varbinder::VariableFlags::PRIVATE;
    }

    if (node->IsProtected()) {
        return varbinder::VariableFlags::PROTECTED;
    }

    return varbinder::VariableFlags::PUBLIC;
}

void ETSChecker::CheckSwitchDiscriminant(ir::Expression *discriminant)
{
    ASSERT(discriminant->TsType());

    auto discriminantType = discriminant->TsType();
    if (discriminantType->HasTypeFlag(TypeFlag::VALID_SWITCH_TYPE)) {
        return;
    }

    if (discriminantType->IsETSObjectType() &&
        discriminantType->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::VALID_SWITCH_TYPE)) {
        if (discriminantType->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::UNBOXABLE_TYPE)) {
            discriminant->SetBoxingUnboxingFlags(GetUnboxingFlag(ETSBuiltinTypeAsPrimitiveType(discriminantType)));
        }
        return;
    }

    ThrowTypeError({"Incompatible types. Found: ", discriminantType,
                    ", required: char , byte , short , int, long , Char , Byte , Short , Int, Long , String "
                    "or an enum type"},
                   discriminant->Start());
}

void ETSChecker::AddBoxingUnboxingFlagsToNode(ir::AstNode *node, Type *boxingUnboxingType)
{
    if (boxingUnboxingType->IsETSObjectType()) {
        node->AddBoxingUnboxingFlags(GetBoxingFlag(boxingUnboxingType));
    } else {
        node->AddBoxingUnboxingFlags(GetUnboxingFlag(boxingUnboxingType));
    }
}

Type *ETSChecker::MaybeBoxExpression(ir::Expression *expr)
{
    auto *promoted = MaybePromotedBuiltinType(expr->TsType());
    if (promoted != expr->TsType()) {
        expr->AddBoxingUnboxingFlags(GetBoxingFlag(promoted));
    }
    return promoted;
}

util::StringView ETSChecker::TypeToName(Type *type) const
{
    auto typeKind = TypeKind(type);
    switch (typeKind) {
        case TypeFlag::ETS_BOOLEAN: {
            return "boolean";
        }
        case TypeFlag::BYTE: {
            return "byte";
        }
        case TypeFlag::CHAR: {
            return "char";
        }
        case TypeFlag::SHORT: {
            return "short";
        }
        case TypeFlag::INT: {
            return "int";
        }
        case TypeFlag::LONG: {
            return "long";
        }
        case TypeFlag::FLOAT: {
            return "float";
        }
        case TypeFlag::DOUBLE: {
            return "number";
        }
        default:
            UNREACHABLE();
    }
}

void ETSChecker::CheckForSameSwitchCases(ArenaVector<ir::SwitchCaseStatement *> *cases)
{
    //  Just to avoid extra nesting level
    auto const checkEnumType = [this](ir::Expression const *const caseTest, ETSEnumType const *const type) -> void {
        if (caseTest->TsType()->AsETSEnumType()->IsSameEnumLiteralType(type)) {
            ThrowTypeError("Case duplicate", caseTest->Start());
        }
    };

    for (size_t caseNum = 0; caseNum < cases->size(); caseNum++) {
        for (size_t compareCase = caseNum + 1; compareCase < cases->size(); compareCase++) {
            auto *caseTest = cases->at(caseNum)->Test();
            auto *compareCaseTest = cases->at(compareCase)->Test();

            if (caseTest == nullptr || compareCaseTest == nullptr) {
                continue;
            }

            if (caseTest->TsType()->IsETSEnumType()) {
                checkEnumType(caseTest, compareCaseTest->TsType()->AsETSEnumType());
                continue;
            }

            if (caseTest->IsIdentifier() || caseTest->IsMemberExpression()) {
                CheckIdentifierSwitchCase(caseTest, compareCaseTest, cases->at(caseNum)->Start());
                continue;
            }

            if (compareCaseTest->IsIdentifier() || compareCaseTest->IsMemberExpression()) {
                CheckIdentifierSwitchCase(compareCaseTest, caseTest, cases->at(compareCase)->Start());
                continue;
            }

            if (GetStringFromLiteral(caseTest) != GetStringFromLiteral(compareCaseTest)) {
                continue;
            }

            ThrowTypeError("Case duplicate", cases->at(compareCase)->Start());
        }
    }
}

std::string ETSChecker::GetStringFromIdentifierValue(checker::Type *caseType) const
{
    const auto identifierTypeKind = ETSChecker::TypeKind(caseType);
    switch (identifierTypeKind) {
        case TypeFlag::BYTE: {
            return std::to_string(caseType->AsByteType()->GetValue());
        }
        case TypeFlag::SHORT: {
            return std::to_string(caseType->AsShortType()->GetValue());
        }
        case TypeFlag::CHAR: {
            return std::to_string(caseType->AsCharType()->GetValue());
        }
        case TypeFlag::INT: {
            return std::to_string(caseType->AsIntType()->GetValue());
        }
        case TypeFlag::LONG: {
            return std::to_string(caseType->AsLongType()->GetValue());
        }
        case TypeFlag::ETS_OBJECT: {
            VarBinder()->ThrowError(caseType->AsETSObjectType()->Variable()->Declaration()->Node()->Start(),
                                    "not implemented");
        }
        default: {
            UNREACHABLE();
        }
    }
}

bool IsConstantMemberOrIdentifierExpression(ir::Expression *expression)
{
    if (expression->IsMemberExpression()) {
        return expression->AsMemberExpression()->PropVar()->Declaration()->IsConstDecl();
    }

    if (expression->IsIdentifier()) {
        return expression->AsIdentifier()->Variable()->Declaration()->IsConstDecl();
    }

    return false;
}

bool ETSChecker::CompareIdentifiersValuesAreDifferent(ir::Expression *compareValue, const std::string &caseValue)
{
    if (IsConstantMemberOrIdentifierExpression(compareValue)) {
        checker::Type *compareCaseType = compareValue->TsType();

        const auto compareCaseValue = GetStringFromIdentifierValue(compareCaseType);
        return caseValue != compareCaseValue;
    }

    return caseValue != GetStringFromLiteral(compareValue);
}

void ETSChecker::CheckIdentifierSwitchCase(ir::Expression *currentCase, ir::Expression *compareCase,
                                           const lexer::SourcePosition &pos)
{
    currentCase->Check(this);

    if (!IsConstantMemberOrIdentifierExpression(currentCase)) {
        ThrowTypeError("Constant expression required", pos);
    }

    checker::Type *caseType = currentCase->TsType();

    if (!CompareIdentifiersValuesAreDifferent(compareCase, GetStringFromIdentifierValue(caseType))) {
        ThrowTypeError("Variable has same value with another switch case", pos);
    }
}

std::string ETSChecker::GetStringFromLiteral(ir::Expression *caseTest) const
{
    switch (caseTest->Type()) {
        case ir::AstNodeType::CHAR_LITERAL: {
            return std::to_string(caseTest->AsCharLiteral()->Char());
        }
        case ir::AstNodeType::STRING_LITERAL:
        case ir::AstNodeType::NUMBER_LITERAL: {
            return util::Helpers::LiteralToPropName(caseTest).Mutf8();
        }
        default:
            UNREACHABLE();
    }
}

bool ETSChecker::IsSameDeclarationType(varbinder::LocalVariable *target, varbinder::LocalVariable *compare)
{
    return target->Declaration()->Type() == compare->Declaration()->Type();
}

bool ETSChecker::CheckRethrowingParams(const ir::AstNode *ancestorFunction, const ir::AstNode *node)
{
    for (const auto param : ancestorFunction->AsScriptFunction()->Signature()->Function()->Params()) {
        if (node->AsCallExpression()->Callee()->AsIdentifier()->Name().Is(
                param->AsETSParameterExpression()->Ident()->Name().Mutf8())) {
            return true;
        }
    }
    return false;
}

void ETSChecker::CheckThrowingStatements(ir::AstNode *node)
{
    ir::AstNode *ancestorFunction = FindAncestorGivenByType(node, ir::AstNodeType::SCRIPT_FUNCTION);

    if (ancestorFunction == nullptr) {
        ThrowTypeError(
            "This statement can cause an exception, therefore it must be enclosed in a try statement with a default "
            "catch clause",
            node->Start());
    }

    if (ancestorFunction->AsScriptFunction()->IsThrowing() ||
        (ancestorFunction->AsScriptFunction()->IsRethrowing() &&
         (!node->IsThrowStatement() && CheckRethrowingParams(ancestorFunction, node)))) {
        return;
    }

    if (!CheckThrowingPlacement(node, ancestorFunction)) {
        if (ancestorFunction->AsScriptFunction()->IsRethrowing() && !node->IsThrowStatement()) {
            ThrowTypeError(
                "This statement can cause an exception, re-throwing functions can throw exception only by their "
                "parameters.",
                node->Start());
        }

        ThrowTypeError(
            "This statement can cause an exception, therefore it must be enclosed in a try statement with a default "
            "catch clause",
            node->Start());
    }
}

bool ETSChecker::CheckThrowingPlacement(ir::AstNode *node, const ir::AstNode *ancestorFunction)
{
    ir::AstNode *startPoint = node;
    ir::AstNode *enclosingCatchClause = nullptr;
    ir::BlockStatement *enclosingFinallyBlock = nullptr;
    ir::AstNode *p = startPoint->Parent();

    bool isHandled = false;
    const auto predicateFunc = [&enclosingCatchClause](ir::CatchClause *clause) {
        return clause == enclosingCatchClause;
    };

    do {
        if (p->IsTryStatement() && p->AsTryStatement()->HasDefaultCatchClause()) {
            enclosingCatchClause = FindAncestorGivenByType(startPoint, ir::AstNodeType::CATCH_CLAUSE, p);
            enclosingFinallyBlock = FindFinalizerOfTryStatement(startPoint, p);
            const auto catches = p->AsTryStatement()->CatchClauses();

            if (std::any_of(catches.begin(), catches.end(), predicateFunc)) {
                startPoint = enclosingCatchClause;
            } else if (enclosingFinallyBlock != nullptr &&
                       enclosingFinallyBlock == p->AsTryStatement()->FinallyBlock()) {
                startPoint = enclosingFinallyBlock;
            } else {
                isHandled = true;
                break;
            }
        }

        p = p->Parent();
    } while (p != ancestorFunction);

    return isHandled;
}

ir::BlockStatement *ETSChecker::FindFinalizerOfTryStatement(ir::AstNode *startFrom, const ir::AstNode *p)
{
    auto *iter = startFrom->Parent();

    do {
        if (iter->IsBlockStatement()) {
            ir::BlockStatement *finallyBlock = iter->AsBlockStatement();

            if (finallyBlock == p->AsTryStatement()->FinallyBlock()) {
                return finallyBlock;
            }
        }

        iter = iter->Parent();
    } while (iter != p);

    return nullptr;
}

void ETSChecker::CheckRethrowingFunction(ir::ScriptFunction *func)
{
    bool foundThrowingParam = false;

    // It doesn't support lambdas yet.
    for (auto item : func->Params()) {
        auto const *type = item->AsETSParameterExpression()->Ident()->TypeAnnotation();

        if (type->IsETSTypeReference()) {
            auto *typeDecl = type->AsETSTypeReference()->Part()->Name()->AsIdentifier()->Variable()->Declaration();
            if (typeDecl->IsTypeAliasDecl()) {
                type = typeDecl->Node()->AsTSTypeAliasDeclaration()->TypeAnnotation();
            }
        }

        if (type->IsETSFunctionType() && type->AsETSFunctionType()->IsThrowing()) {
            foundThrowingParam = true;
            break;
        }
    }

    if (!foundThrowingParam) {
        ThrowTypeError("A rethrowing function must have a throwing function parameter", func->Start());
    }
}

ETSObjectType *ETSChecker::GetRelevantArgumentedTypeFromChild(ETSObjectType *const child, ETSObjectType *const target)
{
    if (child->GetDeclNode() == target->GetDeclNode()) {
        auto *relevantType = CreateNewETSObjectType(child->Name(), child->GetDeclNode(), child->ObjectFlags());

        ArenaVector<Type *> params = child->TypeArguments();

        relevantType->SetTypeArguments(std::move(params));
        relevantType->SetEnclosingType(child->EnclosingType());
        relevantType->SetSuperType(child->SuperType());

        return relevantType;
    }

    ASSERT(child->SuperType() != nullptr);

    return GetRelevantArgumentedTypeFromChild(child->SuperType(), target);
}

void ETSChecker::EmplaceSubstituted(Substitution *substitution, ETSTypeParameter *tparam, Type *typeArg)
{
    substitution->emplace(tparam, typeArg);
}

util::StringView ETSChecker::GetHashFromTypeArguments(const ArenaVector<Type *> &typeArgTypes)
{
    std::stringstream ss;

    for (auto *it : typeArgTypes) {
        it->ToString(ss, true);
        ss << compiler::Signatures::MANGLE_SEPARATOR;
    }

    return util::UString(ss.str(), Allocator()).View();
}

util::StringView ETSChecker::GetHashFromSubstitution(const Substitution *substitution)
{
    std::vector<std::string> fields;
    for (auto [k, v] : *substitution) {
        std::stringstream ss;
        k->ToString(ss, true);
        ss << ":";
        v->ToString(ss, true);
        fields.push_back(ss.str());
    }
    std::sort(fields.begin(), fields.end());

    std::stringstream ss;
    for (auto &fstr : fields) {
        ss << fstr;
        ss << ";";
    }
    return util::UString(ss.str(), Allocator()).View();
}

util::StringView ETSChecker::GetHashFromFunctionType(ir::ETSFunctionType *type)
{
    std::stringstream ss;
    for (auto *p : type->Params()) {
        auto *const param = p->AsETSParameterExpression();
        param->TypeAnnotation()->GetType(this)->ToString(ss, true);
        ss << ";";
    }

    type->ReturnType()->GetType(this)->ToString(ss, true);
    ss << ";";

    if (type->IsThrowing()) {
        ss << "throws;";
    }

    if (type->IsRethrowing()) {
        ss << "rethrows;";
    }

    return util::UString(ss.str(), Allocator()).View();
}

ETSObjectType *ETSChecker::GetOriginalBaseType(Type *const object)
{
    if (object == nullptr || !object->IsETSObjectType()) {
        return nullptr;
    }

    return object->AsETSObjectType()->GetOriginalBaseType();
}

void ETSChecker::CheckValidGenericTypeParameter(Type *const argType, const lexer::SourcePosition &pos)
{
    if (!argType->IsETSEnumType() && !argType->IsETSStringEnumType()) {
        return;
    }
    std::stringstream ss;
    argType->ToString(ss);
    ThrowTypeError("Type '" + ss.str() + "' is not valid for generic type arguments", pos);
}

void ETSChecker::CheckNumberOfTypeArguments(ETSObjectType *const type, ir::TSTypeParameterInstantiation *const typeArgs,
                                            const lexer::SourcePosition &pos)
{
    auto const &typeParams = type->TypeArguments();
    if (typeParams.empty()) {
        if (typeArgs != nullptr) {
            ThrowTypeError({"Type '", type, "' is not generic."}, pos);
        }
        return;
    }

    if (typeArgs == nullptr) {
        return;
    }

    size_t minimumTypeArgs = std::count_if(typeParams.begin(), typeParams.end(), [](Type *param) {
        return param->AsETSTypeParameter()->GetDefaultType() == nullptr;
    });
    if (typeArgs == nullptr && minimumTypeArgs > 0) {
        ThrowTypeError({"Type '", type, "' is generic but type argument were not provided."}, pos);
    }

    if (typeArgs != nullptr &&
        ((minimumTypeArgs > typeArgs->Params().size()) || (typeParams.size() < typeArgs->Params().size()))) {
        ThrowTypeError({"Type '", type, "' has ", minimumTypeArgs, " number of type parameters, but ",
                        typeArgs->Params().size(), " type arguments were provided."},
                       pos);
    }
}

bool ETSChecker::NeedTypeInference(const ir::ScriptFunction *lambda)
{
    if (lambda->ReturnTypeAnnotation() == nullptr) {
        return true;
    }
    for (auto *const param : lambda->Params()) {
        const auto *const lambdaParam = param->AsETSParameterExpression()->Ident();
        if (lambdaParam->TypeAnnotation() == nullptr) {
            return true;
        }
    }
    return false;
}

std::vector<bool> ETSChecker::FindTypeInferenceArguments(const ArenaVector<ir::Expression *> &arguments)
{
    std::vector<bool> argTypeInferenceRequired(arguments.size());
    size_t index = 0;
    for (ir::Expression *arg : arguments) {
        if (arg->IsArrowFunctionExpression()) {
            ir::ScriptFunction *const lambda = arg->AsArrowFunctionExpression()->Function();
            if (NeedTypeInference(lambda)) {
                argTypeInferenceRequired[index] = true;
            }
        }
        ++index;
    }
    return argTypeInferenceRequired;
}

bool ETSChecker::CheckLambdaAssignableUnion(ir::AstNode *typeAnn, ir::ScriptFunction *lambda)
{
    for (auto *type : typeAnn->AsETSUnionType()->Types()) {
        if (type->IsETSFunctionType()) {
            return lambda->Params().size() == type->AsETSFunctionType()->Params().size();
        }
    }

    return false;
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

void ETSChecker::ModifyPreferredType(ir::ArrayExpression *const arrayExpr, Type *const newPreferredType)
{
    // After modifying the preferred type of an array expression, it needs to be rechecked at the call site
    arrayExpr->SetPreferredType(newPreferredType);
    arrayExpr->SetTsType(nullptr);

    for (auto *const element : arrayExpr->Elements()) {
        if (element->IsArrayExpression()) {
            ModifyPreferredType(element->AsArrayExpression(), nullptr);
        }
    }
}

bool ETSChecker::IsInLocalClass(const ir::AstNode *node) const
{
    while (node != nullptr) {
        if (node->Type() == ir::AstNodeType::CLASS_DEFINITION) {
            return node->AsClassDefinition()->IsLocal();
        }
        node = node->Parent();
    }

    return false;
}

ir::Expression *ETSChecker::GenerateImplicitInstantiateArg(varbinder::LocalVariable *instantiateMethod,
                                                           const std::string &className)
{
    auto callSignatures = instantiateMethod->TsType()->AsETSFunctionType()->CallSignatures();
    ASSERT(!callSignatures.empty());
    auto methodOwner = std::string(callSignatures[0]->Owner()->Name());
    std::string implicitInstantiateArgument = "()=>{return new " + className + "()";
    if (methodOwner != className) {
        implicitInstantiateArgument.append(" as " + methodOwner);
    }
    implicitInstantiateArgument.append("}");

    parser::Program program(Allocator(), VarBinder());
    es2panda::CompilerOptions options;
    auto parser = parser::ETSParser(&program, options, parser::ParserStatus::NO_OPTS);
    auto *argExpr = parser.CreateExpression(implicitInstantiateArgument);
    compiler::InitScopesPhaseETS::RunExternalNode(argExpr, &program);

    return argExpr;
}

void ETSChecker::GenerateGetterSetterBody(ArenaVector<ir::Statement *> &stmts, ArenaVector<ir::Expression *> &params,
                                          ir::ClassProperty *const field, varbinder::FunctionParamScope *paramScope,
                                          bool isSetter)
{
    if (!isSetter) {
        auto *clone = field->Key()->Clone(Allocator(), nullptr)->AsExpression();
        stmts.push_back(AllocNode<ir::ReturnStatement>(clone));
        return;
    }

    auto *paramIdent = field->Key()->AsIdentifier()->Clone(Allocator(), nullptr);
    auto *const typeAnnotation = field->TypeAnnotation()->Clone(Allocator(), paramIdent);
    paramIdent->SetTsTypeAnnotation(typeAnnotation);

    auto *paramExpression = AllocNode<ir::ETSParameterExpression>(paramIdent, nullptr);
    paramExpression->SetRange(paramIdent->Range());
    auto *const paramVar = std::get<2>(paramScope->AddParamDecl(Allocator(), paramExpression));
    paramExpression->SetVariable(paramVar);

    params.push_back(paramExpression);

    auto *assignmentExpression = AllocNode<ir::AssignmentExpression>(
        field->Key()->Clone(Allocator(), nullptr)->AsExpression(), paramExpression->Clone(Allocator(), nullptr),
        lexer::TokenType::PUNCTUATOR_SUBSTITUTION);

    assignmentExpression->SetRange({field->Start(), field->End()});
    // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
    stmts.push_back(AllocNode<ir::ExpressionStatement>(assignmentExpression));
    // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
    stmts.push_back(Allocator()->New<ir::ReturnStatement>(nullptr));
}

ir::MethodDefinition *ETSChecker::GenerateDefaultGetterSetter(ir::ClassProperty *const field,
                                                              varbinder::ClassScope *classScope, bool isSetter,
                                                              ETSChecker *checker)
{
    auto *paramScope = checker->Allocator()->New<varbinder::FunctionParamScope>(checker->Allocator(), classScope);
    auto *functionScope = checker->Allocator()->New<varbinder::FunctionScope>(checker->Allocator(), paramScope);

    functionScope->BindParamScope(paramScope);
    paramScope->BindFunctionScope(functionScope);

    auto flags = ir::ModifierFlags::PUBLIC;

    ArenaVector<ir::Expression *> params(checker->Allocator()->Adapter());
    ArenaVector<ir::Statement *> stmts(checker->Allocator()->Adapter());
    checker->GenerateGetterSetterBody(stmts, params, field, paramScope, isSetter);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
    auto *body = checker->AllocNode<ir::BlockStatement>(checker->Allocator(), std::move(stmts));
    auto funcFlags = isSetter ? ir::ScriptFunctionFlags::SETTER : ir::ScriptFunctionFlags::GETTER;
    auto *const returnTypeAnn = isSetter ? nullptr : field->TypeAnnotation()->Clone(checker->Allocator(), nullptr);
    auto *func =
        // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
        checker->AllocNode<ir::ScriptFunction>(ir::FunctionSignature(nullptr, std::move(params), returnTypeAnn), body,
                                               ir::ScriptFunction::ScriptFunctionData {funcFlags, flags, true});

    func->SetRange(field->Range());
    func->SetScope(functionScope);
    body->SetScope(functionScope);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
    auto *methodIdent = field->Key()->AsIdentifier()->Clone(checker->Allocator(), nullptr);
    auto *decl = checker->Allocator()->New<varbinder::FunctionDecl>(
        checker->Allocator(), field->Key()->AsIdentifier()->Name(),
        field->Key()->AsIdentifier()->Variable()->Declaration()->Node());
    auto *var = functionScope->AddDecl(checker->Allocator(), decl, ScriptExtension::ETS);

    methodIdent->SetVariable(var);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
    auto *funcExpr = checker->AllocNode<ir::FunctionExpression>(func);
    funcExpr->SetRange(func->Range());
    func->AddFlag(ir::ScriptFunctionFlags::METHOD);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
    auto *method = checker->AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD, methodIdent, funcExpr,
                                                            flags, checker->Allocator(), false);

    method->Id()->SetMutator();
    method->SetRange(field->Range());
    method->Function()->SetIdent(method->Id()->Clone(checker->Allocator(), nullptr));
    method->Function()->AddModifier(method->Modifiers());
    method->SetVariable(var);

    paramScope->BindNode(func);
    functionScope->BindNode(func);

    checker->VarBinder()->AsETSBinder()->ResolveMethodDefinition(method);
    functionScope->BindName(classScope->Node()->AsClassDefinition()->InternalName());
    method->Check(checker);

    return method;
}

const Type *ETSChecker::TryGettingFunctionTypeFromInvokeFunction(const Type *type) const
{
    if (type->IsETSObjectType() && type->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::FUNCTIONAL)) {
        auto const propInvoke = type->AsETSObjectType()->GetProperty(FUNCTIONAL_INTERFACE_INVOKE_METHOD_NAME,
                                                                     PropertySearchFlags::SEARCH_INSTANCE_METHOD);
        ASSERT(propInvoke != nullptr);

        return propInvoke->TsType();
    }

    return type;
}

bool ETSChecker::TryTransformingToStaticInvoke(ir::Identifier *const ident, const Type *resolvedType)
{
    ASSERT(ident->Parent()->IsCallExpression());
    ASSERT(ident->Parent()->AsCallExpression()->Callee() == ident);

    if (!resolvedType->IsETSObjectType()) {
        return false;
    }

    auto className = ident->Name();
    std::string_view propertyName;

    PropertySearchFlags searchFlag = PropertySearchFlags::SEARCH_IN_INTERFACES | PropertySearchFlags::SEARCH_IN_BASE |
                                     PropertySearchFlags::SEARCH_STATIC_METHOD;
    // clang-format off
    auto *instantiateMethod =
        resolvedType->AsETSObjectType()->GetProperty(compiler::Signatures::STATIC_INSTANTIATE_METHOD, searchFlag);
    auto *invokeMethod =
        resolvedType->AsETSObjectType()->GetProperty(compiler::Signatures::STATIC_INVOKE_METHOD, searchFlag);
    if (instantiateMethod != nullptr) {
        propertyName = compiler::Signatures::STATIC_INSTANTIATE_METHOD;
    } else if (invokeMethod != nullptr) {
        propertyName = compiler::Signatures::STATIC_INVOKE_METHOD;
    } else {
        ThrowTypeError({"No static ", compiler::Signatures::STATIC_INVOKE_METHOD, " method and static ",
                        compiler::Signatures::STATIC_INSTANTIATE_METHOD, " method in ", className, ". ", className,
                        "() is not allowed."},
                       ident->Start());
    }
    // clang-format on
    // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
    auto *classId = AllocNode<ir::Identifier>(className, Allocator());
    // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
    auto *methodId = AllocNode<ir::Identifier>(propertyName, Allocator());
    if (propertyName == compiler::Signatures::STATIC_INSTANTIATE_METHOD) {
        methodId->SetVariable(instantiateMethod);
    } else if (propertyName == compiler::Signatures::STATIC_INVOKE_METHOD) {
        methodId->SetVariable(invokeMethod);
    }

    auto *transformedCallee =
        // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
        AllocNode<ir::MemberExpression>(classId, methodId, ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);

    classId->SetRange(ident->Range());
    methodId->SetRange(ident->Range());
    transformedCallee->SetRange(ident->Range());

    auto *callExpr = ident->Parent()->AsCallExpression();
    transformedCallee->SetParent(callExpr);
    callExpr->SetCallee(transformedCallee);

    if (instantiateMethod != nullptr) {
        // SUPPRESS_CSA_NEXTLINE(alpha.core.AllocatorETSCheckerHint)
        auto *argExpr = GenerateImplicitInstantiateArg(instantiateMethod, std::string(className));

        argExpr->SetParent(callExpr);
        argExpr->SetRange(ident->Range());

        VarBinder()->AsETSBinder()->HandleCustomNodes(argExpr);

        auto &arguments = callExpr->Arguments();
        arguments.insert(arguments.begin(), argExpr);
    }

    return true;
}
}  // namespace ark::es2panda::checker
