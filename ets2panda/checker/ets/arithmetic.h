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

#ifndef ES2PANDA_COMPILER_CHECKER_ETS_ARITHMETIC_H
#define ES2PANDA_COMPILER_CHECKER_ETS_ARITHMETIC_H

#include "checker/types/ets/types.h"

namespace panda::es2panda::checker {

class ETSChecker;

class ArithmeticChecker {
public:
    using CheckBinaryT = std::tuple<Type *, Type *>;

    // Arithmetic
    struct BinaryOpData {
        ir::Expression *left;
        ir::Expression *right;
        ir::Expression *expr;
        lexer::TokenType operationType;
        lexer::SourcePosition pos;
    };

    // Note: From checker we need only a few helper methods. Remove this dependency once they extracted.
    explicit ArithmeticChecker(ETSChecker *checker) : checker_(checker) {}

    CheckBinaryT CheckBinaryOperator(BinaryOpData data, checker::Type *leftType, checker::Type *rightType,
                                     bool forcePromotion = false);
    Type *CheckUnaryOperatorHelper(ir::UnaryExpression *expr, checker::Type *operandType, checker::Type *argType);

private:
    struct BinaryOperationArgs {
        bool AnyDouble() const
        {
            return left->IsDoubleType() || right->IsDoubleType();
        }

        bool AnyFloat() const
        {
            return left->IsFloatType() || right->IsFloatType();
        }

        bool AnyLong() const
        {
            return left->IsLongType() || right->IsLongType();
        }

        bool AnyETSUnion() const
        {
            return left->IsETSUnionType() || right->IsETSUnionType();
        }

        bool AnyETSString() const
        {
            return left->IsETSStringType() || right->IsETSStringType();
        }

        bool AnyETSDynamic() const
        {
            return left->IsETSDynamicType() || right->IsETSDynamicType();
        }

        bool AllETSDynamic() const
        {
            return left->IsETSDynamicType() && right->IsETSDynamicType();
        }

        bool AllReference() const;

        bool AllETSEnum() const
        {
            return left->IsETSEnumType() && right->IsETSEnumType();
        }

        bool AllETSStringEnum() const
        {
            return left->IsETSStringEnumType() && right->IsETSStringEnumType();
        }

        Type *left;                      // NOLINT(misc-non-private-member-variables-in-classes)
        Type *right;                     // NOLINT(misc-non-private-member-variables-in-classes)
        lexer::TokenType operationType;  // NOLINT(misc-non-private-member-variables-in-classes)
    };

    struct BinaryOpTypes : public BinaryOperationArgs {
        bool AllUnboxedBoolean() const
        {
            return unboxedL != nullptr && unboxedL->HasTypeFlag(TypeFlag::ETS_BOOLEAN) && unboxedR != nullptr &&
                   unboxedR->HasTypeFlag(TypeFlag::ETS_BOOLEAN);
        }

        Type *unboxedL;  // NOLINT(misc-non-private-member-variables-in-classes)
        Type *unboxedR;  // NOLINT(misc-non-private-member-variables-in-classes)
    };

    using CheckBinaryFunction = std::function<checker::Type *(ArithmeticChecker *, ArithmeticChecker::BinaryOpData data,
                                                              bool isEqualOp, ArithmeticChecker::BinaryOpTypes types)>;
    static std::map<lexer::TokenType, CheckBinaryFunction> &GetCheckMap();

    ArithmeticChecker::CheckBinaryT CheckBinaryOperatorHelper(ArithmeticChecker::BinaryOpData data,
                                                              ArithmeticChecker::BinaryOpTypes types, bool isEqualOp);

    Type *NegateNumericType(CheckerType *type, ir::Expression *node);
    Type *BitwiseNegateNumericType(CheckerType *type, ir::Expression *node);
    bool CheckBinaryOperatorForBigInt(Type *left, Type *right, ir::Expression *expr, lexer::TokenType op);
    Type *CheckBinaryOperatorMulDivMod(BinaryOpData data, bool isEqualOp, BinaryOpTypes types);
    checker::Type *CheckBinaryOperatorPlus(BinaryOpData data, bool isEqualOp, BinaryOpTypes types);
    checker::Type *CheckBinaryOperatorShift(BinaryOpData data, bool isEqualOp, BinaryOpTypes types);
    checker::Type *CheckBinaryOperatorBitwise(BinaryOpData data, bool isEqualOp, BinaryOpTypes types);
    checker::Type *CheckBinaryOperatorLogical(BinaryOpData data, BinaryOpTypes types);
    CheckBinaryT CheckBinaryOperatorStrictEqual(BinaryOpData data, BinaryOpTypes types);
    CheckBinaryT CheckBinaryOperatorEqual(BinaryOpData data, BinaryOpTypes types);
    CheckBinaryT CheckBinaryOperatorEqualDynamic(BinaryOpData data);
    CheckBinaryT CheckBinaryOperatorLessGreater(BinaryOpData data, bool isEqualOp, BinaryOpTypes types);
    CheckBinaryT CheckBinaryOperatorInstanceOf(lexer::SourcePosition pos, const BinaryOperationArgs &args);
    checker::Type *CheckBinaryOperatorNullishCoalescing(BinaryOpData data, BinaryOpTypes types);
    Type *HandleArithmeticOperationOnTypes(const BinaryOperationArgs &args);
    Type *HandleBitwiseOperationOnTypes(const BinaryOperationArgs &args);
    void FlagExpressionWithUnboxing(CheckerType *type, CheckerType *unboxedType, ir::Expression *typeExpression);
    template <typename ValueType>
    Type *PerformArithmeticOperationOnTypes(BinaryOperationArgs args);

    Type *HandleRelationOperationOnTypes(const BinaryOperationArgs &args);
    template <typename TargetType>
    Type *PerformRelationOperationOnTypes(BinaryOperationArgs args);

    template <typename FloatOrIntegerType, typename IntegerType = FloatOrIntegerType>
    Type *HandleBitWiseArithmetic(BinaryOperationArgs args);

    template <typename TargetType>
    typename TargetType::UType GetOperand(CheckerType *type);

    template <typename UType>
    UType HandleModulo(UType leftValue, UType rightValue);

    [[nodiscard]] static inline TypeFlag ETSType(CheckerType *const type) noexcept
    {
        return static_cast<TypeFlag>(type->TypeFlags() & TypeFlag::ETS_TYPE);
    }

    Type *HandleBooleanLogicalOperators(BinaryOperationArgs args);
    std::tuple<Type *, bool> ApplyBinaryOperatorPromotion(Type *left, Type *right, TypeFlag test,
                                                          bool doPromotion = true);

    Type *ProcessExclamationMark(ir::UnaryExpression *expr, CheckerType *operandType);

    Type *CreateETSBooleanType(bool val);
    ArenaAllocator *Allocator() const;
    [[noreturn]] void ThrowTypeError(std::string_view message, const lexer::SourcePosition &pos);

private:
    ETSChecker *const checker_;  // NOTE: rid of it once checker separated into multiple files
};

template <typename TargetType>
typename TargetType::UType ArithmeticChecker::GetOperand(CheckerType *type)
{
    switch (ETSType(type)) {
        case TypeFlag::BYTE: {
            return type->AsByteType()->GetValue();
        }
        case TypeFlag::CHAR: {
            return type->AsCharType()->GetValue();
        }
        case TypeFlag::SHORT: {
            return type->AsShortType()->GetValue();
        }
        case TypeFlag::INT: {
            return type->AsIntType()->GetValue();
        }
        case TypeFlag::LONG: {
            return type->AsLongType()->GetValue();
        }
        case TypeFlag::FLOAT: {
            return type->AsFloatType()->GetValue();
        }
        case TypeFlag::DOUBLE: {
            return type->AsDoubleType()->GetValue();
        }
        default: {
            UNREACHABLE();
        }
    }
}

template <typename TargetType>
Type *ArithmeticChecker::PerformRelationOperationOnTypes(BinaryOperationArgs args)
{
    using UType = typename TargetType::UType;

    UType leftValue = GetOperand<TargetType>(args.left);
    UType rightValue = GetOperand<TargetType>(args.right);

    bool result {};
    switch (args.operationType) {
        case lexer::TokenType::PUNCTUATOR_LESS_THAN: {
            result = leftValue < rightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LESS_THAN_EQUAL: {
            result = leftValue <= rightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN: {
            result = leftValue > rightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN_EQUAL: {
            result = leftValue >= rightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_EQUAL: {
            result = leftValue == rightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_NOT_EQUAL: {
            result = leftValue != rightValue;
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    return CreateETSBooleanType(result);
}

template <typename TargetType>
Type *ArithmeticChecker::PerformArithmeticOperationOnTypes(BinaryOperationArgs args)
{
    using UType = typename TargetType::UType;

    UType leftValue = GetOperand<TargetType>(args.left);
    UType rightValue = GetOperand<TargetType>(args.right);
    auto result = leftValue;
    auto isForbiddenZeroDivision = [&rightValue]() { return std::is_integral<UType>::value && rightValue == 0; };

    switch (args.operationType) {
        case lexer::TokenType::PUNCTUATOR_PLUS:
        case lexer::TokenType::PUNCTUATOR_PLUS_EQUAL: {
            result = leftValue + rightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_MINUS:
        case lexer::TokenType::PUNCTUATOR_MINUS_EQUAL: {
            result = leftValue - rightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_DIVIDE:
        case lexer::TokenType::PUNCTUATOR_DIVIDE_EQUAL: {
            if (isForbiddenZeroDivision()) {
                return nullptr;
            }
            result = leftValue / rightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_MULTIPLY:
        case lexer::TokenType::PUNCTUATOR_MULTIPLY_EQUAL: {
            result = leftValue * rightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_MOD:
        case lexer::TokenType::PUNCTUATOR_MOD_EQUAL: {
            if (isForbiddenZeroDivision()) {
                return nullptr;
            }
            result = HandleModulo<UType>(leftValue, rightValue);
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    return Allocator()->New<TargetType>(result);
}

template <>
inline IntType::UType ArithmeticChecker::HandleModulo<IntType::UType>(IntType::UType leftValue,
                                                                      IntType::UType rightValue)
{
    ASSERT(rightValue != 0);
    return leftValue % rightValue;
}

template <>
inline LongType::UType ArithmeticChecker::HandleModulo<LongType::UType>(LongType::UType leftValue,
                                                                        LongType::UType rightValue)
{
    ASSERT(rightValue != 0);
    return leftValue % rightValue;
}

template <>
inline FloatType::UType panda::es2panda::checker::ArithmeticChecker::HandleModulo<FloatType::UType>(
    FloatType::UType leftValue, FloatType::UType rightValue)
{
    return std::fmod(leftValue, rightValue);
}

template <>
inline DoubleType::UType panda::es2panda::checker::ArithmeticChecker::HandleModulo<DoubleType::UType>(
    DoubleType::UType leftValue, DoubleType::UType rightValue)
{
    return std::fmod(leftValue, rightValue);
}

template <typename IntegerUType, typename FloatOrIntegerUType>
inline IntegerUType CastIfFloat(FloatOrIntegerUType num)
{
    if constexpr (std::is_floating_point_v<FloatOrIntegerUType>) {
        return CastFloatToInt<FloatOrIntegerUType, IntegerUType>(num);
    } else {
        return num;
    }
}

template <typename FloatOrIntegerType, typename IntegerType>
Type *ArithmeticChecker::HandleBitWiseArithmetic(BinaryOperationArgs args)
{
    using IntegerUType = typename IntegerType::UType;
    using UnsignedUType = std::make_unsigned_t<IntegerUType>;

    UnsignedUType result = 0;
    UnsignedUType unsignedLeftValue = CastIfFloat<IntegerUType>(GetOperand<FloatOrIntegerType>(args.left));
    UnsignedUType unsignedRightValue = CastIfFloat<IntegerUType>(GetOperand<FloatOrIntegerType>(args.right));

    auto mask = std::numeric_limits<UnsignedUType>::digits - 1U;
    auto shift = unsignedRightValue & mask;

    switch (args.operationType) {
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND:
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND_EQUAL: {
            result = unsignedLeftValue & unsignedRightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR:
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR_EQUAL: {
            result = unsignedLeftValue | unsignedRightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR:
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR_EQUAL: {
            result = unsignedLeftValue ^ unsignedRightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT_EQUAL: {
            static_assert(sizeof(UnsignedUType) == 4 || sizeof(UnsignedUType) == 8);
            result = unsignedLeftValue << shift;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT_EQUAL: {
            static_assert(sizeof(IntegerUType) == 4 || sizeof(IntegerUType) == 8);
            result = static_cast<IntegerUType>(unsignedLeftValue) >> shift;  // NOLINT(hicpp-signed-bitwise)
            break;
        }
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT_EQUAL: {
            static_assert(sizeof(UnsignedUType) == 4 || sizeof(UnsignedUType) == 8);
            result = unsignedLeftValue >> shift;
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    return Allocator()->New<IntegerType>(result);
}
}  // namespace panda::es2panda::checker

#endif
