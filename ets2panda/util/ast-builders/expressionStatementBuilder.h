#pragma once

#include "mem/arena_allocator.h"
#include "astBuilder.h"
#include "ir/statements/expressionStatement.h"

namespace ark::es2panda::ir {

class ExpressionStatementBuilder : public AstBuilder {
public:
    ExpressionStatementBuilder(ark::ArenaAllocator *allocator) : AstBuilder(allocator) {}

    ExpressionStatementBuilder &SetExpression(Expression *expr)
    {
        expression_ = expr;
        return *this;
    }

    void SetParent(AstNode *const parent)
    {
        parent_ = parent;
    }

    ExpressionStatement *Build()
    {
        ir::ExpressionStatement *node = AllocNode<ir::ExpressionStatement>(expression_);
        node->SetParent(parent_);
        return node;
    }

private:
    Expression *expression_;
    AstNode *parent_ {};
};

}  // namespace ark::es2panda::ir