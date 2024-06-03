#pragma once

#include "mem/arena_allocator.h"
#include "astBuilder.h"
#include "ir/expressions/binaryExpression.h"
#include "ir/expressions/literals/numberLiteral.h"

namespace ark::es2panda::ir {

class BinaryExpressionBuilder : public AstBuilder {
public:
    BinaryExpressionBuilder(ark::ArenaAllocator *allocator)
        : AstBuilder(allocator), left_(nullptr), right_(nullptr), operator_(lexer::TokenType::PUNCTUATOR_PLUS)
    {
        // Defaults
        left_ = AllocNode<ir::NumberLiteral>(lexer::Number(10));
        right_ = AllocNode<ir::NumberLiteral>(lexer::Number(20));
    }

    BinaryExpressionBuilder &setOperator(lexer::TokenType op)
    {
        operator_ = op;
        return *this;
    }

    BinaryExpressionBuilder &setLeft(Expression *left)
    {
        left_ = left;
        return *this;
    }

    BinaryExpressionBuilder &setRight(Expression *right)
    {
        right_ = right;
        return *this;
    }

    BinaryExpression *build()
    {
        return AllocNode<ir::BinaryExpression>(left_, right_, operator_);
    }

private:
    Expression *left_;
    Expression *right_;
    lexer::TokenType operator_;
};

}  // namespace ark::es2panda::ir
