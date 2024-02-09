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

#include "arithmetic.h"

#include "ir/expressions/identifier.h"
#include "varbinder/variable.h"
#include "varbinder/scope.h"
#include "varbinder/declaration.h"
#include "checker/ETSchecker.h"

namespace panda::es2panda::checker {

Type *ArithmeticChecker::NegateNumericType(CheckerType *type, ir::Expression *node)
{
    ASSERT(type->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC));

    TypeFlag typeKind = ETSType(type);
    Type *result = nullptr;

    switch (typeKind) {
        case TypeFlag::BYTE: {
            result = checker_->CreateByteType(-(type->AsByteType()->GetValue()));
            break;
        }
        case TypeFlag::CHAR: {
            result = checker_->CreateCharType(-(type->AsCharType()->GetValue()));
            break;
        }
        case TypeFlag::SHORT: {
            result = checker_->CreateShortType(-(type->AsShortType()->GetValue()));
            break;
        }
        case TypeFlag::INT: {
            result = checker_->CreateIntType(-(type->AsIntType()->GetValue()));
            break;
        }
        case TypeFlag::LONG: {
            result = checker_->CreateLongType(-(type->AsLongType()->GetValue()));
            break;
        }
        case TypeFlag::FLOAT: {
            result = checker_->CreateFloatType(-(type->AsFloatType()->GetValue()));
            break;
        }
        case TypeFlag::DOUBLE: {
            result = checker_->CreateDoubleType(-(type->AsDoubleType()->GetValue()));
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    node->SetTsType(result);
    return result;
}

Type *ArithmeticChecker::BitwiseNegateNumericType(CheckerType *type, ir::Expression *node)
{
    ASSERT(type->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_INTEGRAL));

    TypeFlag typeKind = ETSType(type);

    Type *result = nullptr;

    switch (typeKind) {
        case TypeFlag::BYTE: {
            auto value = static_cast<int8_t>(~static_cast<uint8_t>(type->AsByteType()->GetValue()));
            result = checker_->CreateByteType(value);
            break;
        }
        case TypeFlag::CHAR: {
            result = checker_->CreateCharType(~(type->AsCharType()->GetValue()));
            break;
        }
        case TypeFlag::SHORT: {
            auto value = static_cast<int16_t>(~static_cast<uint16_t>(type->AsShortType()->GetValue()));
            result = checker_->CreateShortType(value);
            break;
        }
        case TypeFlag::INT: {
            auto value = static_cast<int32_t>(~static_cast<uint32_t>(type->AsIntType()->GetValue()));
            result = checker_->CreateIntType(value);
            break;
        }
        case TypeFlag::LONG: {
            auto value = static_cast<int64_t>(~static_cast<uint64_t>(type->AsLongType()->GetValue()));
            result = checker_->CreateLongType(value);
            break;
        }
        case TypeFlag::FLOAT: {
            result = checker_->CreateIntType(
                ~static_cast<uint32_t>(CastFloatToInt<FloatType::UType, int32_t>(type->AsFloatType()->GetValue())));
            break;
        }
        case TypeFlag::DOUBLE: {
            result = checker_->CreateLongType(
                ~static_cast<uint64_t>(CastFloatToInt<DoubleType::UType, int64_t>(type->AsDoubleType()->GetValue())));
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    node->SetTsType(result);
    return result;
}

Type *ArithmeticChecker::HandleRelationOperationOnTypes(const BinaryOperationArgs &args)
{
    ASSERT(args.left->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC) &&
           args.right->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC));

    if (args.AnyDouble()) {
        return PerformRelationOperationOnTypes<DoubleType>(args);
    }

    if (args.AnyFloat()) {
        return PerformRelationOperationOnTypes<FloatType>(args);
    }

    if (args.AnyLong()) {
        return PerformRelationOperationOnTypes<LongType>(args);
    }

    return PerformRelationOperationOnTypes<IntType>(args);
}

bool ArithmeticChecker::CheckBinaryOperatorForBigInt(Type *left, Type *right, ir::Expression *expr, lexer::TokenType op)
{
    if ((left == nullptr) || (right == nullptr)) {
        return false;
    }

    if (!left->IsETSBigIntType()) {
        return false;
    }

    if (!right->IsETSBigIntType()) {
        return false;
    }

    if (expr->IsBinaryExpression()) {
        ir::BinaryExpression *be = expr->AsBinaryExpression();
        if (be->OperatorType() == lexer::TokenType::PUNCTUATOR_STRICT_EQUAL) {
            // Handle strict comparison as normal comparison for bigint objects
            be->SetOperator(lexer::TokenType::PUNCTUATOR_EQUAL);
        }
    }

    switch (op) {
        case lexer::TokenType::PUNCTUATOR_EQUAL:
        case lexer::TokenType::KEYW_INSTANCEOF:
            // This is handled in the main CheckBinaryOperator function
            return false;
        default:
            break;
    }

    // Remove const flag - currently there are no compile time operations for bigint
    left->RemoveTypeFlag(TypeFlag::CONSTANT);
    right->RemoveTypeFlag(TypeFlag::CONSTANT);

    return true;
}

std::tuple<Type *, bool> ArithmeticChecker::ApplyBinaryOperatorPromotion(Type *left, Type *right, TypeFlag test,
                                                                         bool doPromotion)
{
    Type *unboxedL = checker_->ETSBuiltinTypeAsPrimitiveType(left);
    Type *unboxedR = checker_->ETSBuiltinTypeAsPrimitiveType(right);
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
    Type *result = unboxedR;
    if (doPromotion) {
        if (unboxedL->HasTypeFlag(TypeFlag::ETS_NUMERIC) && unboxedR->HasTypeFlag(TypeFlag::ETS_NUMERIC)) {
            if (unboxedL->IsDoubleType() || unboxedR->IsDoubleType()) {
                result = checker_->GlobalDoubleType();
            } else if (unboxedL->IsFloatType() || unboxedR->IsFloatType()) {
                result = checker_->GlobalFloatType();
            } else if (unboxedL->IsLongType() || unboxedR->IsLongType()) {
                result = checker_->GlobalLongType();
            } else if (unboxedL->IsCharType() && unboxedR->IsCharType()) {
                result = checker_->GlobalCharType();
            } else {
                result = checker_->GlobalIntType();
            }
        } else if (checker_->IsTypeIdenticalTo(unboxedL, unboxedR)) {
            result = unboxedL;
        }
    }

    return {result, bothConst};
}

Type *ArithmeticChecker::CheckBinaryOperatorMulDivMod(BinaryOpData data, bool isEqualOp, BinaryOpTypes types)
{
    Type *tsType {};
    auto [promotedType, bothConst] =
        ApplyBinaryOperatorPromotion(types.unboxedL, types.unboxedR, TypeFlag::ETS_NUMERIC, !isEqualOp);

    FlagExpressionWithUnboxing(types.left, types.unboxedL, data.left);
    FlagExpressionWithUnboxing(types.right, types.unboxedR, data.right);

    if (types.left->IsETSUnionType() || types.right->IsETSUnionType()) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", data.pos);
    }

    if (promotedType == nullptr && !bothConst) {
        ThrowTypeError("Bad operand type, the types of the operands must be numeric type.", data.pos);
    }

    if (bothConst) {
        tsType = HandleArithmeticOperationOnTypes(types);
    }

    tsType = (tsType != nullptr) ? tsType : promotedType;
    return tsType;
}

Type *ArithmeticChecker::CheckBinaryOperatorPlus(BinaryOpData data, bool isEqualOp, BinaryOpTypes types)
{
    if (types.AnyETSUnion()) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", data.pos);
    }

    if (types.AnyETSString()) {
        if (data.operationType == lexer::TokenType::PUNCTUATOR_MINUS ||
            data.operationType == lexer::TokenType::PUNCTUATOR_MINUS_EQUAL) {
            ThrowTypeError("Bad operand type, the types of the operands must be numeric type.", data.pos);
        }

        return checker_->HandleStringConcatenation(types.left, types.right);
    }

    auto [promotedType, bothConst] =
        ApplyBinaryOperatorPromotion(types.unboxedL, types.unboxedR, TypeFlag::ETS_NUMERIC, !isEqualOp);

    FlagExpressionWithUnboxing(types.left, types.unboxedL, data.left);
    FlagExpressionWithUnboxing(types.right, types.unboxedR, data.right);

    if (promotedType == nullptr && !bothConst) {
        ThrowTypeError("Bad operand type, the types of the operands must be numeric type or String.", data.pos);
    }

    if (bothConst) {
        return HandleArithmeticOperationOnTypes(types);
    }

    return promotedType;
}

Type *ArithmeticChecker::CheckBinaryOperatorShift(BinaryOpData data, bool isEqualOp, BinaryOpTypes types)
{
    if (types.AnyETSUnion()) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", data.pos);
    }

    auto promotedLeftType = checker_->ApplyUnaryOperatorPromotion(types.unboxedL, false, !isEqualOp);
    auto promotedRightType = checker_->ApplyUnaryOperatorPromotion(types.unboxedR, false, !isEqualOp);

    FlagExpressionWithUnboxing(types.left, types.unboxedL, data.left);
    FlagExpressionWithUnboxing(types.right, types.unboxedR, data.right);

    if (promotedLeftType == nullptr || !promotedLeftType->HasTypeFlag(TypeFlag::ETS_NUMERIC) ||
        promotedRightType == nullptr || !promotedRightType->HasTypeFlag(TypeFlag::ETS_NUMERIC)) {
        ThrowTypeError("Bad operand type, the types of the operands must be numeric type.", data.pos);
    }

    if (promotedLeftType->HasTypeFlag(TypeFlag::CONSTANT) && promotedRightType->HasTypeFlag(TypeFlag::CONSTANT)) {
        return HandleBitwiseOperationOnTypes({promotedLeftType, promotedRightType, data.operationType});
    }

    switch (ETSType(promotedLeftType)) {
        case TypeFlag::BYTE: {
            return checker_->GlobalByteType();
        }
        case TypeFlag::SHORT: {
            return checker_->GlobalShortType();
        }
        case TypeFlag::CHAR: {
            return checker_->GlobalCharType();
        }
        case TypeFlag::INT:
        case TypeFlag::FLOAT: {
            return checker_->GlobalIntType();
        }
        case TypeFlag::LONG:
        case TypeFlag::DOUBLE: {
            return checker_->GlobalLongType();
        }
        default: {
            UNREACHABLE();
        }
    }
    return nullptr;
}

Type *ArithmeticChecker::CheckBinaryOperatorBitwise(BinaryOpData data, bool isEqualOp, BinaryOpTypes types)
{
    // NOTE (mmartin): These need to be done for other binary expressions, but currently it's not defined precisely when
    // to apply this conversion

    if (types.left->IsETSEnumType()) {
        data.left->AddAstNodeFlags(ir::AstNodeFlags::ENUM_GET_VALUE);
        types.unboxedL = checker_->GlobalIntType();
    }

    if (types.right->IsETSEnumType()) {
        data.right->AddAstNodeFlags(ir::AstNodeFlags::ENUM_GET_VALUE);
        types.unboxedR = checker_->GlobalIntType();
    }

    if (types.AnyETSUnion()) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", data.pos);
    }

    if (types.AllUnboxedBoolean()) {
        FlagExpressionWithUnboxing(types.left, types.unboxedL, data.left);
        FlagExpressionWithUnboxing(types.right, types.unboxedR, data.right);
        return HandleBooleanLogicalOperators({types.unboxedL, types.unboxedR, data.operationType});
    }

    auto [promotedType, bothConst] =
        ApplyBinaryOperatorPromotion(types.unboxedL, types.unboxedR, TypeFlag::ETS_NUMERIC, !isEqualOp);

    FlagExpressionWithUnboxing(types.left, types.unboxedL, data.left);
    FlagExpressionWithUnboxing(types.right, types.unboxedR, data.right);

    if (promotedType == nullptr && !bothConst) {
        ThrowTypeError("Bad operand type, the types of the operands must be numeric type.", data.pos);
    }

    if (bothConst) {
        return HandleBitwiseOperationOnTypes(types);
    }

    return checker_->SelectGlobalIntegerTypeForNumeric(promotedType);
}

Type *ArithmeticChecker::CheckBinaryOperatorLogical(BinaryOpData data, BinaryOpTypes types)
{
    if (types.AnyETSUnion()) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", data.pos);
    }

    if (types.unboxedL == nullptr || !types.unboxedL->IsConditionalExprType() || types.unboxedR == nullptr ||
        !types.unboxedR->IsConditionalExprType()) {
        ThrowTypeError("Bad operand type, the types of the operands must be of possible condition type.", data.pos);
    }

    if (types.unboxedL->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
        FlagExpressionWithUnboxing(types.left, types.unboxedL, data.left);
    }

    if (types.unboxedR->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
        FlagExpressionWithUnboxing(types.right, types.unboxedR, data.right);
    }

    if (data.expr->IsBinaryExpression()) {
        return checker_->HandleBooleanLogicalOperatorsExtended(types.unboxedL, types.unboxedR,
                                                               data.expr->AsBinaryExpression());
    }

    UNREACHABLE();
}

Type *ArithmeticChecker::HandleBooleanLogicalOperators(BinaryOperationArgs args)
{
    using UType = typename ETSBooleanType::UType;
    ASSERT(args.left->IsETSBooleanType() && args.right->IsETSBooleanType());

    if (!args.left->HasTypeFlag(TypeFlag::CONSTANT) || !args.right->HasTypeFlag(TypeFlag::CONSTANT)) {
        return checker_->GlobalETSBooleanType();
    }

    UType leftValue = args.left->AsETSBooleanType()->GetValue();
    UType rightValue = args.right->AsETSBooleanType()->GetValue();

    bool result;
    switch (args.operationType) {
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR: {
            result = (leftValue ^ rightValue) != 0;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND: {
            result = (static_cast<uint8_t>(leftValue) & static_cast<uint8_t>(rightValue)) != 0;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR: {
            result = (static_cast<uint8_t>(leftValue) | static_cast<uint8_t>(rightValue)) != 0;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LOGICAL_OR: {
            result = leftValue || rightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LOGICAL_AND: {
            result = leftValue && rightValue;
            break;
        }
        default: {
            UNREACHABLE();
            return nullptr;
        }
    }
    return checker_->CreateETSBooleanType(result);
}

ArithmeticChecker::CheckBinaryT ArithmeticChecker::CheckBinaryOperatorStrictEqual(BinaryOpData data,
                                                                                  BinaryOpTypes types)
{
    Type *tsType {};
    if (!(types.left->HasTypeFlag(TypeFlag::ETS_ARRAY_OR_OBJECT) || types.left->IsETSUnionType()) ||
        !(types.right->HasTypeFlag(TypeFlag::ETS_ARRAY_OR_OBJECT) || types.right->IsETSUnionType())) {
        ThrowTypeError("Both operands have to be reference types", data.pos);
    }

    checker_->Relation()->SetNode(data.left);
    if (!checker_->Relation()->IsCastableTo(types.left, types.right) &&
        !checker_->Relation()->IsCastableTo(types.right, types.left)) {
        ThrowTypeError("The operands of strict equality are not compatible with each other", data.pos);
    }
    tsType = checker_->GlobalETSBooleanType();
    if (types.AllETSDynamic()) {
        return {tsType, checker_->GlobalBuiltinJSValueType()};
    }
    return {tsType, checker_->GlobalETSObjectType()};
}

ArithmeticChecker::CheckBinaryT ArithmeticChecker::CheckBinaryOperatorEqual(BinaryOpData data, BinaryOpTypes types)
{
    Type *tsType {};
    if (types.AllETSEnum() || types.AllETSStringEnum()) {
        bool isSameEnum = true;
        if (types.AllETSEnum()) {
            isSameEnum = types.left->AsETSEnumType()->IsSameEnumType(types.right->AsETSEnumType());
        } else if (types.AllETSStringEnum()) {
            isSameEnum = types.left->AsETSStringEnumType()->IsSameEnumType(types.right->AsETSStringEnumType());
        } else {
            UNREACHABLE();
        }
        if (!isSameEnum) {
            ThrowTypeError("Bad operand type, the types of the operands must be the same enum type.", data.pos);
        }

        tsType = checker_->GlobalETSBooleanType();
        return {tsType, types.left};
    }

    if (types.AnyETSDynamic()) {
        return CheckBinaryOperatorEqualDynamic(data);
    }

    if (types.AllReference()) {
        tsType = checker_->GlobalETSBooleanType();
        auto *opType = checker_->GlobalETSObjectType();
        return {tsType, opType};
    }

    if (types.AllUnboxedBoolean()) {
        if (types.unboxedL->HasTypeFlag(TypeFlag::CONSTANT) && types.unboxedR->HasTypeFlag(TypeFlag::CONSTANT)) {
            bool res = types.unboxedL->AsETSBooleanType()->GetValue() == types.unboxedR->AsETSBooleanType()->GetValue();

            tsType =
                checker_->CreateETSBooleanType(data.operationType == lexer::TokenType::PUNCTUATOR_EQUAL ? res : !res);
            return {tsType, tsType};
        }

        FlagExpressionWithUnboxing(types.left, types.unboxedL, data.left);
        FlagExpressionWithUnboxing(types.right, types.unboxedR, data.right);

        tsType = checker_->GlobalETSBooleanType();
        return {tsType, tsType};
    }
    return {nullptr, nullptr};
}

ArithmeticChecker::CheckBinaryT ArithmeticChecker::CheckBinaryOperatorEqualDynamic(BinaryOpData data)
{
    // NOTE: vpukhov. enforce intrinsic call in any case?
    // canonicalize
    auto *const dynExp = data.left->TsType()->IsETSDynamicType() ? data.left : data.right;
    auto *const otherExp = dynExp == data.left ? data.right : data.left;

    if (otherExp->TsType()->IsETSDynamicType()) {
        return {checker_->GlobalETSBooleanType(), checker_->GlobalBuiltinJSValueType()};
    }
    if (dynExp->TsType()->AsETSDynamicType()->IsConvertible(otherExp->TsType())) {
        // NOTE: vpukhov. boxing flags are not set in dynamic values
        return {checker_->GlobalETSBooleanType(), otherExp->TsType()};
    }
    if (checker_->IsReferenceType(otherExp->TsType())) {
        // have to prevent casting dyn_exp via ApplyCast without nullish flag
        return {checker_->GlobalETSBooleanType(), checker_->GlobalETSNullishObjectType()};
    }
    ThrowTypeError("Unimplemented case in dynamic type comparison.", data.pos);
}

ArithmeticChecker::CheckBinaryT ArithmeticChecker::CheckBinaryOperatorLessGreater(BinaryOpData data, bool isEqualOp,
                                                                                  BinaryOpTypes types)
{
    if (types.AnyETSUnion() && data.operationType != lexer::TokenType::PUNCTUATOR_EQUAL &&
        data.operationType != lexer::TokenType::PUNCTUATOR_NOT_EQUAL) {
        ThrowTypeError("Bad operand type, unions are not allowed in binary expressions except equality.", data.pos);
    }

    Type *tsType {};
    auto [promotedType, bothConst] =
        ApplyBinaryOperatorPromotion(types.unboxedL, types.unboxedR, TypeFlag::ETS_NUMERIC, !isEqualOp);

    FlagExpressionWithUnboxing(types.left, types.unboxedL, data.left);
    FlagExpressionWithUnboxing(types.right, types.unboxedR, data.right);

    if (types.left->IsETSUnionType()) {
        tsType = checker_->GlobalETSBooleanType();
        return {tsType, types.left->AsETSUnionType()};
    }

    if (types.right->IsETSUnionType()) {
        tsType = checker_->GlobalETSBooleanType();
        return {tsType, types.right->AsETSUnionType()};
    }

    if (promotedType == nullptr && !bothConst) {
        ThrowTypeError("Bad operand type, the types of the operands must be numeric type.", data.pos);
    }

    if (bothConst) {
        tsType = HandleRelationOperationOnTypes(types);
        return {tsType, tsType};
    }

    tsType = checker_->GlobalETSBooleanType();
    auto *opType = promotedType;
    return {tsType, opType};
}

ArithmeticChecker::CheckBinaryT ArithmeticChecker::CheckBinaryOperatorInstanceOf(lexer::SourcePosition pos,
                                                                                 const BinaryOperationArgs &args)
{
    Type *tsType {};
    if (!args.AllReference()) {
        ThrowTypeError("Bad operand type, the types of the operands must be same type.", pos);
    }

    if (args.AnyETSDynamic()) {
        if (!args.AllETSDynamic()) {
            ThrowTypeError("Bad operand type, both types of the operands must be dynamic.", pos);
        }
    }

    tsType = checker_->GlobalETSBooleanType();
    Type *opType = nullptr;
    if (args.right->IsETSDynamicType()) {
        opType = checker_->GlobalBuiltinJSValueType();
    } else {
        opType = checker_->GlobalETSObjectType();
    }
    return {tsType, opType};
}

Type *ArithmeticChecker::CheckBinaryOperatorNullishCoalescing(BinaryOpData data, BinaryOpTypes types)
{
    if (!types.left->HasTypeFlag(TypeFlag::ETS_ARRAY_OR_OBJECT) && !types.left->IsETSTypeParameter()) {
        ThrowTypeError("Left-hand side expression must be a reference type.", data.pos);
    }

    Type *nonNullishLeftType = types.left;
    Type *rightType = types.right;

    if (nonNullishLeftType->IsNullish()) {
        nonNullishLeftType = checker_->GetNonNullishType(nonNullishLeftType);
    }

    // NOTE: vpukhov. check convertibility and use numeric promotion

    if (rightType->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
        checker_->Relation()->SetNode(data.right);
        auto *const boxedRightType = checker_->PrimitiveTypeAsETSBuiltinType(rightType);
        if (boxedRightType == nullptr) {
            ThrowTypeError("Invalid right-hand side expression", data.pos);
        }
        data.right->AddBoxingUnboxingFlags(checker_->GetBoxingFlag(boxedRightType));
        rightType = boxedRightType;
    }

    if (rightType->IsETSNullType() || rightType->IsETSUndefinedType()) {
        TypeFlag flag = rightType->IsETSNullType() ? TypeFlag::NULL_TYPE : TypeFlag::UNDEFINED;
        return checker_->CreateNullishType(nonNullishLeftType, flag, Allocator(), checker_->Relation(),
                                           checker_->GetGlobalTypesHolder());
    }

    if (nonNullishLeftType->IsETSTypeParameter()) {
        nonNullishLeftType = nonNullishLeftType->AsETSTypeParameter()->GetConstraintType();
    }

    if (rightType->IsETSTypeParameter()) {
        rightType = rightType->AsETSTypeParameter()->GetConstraintType();
    }

    return checker_->FindLeastUpperBound(nonNullishLeftType, rightType);
}

std::map<lexer::TokenType, ArithmeticChecker::CheckBinaryFunction> &ArithmeticChecker::GetCheckMap()
{
    static std::map<lexer::TokenType, CheckBinaryFunction> checkMap = {
        {lexer::TokenType::PUNCTUATOR_MULTIPLY, &ArithmeticChecker::CheckBinaryOperatorMulDivMod},
        {lexer::TokenType::PUNCTUATOR_MULTIPLY_EQUAL, &ArithmeticChecker::CheckBinaryOperatorMulDivMod},
        {lexer::TokenType::PUNCTUATOR_DIVIDE, &ArithmeticChecker::CheckBinaryOperatorMulDivMod},
        {lexer::TokenType::PUNCTUATOR_DIVIDE_EQUAL, &ArithmeticChecker::CheckBinaryOperatorMulDivMod},
        {lexer::TokenType::PUNCTUATOR_MOD, &ArithmeticChecker::CheckBinaryOperatorMulDivMod},
        {lexer::TokenType::PUNCTUATOR_MOD_EQUAL, &ArithmeticChecker::CheckBinaryOperatorMulDivMod},

        {lexer::TokenType::PUNCTUATOR_MINUS, &ArithmeticChecker::CheckBinaryOperatorPlus},
        {lexer::TokenType::PUNCTUATOR_MINUS_EQUAL, &ArithmeticChecker::CheckBinaryOperatorPlus},
        {lexer::TokenType::PUNCTUATOR_PLUS, &ArithmeticChecker::CheckBinaryOperatorPlus},
        {lexer::TokenType::PUNCTUATOR_PLUS_EQUAL, &ArithmeticChecker::CheckBinaryOperatorPlus},

        {lexer::TokenType::PUNCTUATOR_LEFT_SHIFT, &ArithmeticChecker::CheckBinaryOperatorShift},
        {lexer::TokenType::PUNCTUATOR_LEFT_SHIFT_EQUAL, &ArithmeticChecker::CheckBinaryOperatorShift},
        {lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT, &ArithmeticChecker::CheckBinaryOperatorShift},
        {lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT_EQUAL, &ArithmeticChecker::CheckBinaryOperatorShift},
        {lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT, &ArithmeticChecker::CheckBinaryOperatorShift},
        {lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT_EQUAL, &ArithmeticChecker::CheckBinaryOperatorShift},

        {lexer::TokenType::PUNCTUATOR_BITWISE_OR, &ArithmeticChecker::CheckBinaryOperatorBitwise},
        {lexer::TokenType::PUNCTUATOR_BITWISE_OR_EQUAL, &ArithmeticChecker::CheckBinaryOperatorBitwise},
        {lexer::TokenType::PUNCTUATOR_BITWISE_AND, &ArithmeticChecker::CheckBinaryOperatorBitwise},
        {lexer::TokenType::PUNCTUATOR_BITWISE_AND_EQUAL, &ArithmeticChecker::CheckBinaryOperatorBitwise},
        {lexer::TokenType::PUNCTUATOR_BITWISE_XOR, &ArithmeticChecker::CheckBinaryOperatorBitwise},
        {lexer::TokenType::PUNCTUATOR_BITWISE_XOR_EQUAL, &ArithmeticChecker::CheckBinaryOperatorBitwise},
    };

    return checkMap;
}

ArithmeticChecker::CheckBinaryT ArithmeticChecker::CheckBinaryOperatorHelper(ArithmeticChecker::BinaryOpData data,
                                                                             ArithmeticChecker::BinaryOpTypes types,
                                                                             bool isEqualOp)
{
    Type *tsType {};
    switch (data.operationType) {
        case lexer::TokenType::PUNCTUATOR_LOGICAL_AND:
        case lexer::TokenType::PUNCTUATOR_LOGICAL_OR: {
            tsType = CheckBinaryOperatorLogical(data, types);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_STRICT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_NOT_STRICT_EQUAL: {
            return CheckBinaryOperatorStrictEqual(data, types);
        }
        case lexer::TokenType::PUNCTUATOR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_NOT_EQUAL: {
            ArithmeticChecker::CheckBinaryT res = CheckBinaryOperatorEqual(data, types);
            if (!(std::get<0>(res) == nullptr && std::get<1>(res) == nullptr)) {
                return res;
            }
            [[fallthrough]];
        }
        case lexer::TokenType::PUNCTUATOR_LESS_THAN:
        case lexer::TokenType::PUNCTUATOR_LESS_THAN_EQUAL:
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN:
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN_EQUAL: {
            return CheckBinaryOperatorLessGreater(data, isEqualOp, types);
        }
        case lexer::TokenType::KEYW_INSTANCEOF: {
            return CheckBinaryOperatorInstanceOf(data.pos, types);
        }
        case lexer::TokenType::PUNCTUATOR_NULLISH_COALESCING: {
            tsType = CheckBinaryOperatorNullishCoalescing(data, types);
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }

    return {tsType, tsType};
}

ArithmeticChecker::CheckBinaryT ArithmeticChecker::CheckBinaryOperator(BinaryOpData data, checker::Type *leftType,
                                                                       checker::Type *rightType, bool forcePromotion)
{
    if ((leftType == nullptr) || (rightType == nullptr)) {
        ThrowTypeError("Unexpected type error in binary expression", data.pos);
    }

    const bool isLogicalExtendedOperator = (data.operationType == lexer::TokenType::PUNCTUATOR_LOGICAL_AND) ||
                                           (data.operationType == lexer::TokenType::PUNCTUATOR_LOGICAL_OR);
    Type *unboxedL = isLogicalExtendedOperator ? checker_->ETSBuiltinTypeAsConditionalType(leftType)
                                               : checker_->ETSBuiltinTypeAsPrimitiveType(leftType);
    Type *unboxedR = isLogicalExtendedOperator ? checker_->ETSBuiltinTypeAsConditionalType(rightType)
                                               : checker_->ETSBuiltinTypeAsPrimitiveType(rightType);

    bool isEqualOp = (data.operationType > lexer::TokenType::PUNCTUATOR_SUBSTITUTION &&
                      data.operationType < lexer::TokenType::PUNCTUATOR_ARROW) &&
                     !forcePromotion;

    if (CheckBinaryOperatorForBigInt(leftType, rightType, data.expr, data.operationType)) {
        switch (data.operationType) {
            case lexer::TokenType::PUNCTUATOR_GREATER_THAN:
            case lexer::TokenType::PUNCTUATOR_LESS_THAN:
            case lexer::TokenType::PUNCTUATOR_GREATER_THAN_EQUAL:
            case lexer::TokenType::PUNCTUATOR_LESS_THAN_EQUAL:
                return {checker_->GlobalETSBooleanType(), checker_->GlobalETSBooleanType()};
            default:
                return {leftType, rightType};
        }
    };
    BinaryOpTypes types = {{leftType, rightType, data.operationType}, unboxedL, unboxedR};
    auto checkMap = GetCheckMap();
    if (checkMap.find(data.operationType) != checkMap.end()) {
        auto check = checkMap[data.operationType];
        auto tsType = check(this, data, isEqualOp, types);
        return {tsType, tsType};
    }

    return CheckBinaryOperatorHelper(data, types, isEqualOp);
}

Type *ArithmeticChecker::HandleArithmeticOperationOnTypes(const BinaryOperationArgs &args)
{
    ASSERT(args.left->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC) &&
           args.right->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC));

    if (args.left->IsDoubleType() || args.right->IsDoubleType()) {
        return PerformArithmeticOperationOnTypes<DoubleType>(args);
    }

    if (args.left->IsFloatType() || args.right->IsFloatType()) {
        return PerformArithmeticOperationOnTypes<FloatType>(args);
    }

    if (args.left->IsLongType() || args.right->IsLongType()) {
        return PerformArithmeticOperationOnTypes<LongType>(args);
    }

    return PerformArithmeticOperationOnTypes<IntType>(args);
}

Type *ArithmeticChecker::HandleBitwiseOperationOnTypes(const BinaryOperationArgs &args)
{
    ASSERT(args.left->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC) &&
           args.right->HasTypeFlag(TypeFlag::CONSTANT | TypeFlag::ETS_NUMERIC));

    if (args.left->IsDoubleType() || args.right->IsDoubleType()) {
        return HandleBitWiseArithmetic<DoubleType, LongType>(args);
    }

    if (args.left->IsFloatType() || args.right->IsFloatType()) {
        return HandleBitWiseArithmetic<FloatType, IntType>(args);
    }

    if (args.left->IsLongType() || args.right->IsLongType()) {
        return HandleBitWiseArithmetic<LongType>(args);
    }

    return HandleBitWiseArithmetic<IntType>(args);
}

void ArithmeticChecker::FlagExpressionWithUnboxing(CheckerType *type, CheckerType *unboxedType,
                                                   ir::Expression *typeExpression)
{
    return checker_->FlagExpressionWithUnboxing(type, unboxedType, typeExpression);
}

Type *ArithmeticChecker::CreateETSBooleanType(bool val)
{
    return checker_->CreateETSBooleanType(val);
}

ArenaAllocator *ArithmeticChecker::Allocator() const
{
    return checker_->Allocator();
}

void ArithmeticChecker::ThrowTypeError(std::string_view message, const lexer::SourcePosition &pos)
{
    checker_->ThrowTypeError(message, pos);
}

Type *ArithmeticChecker::ProcessExclamationMark(ir::UnaryExpression *expr, CheckerType *operandType)
{
    if (checker_->IsNullLikeOrVoidExpression(expr->Argument())) {
        auto tsType = checker_->CreateETSBooleanType(true);
        tsType->AddTypeFlag(checker::TypeFlag::CONSTANT);
        return tsType;
    }

    if (operandType == nullptr || !operandType->IsConditionalExprType()) {
        ThrowTypeError("Bad operand type, the type of the operand must be boolean type.", expr->Argument()->Start());
    }

    auto exprRes = operandType->ResolveConditionExpr();
    if (std::get<0>(exprRes)) {
        auto tsType = checker_->CreateETSBooleanType(!std::get<1>(exprRes));
        tsType->AddTypeFlag(checker::TypeFlag::CONSTANT);
        return tsType;
    }

    return checker_->GlobalETSBooleanType();
}

checker::Type *ArithmeticChecker::CheckUnaryOperatorHelper(ir::UnaryExpression *expr, checker::Type *operandType,
                                                           checker::Type *argType)
{
    switch (expr->OperatorType()) {
        case lexer::TokenType::PUNCTUATOR_MINUS:
        case lexer::TokenType::PUNCTUATOR_PLUS: {
            if (operandType == nullptr || !operandType->HasTypeFlag(checker::TypeFlag::ETS_NUMERIC)) {
                ThrowTypeError("Bad operand type, the type of the operand must be numeric type.",
                               expr->Argument()->Start());
            }

            if (operandType->HasTypeFlag(checker::TypeFlag::CONSTANT) &&
                expr->OperatorType() == lexer::TokenType::PUNCTUATOR_MINUS) {
                return NegateNumericType(operandType, expr);
            }

            return operandType;
        }
        case lexer::TokenType::PUNCTUATOR_TILDE: {
            if (operandType == nullptr || !operandType->HasTypeFlag(checker::TypeFlag::ETS_NUMERIC)) {
                ThrowTypeError("Bad operand type, the type of the operand must be numeric type.",
                               expr->Argument()->Start());
            }

            if (operandType->HasTypeFlag(checker::TypeFlag::CONSTANT)) {
                return BitwiseNegateNumericType(operandType, expr);
            }

            return checker_->SelectGlobalIntegerTypeForNumeric(operandType);
        }
        case lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK: {
            return ProcessExclamationMark(expr, operandType);
        }
        case lexer::TokenType::PUNCTUATOR_DOLLAR_DOLLAR: {
            return argType;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }
}

bool ArithmeticChecker::BinaryOperationArgs::AllReference() const
{
    return ETSChecker::IsReferenceType(left) && ETSChecker::IsReferenceType(right);
}
}  // namespace panda::es2panda::checker
