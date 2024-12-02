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

#ifndef ES2PANDA_COMPILER_LOWERING_CONSTANT_EXPRESSION_LOWERING_H
#define ES2PANDA_COMPILER_LOWERING_CONSTANT_EXPRESSION_LOWERING_H

#include "compiler/lowering/phase.h"

namespace ark::es2panda::compiler {

enum class TypeRank {
    // Keep this order
    CHAR,
    INT32,
    INT64,
    FLOAT,
    DOUBLE
};

class ConstantExpressionLowering : public Phase {
public:
    std::string_view Name() const override
    {
        return "ConstantExpressionLowering";
    }

    bool Perform(public_lib::Context *ctx, parser::Program *program) override;

private:
    void LogSyntaxError(std::string_view errorMessage, const lexer::SourcePosition &pos) const;

    ir::AstNode *FoldTernaryConstant(ir::ConditionalExpression *cond);

    template <typename InputType>
    bool PerformRelationOperator(InputType left, InputType right, lexer::TokenType opType);

    bool HandleRelationOperator(ir::Literal *left, ir::Literal *right, lexer::TokenType opType);

    ir::AstNode *HandleLogicalOperator(ir::BinaryExpression *concat, lexer::TokenType opType);

    ir::AstNode *FoldBinaryBooleanConstant(ir::BinaryExpression *concat);

    template <typename IntegerType>
    IntegerType PerformBitwiseArithmetic(IntegerType left, IntegerType right, lexer::TokenType operationType);

    template <typename TargetType>
    lexer::Number HandleBitwiseOperator(TargetType leftNum, TargetType rightNum, lexer::TokenType operationType,
                                        TypeRank targetRank);

    template <typename TargetType>
    TargetType HandleArithmeticOperation(TargetType leftNum, TargetType rightNum, lexer::TokenType operationType);

    template <typename InputType>
    ir::AstNode *FoldBinaryNumericConstantHelper(ir::BinaryExpression *concat, TypeRank targetRank);

    ir::AstNode *FoldBinaryNumericConstant(ir::BinaryExpression *concat);

    ir::AstNode *FoldBinaryConstant(ir::BinaryExpression *concat);

    template <typename InputType>
    lexer::Number HandleBitwiseNegate(InputType value, TypeRank rank);

    template <typename InputType>
    ir::AstNode *FoldUnaryNumericConstantHelper(ir::UnaryExpression *unary, ir::Literal *node, TypeRank rank);

    ir::AstNode *FoldUnaryNumericConstant(ir::UnaryExpression *unary);

    ir::AstNode *FoldUnaryBooleanConstant(ir::UnaryExpression *unary);

    ir::AstNode *FoldUnaryConstant(ir::UnaryExpression *unary);

    ir::AstNode *FoldConstant(ir::AstNode *constantNode);

    public_lib::Context *context_ {nullptr};
    parser::Program *program_ {nullptr};
};

}  // namespace ark::es2panda::compiler

#endif  // ES2PANDA_COMPILER_LOWERING_CONSTANT_EXPRESSION_LOWERING_H
