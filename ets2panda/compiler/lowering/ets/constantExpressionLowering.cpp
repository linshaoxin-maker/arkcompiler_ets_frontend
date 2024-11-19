/*
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

#include "constantExpressionLowering.h"

#include "checker/ETSchecker.h"
#include "util/errorHandler.h"

#include <lexer/token/token.h>

namespace ark::es2panda::compiler {

void ConstantExpressionLowering::LogSyntaxError(std::string_view errorMessage, const lexer::SourcePosition &pos) const
{
    util::ErrorHandler::LogSyntaxError(context_->parser->ErrorLogger(), program_, errorMessage, pos);
}

static bool IsSupportedConditionalLiteral(ir::Expression *const node)
{
    if (!node->IsLiteral()) {
        return false;
    }

    auto literal = node->AsLiteral();
    return literal->IsNumberLiteral() || literal->IsCharLiteral() || literal->IsBooleanLiteral() ||
           literal->IsStringLiteral() || literal->IsUndefinedLiteral() || literal->IsNullLiteral();
}

static bool IsSupportedLiteral(ir::Expression *const node)
{
    if (!node->IsLiteral()) {
        return false;
    }

    auto literal = node->AsLiteral();
    return literal->IsNumberLiteral() || literal->IsCharLiteral() || literal->IsBooleanLiteral();
}

static bool CheckIsBooleanConstantForUnary(ir::Literal *const unaryLiteral, lexer::TokenType opType)
{
    if (unaryLiteral->IsBooleanLiteral()) {
        return true;
    }
    return opType == lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK;
}

static bool CheckIsBooleanConstantForBinary(ir::Literal *const left, ir::Literal *const right, lexer::TokenType opType)
{
    if (left->IsBooleanLiteral() || right->IsBooleanLiteral()) {
        return true;
    }
    return opType == lexer::TokenType::PUNCTUATOR_GREATER_THAN ||
           opType == lexer::TokenType::PUNCTUATOR_GREATER_THAN_EQUAL ||
           opType == lexer::TokenType::PUNCTUATOR_LESS_THAN || opType == lexer::TokenType::PUNCTUATOR_LESS_THAN_EQUAL ||
           opType == lexer::TokenType::PUNCTUATOR_EQUAL || opType == lexer::TokenType::PUNCTUATOR_NOT_EQUAL ||
           opType == lexer::TokenType::PUNCTUATOR_LOGICAL_AND || opType == lexer::TokenType::PUNCTUATOR_LOGICAL_OR;
}

static bool CheckIsNumericConstant(ir::Literal *const left, ir::Literal *const right)
{
    return (left->IsNumberLiteral() || left->IsCharLiteral()) && (right->IsNumberLiteral() || right->IsCharLiteral());
}

template <typename TargetType>
static TargetType GetOperand(ir::Literal *const node)
{
    if (node->IsBooleanLiteral()) {
        return node->AsBooleanLiteral()->Value();
    }

    if (node->IsNumberLiteral()) {
        auto numNode = node->AsNumberLiteral();
        if (numNode->Number().IsInt()) {
            return numNode->Number().GetInt();
        }
        if (numNode->Number().IsLong()) {
            return numNode->Number().GetLong();
        }
        if (numNode->Number().IsFloat()) {
            return numNode->Number().GetFloat();
        }
        if (numNode->Number().IsDouble()) {
            return numNode->Number().GetDouble();
        }
        UNREACHABLE();
    }

    if (node->IsCharLiteral()) {
        return node->AsCharLiteral()->Char();
    }

    UNREACHABLE();
}

static TypeRank GetTypeRank(ir::Literal *const literal)
{
    if (literal->IsCharLiteral()) {
        return TypeRank::CHAR;
    }
    if (literal->IsNumberLiteral()) {
        auto number = literal->AsNumberLiteral()->Number();
        if (number.IsInt()) {
            return TypeRank::INT32;
        }
        if (number.IsLong()) {
            return TypeRank::INT64;
        }
        if (number.IsDouble()) {
            return TypeRank::DOUBLE;
        }
        return TypeRank::FLOAT;
    }
    UNREACHABLE();
}

static bool TestLiteralIsNotZero(ir::Literal *literal)
{
    assert(literal->IsCharLiteral() || literal->IsNumberLiteral());
    if (literal->IsCharLiteral()) {
        return literal->AsCharLiteral()->Char() != 0;
    }

    auto number = literal->AsNumberLiteral()->Number();
    if (number.IsInt()) {
        return number.GetInt() != 0;
    }
    if (number.IsLong()) {
        return number.GetLong() != 0;
    }
    if (number.IsDouble()) {
        return number.GetDouble() != 0;
    }
    if (number.IsFloat()) {
        return number.GetFloat() != 0;
    }
    UNREACHABLE();
}

ir::AstNode *ConstantExpressionLowering::FoldTernaryConstant(ir::ConditionalExpression *cond)
{
    ir::AstNode *resNode {};

    auto const testCond = cond->Test()->AsLiteral();
    if (testCond->IsBooleanLiteral()) {
        resNode = testCond->AsBooleanLiteral()->Value() ? cond->Consequent() : cond->Alternate();
    }
    // 15.10.1 Extended Conditional Expression
    if (testCond->IsStringLiteral()) {
        resNode = !testCond->AsStringLiteral()->Str().Empty() ? cond->Consequent() : cond->Alternate();
    }
    if (testCond->IsNullLiteral() || testCond->IsUndefinedLiteral()) {
        resNode = cond->Alternate();
    }
    if (testCond->IsCharLiteral() || testCond->IsNumberLiteral()) {
        resNode = TestLiteralIsNotZero(testCond) ? cond->Consequent() : cond->Alternate();
    }

    if (resNode == nullptr) {
        return cond;
    }

    resNode->SetParent(cond->Parent());
    resNode->SetRange(cond->Range());
    return resNode;
}

template <typename InputType>
bool ConstantExpressionLowering::PerformRelationOperator(InputType left, InputType right, lexer::TokenType opType)
{
    switch (opType) {
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN: {
            return left > right;
        }
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN_EQUAL: {
            return left >= right;
        }
        case lexer::TokenType::PUNCTUATOR_LESS_THAN: {
            return left < right;
        }
        case lexer::TokenType::PUNCTUATOR_LESS_THAN_EQUAL: {
            return left <= right;
        }
        case lexer::TokenType::PUNCTUATOR_EQUAL: {
            return left == right;
        }
        case lexer::TokenType::PUNCTUATOR_NOT_EQUAL: {
            return left != right;
        }
        default: {
            UNREACHABLE();
        }
    }
}

bool ConstantExpressionLowering::HandleRelationOperator(ir::Literal *left, ir::Literal *right, lexer::TokenType opType)
{
    if (left->IsBooleanLiteral()) {
        return PerformRelationOperator(GetOperand<bool>(left), GetOperand<bool>(right), opType);
    }

    TypeRank leftRank = GetTypeRank(left);
    TypeRank rightRank = GetTypeRank(right);
    TypeRank targetRank = (leftRank > rightRank) ? leftRank : rightRank;
    switch (targetRank) {
        case TypeRank::DOUBLE: {
            return PerformRelationOperator(GetOperand<double>(left), GetOperand<double>(right), opType);
        }
        case TypeRank::FLOAT: {
            return PerformRelationOperator(GetOperand<float>(left), GetOperand<float>(right), opType);
        }
        case TypeRank::INT64: {
            return PerformRelationOperator(GetOperand<int64_t>(left), GetOperand<int64_t>(right), opType);
        }
        case TypeRank::INT32:
        case TypeRank::CHAR: {
            return PerformRelationOperator(GetOperand<int32_t>(left), GetOperand<int32_t>(right), opType);
        }
        default: {
            UNREACHABLE();
        }
    }
}

ir::AstNode *ConstantExpressionLowering::HandleLogicalOperator(ir::BinaryExpression *concat, lexer::TokenType opType)
{
    auto left = concat->Left();
    auto right = concat->Right();

    bool leftBoolValue = false;
    ir::AstNode *resultValueNode = nullptr;

    if (left->IsBooleanLiteral()) {
        leftBoolValue = left->AsBooleanLiteral()->Value();
    } else if (left->IsNumberLiteral() || left->IsCharLiteral()) {
        leftBoolValue = GetOperand<int32_t>(left->AsLiteral()) != 0;
    } else {
        UNREACHABLE();
    }

    switch (opType) {
        case lexer::TokenType::PUNCTUATOR_LOGICAL_AND: {
            if (!leftBoolValue) {
                resultValueNode = left;
                break;
            }
            resultValueNode = right;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LOGICAL_OR: {
            if (leftBoolValue) {
                resultValueNode = left;
            } else {
                resultValueNode = right;
            }
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    resultValueNode->SetParent(concat->Parent());
    resultValueNode->SetRange({left->Range().start, right->Range().end});
    return resultValueNode;
}

ir::AstNode *ConstantExpressionLowering::FoldBinaryBooleanConstant(ir::BinaryExpression *concat)
{
    auto left = concat->Left()->AsLiteral();
    auto right = concat->Right()->AsLiteral();

    bool result {};
    switch (concat->OperatorType()) {
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN:
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN_EQUAL:
        case lexer::TokenType::PUNCTUATOR_LESS_THAN:
        case lexer::TokenType::PUNCTUATOR_LESS_THAN_EQUAL:
        case lexer::TokenType::PUNCTUATOR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_NOT_EQUAL: {
            if ((left->IsBooleanLiteral() && right->IsBooleanLiteral()) || CheckIsNumericConstant(left, right)) {
                result = HandleRelationOperator(left, right, concat->OperatorType());
                break;
            }
            return concat;
        }
        case lexer::TokenType::PUNCTUATOR_LOGICAL_AND:
        case lexer::TokenType::PUNCTUATOR_LOGICAL_OR: {
            // Special because of extended conditional expression
            return HandleLogicalOperator(concat, concat->OperatorType());
        }
        default: {
            return concat;
        }
    }

    auto resNode = util::NodeAllocator::Alloc<ir::BooleanLiteral>(context_->allocator, result);
    resNode->SetParent(concat->Parent());
    resNode->SetRange({left->Range().start, right->Range().end});
    return resNode;
}

template <typename IntegerType>
IntegerType ConstantExpressionLowering::PerformBitwiseArithmetic(IntegerType left, IntegerType right,
                                                                 lexer::TokenType operationType)
{
    using UnsignedType = std::make_unsigned_t<IntegerType>;

    UnsignedType result = 0;
    UnsignedType unsignedLeftValue = left;
    UnsignedType unsignedRightValue = right;

    auto mask = std::numeric_limits<UnsignedType>::digits - 1U;
    auto shift = unsignedRightValue & mask;

    switch (operationType) {
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND: {
            result = unsignedLeftValue & unsignedRightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR: {
            result = unsignedLeftValue | unsignedRightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR: {
            result = unsignedLeftValue ^ unsignedRightValue;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT: {
            static_assert(sizeof(UnsignedType) == 4 || sizeof(UnsignedType) == 8);
            result = unsignedLeftValue << shift;
            break;
        }
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT: {
            static_assert(sizeof(IntegerType) == 4 || sizeof(IntegerType) == 8);
            result = static_cast<IntegerType>(unsignedLeftValue) >> shift;  // NOLINT(hicpp-signed-bitwise)
            break;
        }
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT: {
            static_assert(sizeof(UnsignedType) == 4 || sizeof(UnsignedType) == 8);
            result = unsignedLeftValue >> shift;
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    return result;
}

template <typename TargetType>
lexer::Number ConstantExpressionLowering::HandleBitwiseOperator(TargetType leftNum, TargetType rightNum,
                                                                lexer::TokenType operationType, TypeRank targetRank)
{
    switch (targetRank) {
        case TypeRank::DOUBLE: {
            return lexer::Number(PerformBitwiseArithmetic<int64_t>(leftNum, rightNum, operationType));
        }
        case TypeRank::FLOAT: {
            return lexer::Number(PerformBitwiseArithmetic<int32_t>(leftNum, rightNum, operationType));
        }
        case TypeRank::INT64: {
            return lexer::Number(PerformBitwiseArithmetic<int64_t>(leftNum, rightNum, operationType));
        }
        case TypeRank::INT32:
        case TypeRank::CHAR: {
            return lexer::Number(PerformBitwiseArithmetic<int32_t>(leftNum, rightNum, operationType));
        }
        default: {
            UNREACHABLE();
        }
    }
}

template <typename TargetType>
TargetType ConstantExpressionLowering::HandleArithmeticOperation(TargetType leftNum, TargetType rightNum,
                                                                 lexer::TokenType operationType)
{
    switch (operationType) {
        case lexer::TokenType::PUNCTUATOR_PLUS: {
            return leftNum + rightNum;
        }
        case lexer::TokenType::PUNCTUATOR_MINUS: {
            return leftNum - rightNum;
        }
        case lexer::TokenType::PUNCTUATOR_DIVIDE: {
            return leftNum / rightNum;
        }
        case lexer::TokenType::PUNCTUATOR_MULTIPLY: {
            return leftNum * rightNum;
        }
        case lexer::TokenType::PUNCTUATOR_MOD: {
            if constexpr (std::is_integral_v<TargetType>) {
                return leftNum % rightNum;
            } else {
                return std::fmod(leftNum, rightNum);
            }
        }
        default:
            UNREACHABLE();
    }
}

template <typename InputType>
ir::AstNode *ConstantExpressionLowering::FoldBinaryNumericConstantHelper(ir::BinaryExpression *concat,
                                                                         TypeRank targetRank)
{
    auto const lhs = concat->Left()->AsLiteral();
    auto const rhs = concat->Right()->AsLiteral();
    lexer::Number resNum {};
    auto lhsNumber = GetOperand<InputType>(lhs);
    auto rhsNumber = GetOperand<InputType>(rhs);
    auto isForbiddenZeroDivision = [&rhsNumber]() { return std::is_integral<InputType>::value && rhsNumber == 0; };
    switch (concat->OperatorType()) {
        case lexer::TokenType::PUNCTUATOR_DIVIDE:
        case lexer::TokenType::PUNCTUATOR_MOD: {
            if (isForbiddenZeroDivision()) {
                LogSyntaxError("Division by zero are not allowed in Enum or Annotation", concat->Start());
                return concat;
            }
            auto num = HandleArithmeticOperation(lhsNumber, rhsNumber, concat->OperatorType());
            resNum = lexer::Number(num);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_PLUS:
        case lexer::TokenType::PUNCTUATOR_MINUS:
        case lexer::TokenType::PUNCTUATOR_MULTIPLY: {
            auto num = HandleArithmeticOperation(lhsNumber, rhsNumber, concat->OperatorType());
            resNum = lexer::Number(num);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR:
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND:
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR: {
            resNum =
                std::move(HandleBitwiseOperator<InputType>(lhsNumber, rhsNumber, concat->OperatorType(), targetRank));
            break;
        }
        default: {
            // Operation might not support.
            return concat;
        }
    }

    ir::TypedAstNode *resNode = util::NodeAllocator::Alloc<ir::NumberLiteral>(context_->allocator, resNum);
    resNode->SetParent(concat->Parent());
    resNode->SetRange({lhs->Range().start, rhs->Range().end});
    return resNode;
}

ir::AstNode *ConstantExpressionLowering::FoldBinaryNumericConstant(ir::BinaryExpression *concat)
{
    auto left = concat->Left()->AsLiteral();
    auto right = concat->Right()->AsLiteral();

    TypeRank leftRank = GetTypeRank(left);
    TypeRank rightRank = GetTypeRank(right);
    TypeRank targetRank = (leftRank > rightRank) ? leftRank : rightRank;
    switch (targetRank) {
        case TypeRank::DOUBLE: {
            return FoldBinaryNumericConstantHelper<double>(concat, targetRank);
        }
        case TypeRank::FLOAT: {
            return FoldBinaryNumericConstantHelper<float>(concat, targetRank);
        }
        case TypeRank::INT64: {
            return FoldBinaryNumericConstantHelper<int64_t>(concat, targetRank);
        }
        case TypeRank::INT32:
        case TypeRank::CHAR: {
            return FoldBinaryNumericConstantHelper<int32_t>(concat, targetRank);
        }
        default: {
            UNREACHABLE();
        }
    }
}

ir::AstNode *ConstantExpressionLowering::FoldBinaryConstant(ir::BinaryExpression *const concat)
{
    auto const lhs = concat->Left()->AsLiteral();
    auto const rhs = concat->Right()->AsLiteral();

    auto isBooleanConstant = CheckIsBooleanConstantForBinary(lhs, rhs, concat->OperatorType());
    if (isBooleanConstant) {
        return FoldBinaryBooleanConstant(concat);
    }

    return FoldBinaryNumericConstant(concat);
}

template <typename InputType>
lexer::Number ConstantExpressionLowering::HandleBitwiseNegate(InputType value, TypeRank rank)
{
    switch (rank) {
        case TypeRank::DOUBLE:
        case TypeRank::INT64: {
            return lexer::Number(static_cast<int64_t>(~static_cast<uint64_t>(value)));
        }
        case TypeRank::FLOAT:
        case TypeRank::INT32:
        case TypeRank::CHAR: {
            return lexer::Number(static_cast<int32_t>(~static_cast<uint32_t>(value)));
        }
        default: {
            UNREACHABLE();
        }
    }
}

template <typename InputType>
ir::AstNode *ConstantExpressionLowering::FoldUnaryNumericConstantHelper(ir::UnaryExpression *unary, ir::Literal *node,
                                                                        TypeRank rank)
{
    auto value = GetOperand<InputType>(node);

    lexer::Number resNum {};
    switch (unary->OperatorType()) {
        case lexer::TokenType::PUNCTUATOR_PLUS: {
            resNum = lexer::Number(+value);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_MINUS: {
            resNum = lexer::Number(-value);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_TILDE: {
            resNum = std::move(HandleBitwiseNegate(value, rank));
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    ir::TypedAstNode *resNode = util::NodeAllocator::Alloc<ir::NumberLiteral>(context_->allocator, resNum);
    resNode->SetParent(unary->Parent());
    resNode->SetRange(unary->Range());
    return resNode;
}

ir::AstNode *ConstantExpressionLowering::FoldUnaryNumericConstant(ir::UnaryExpression *unary)
{
    auto literal = unary->Argument()->AsLiteral();
    TypeRank rank = GetTypeRank(literal);

    switch (rank) {
        case TypeRank::DOUBLE: {
            return FoldUnaryNumericConstantHelper<double>(unary, literal, rank);
        }
        case TypeRank::FLOAT: {
            return FoldUnaryNumericConstantHelper<float>(unary, literal, rank);
        }
        case TypeRank::INT64: {
            return FoldUnaryNumericConstantHelper<int64_t>(unary, literal, rank);
        }
        case TypeRank::INT32:
        case TypeRank::CHAR: {
            return FoldUnaryNumericConstantHelper<int32_t>(unary, literal, rank);
        }
        default: {
            UNREACHABLE();
        }
    }
}

ir::AstNode *ConstantExpressionLowering::FoldUnaryBooleanConstant(ir::UnaryExpression *unary)
{
    bool value = GetOperand<bool>(unary->Argument()->AsLiteral());

    bool result {};
    if (unary->OperatorType() == lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK) {
        result = !value;
    } else {
        UNREACHABLE();
    }

    auto resNode = util::NodeAllocator::Alloc<ir::BooleanLiteral>(context_->allocator, result);
    resNode->SetParent(unary->Parent());
    resNode->SetRange(unary->Range());
    return resNode;
}

ir::AstNode *ConstantExpressionLowering::FoldUnaryConstant(ir::UnaryExpression *const unary)
{
    auto unaryLiteral = unary->Argument()->AsLiteral();

    auto isBooleanConstant = CheckIsBooleanConstantForUnary(unaryLiteral, unary->OperatorType());
    if (isBooleanConstant) {
        return FoldUnaryBooleanConstant(unary);
    }

    return FoldUnaryNumericConstant(unary);
}

ir::AstNode *ConstantExpressionLowering::FoldConstant(ir::AstNode *constantNode)
{
    ir::NodeTransformer handleFoldConstant = [this](ir::AstNode *const node) {
        if (node->IsUnaryExpression()) {
            auto unaryOp = node->AsUnaryExpression();
            if (IsSupportedLiteral(unaryOp->Argument())) {
                return FoldUnaryConstant(unaryOp);
            }
            LogSyntaxError("Only constant expression is expected in the field", node->Start());
        }
        if (node->IsBinaryExpression()) {
            auto binop = node->AsBinaryExpression();
            if (IsSupportedLiteral(binop->Left()) && IsSupportedLiteral(binop->Right())) {
                return FoldBinaryConstant(binop);
            }
            LogSyntaxError("Only constant expression is expected in the field", node->Start());
        }
        if (node->IsConditionalExpression()) {
            auto condExp = node->AsConditionalExpression();
            if (IsSupportedConditionalLiteral(condExp->Test())) {
                return FoldTernaryConstant(condExp);
            }
            LogSyntaxError("Only constant expression is expected in the field", node->Start());
        }
        return node;
    };

    constantNode->TransformChildrenRecursivelyPostorder(handleFoldConstant, Name());

    return constantNode;
}

bool ConstantExpressionLowering::Perform(public_lib::Context *ctx, parser::Program *program)
{
    if (context_ == nullptr) {
        context_ = ctx;
    }
    for (auto &[_, ext_programs] : program->ExternalSources()) {
        (void)_;
        for (auto *extProg : ext_programs) {
            Perform(ctx, extProg);
        }
    }
    program_ = program;
    program->Ast()->TransformChildrenRecursively(
        // CC-OFFNXT(G.FMT.14-CPP) project code style
        [this](ir::AstNode *const node) -> ir::AstNode * {
            if (node->IsAnnotationUsage() || node->IsTSEnumDeclaration() || node->IsAnnotationDeclaration()) {
                return FoldConstant(node);
            }
            return node;
        },
        Name());

    return true;
}

}  // namespace ark::es2panda::compiler
