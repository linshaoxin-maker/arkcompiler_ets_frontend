#pragma once

#include "mem/arena_allocator.h"
#include "astBuilder.h"
#include "ir/expressions/callExpression.h"

namespace ark::es2panda::ir {

class CallExpressionBuilder : public AstBuilder {
public:
    CallExpressionBuilder(ark::ArenaAllocator *allocator) : AstBuilder(allocator)
    {
    }

    CallExpressionBuilder &SetCallee(Expression *const callee)
    {
        callee_ = callee;
        return *this;
    }

    CallExpressionBuilder &SetArguments(ArenaVector<Expression *> &&arguments)
    {
        arguments_ = std::move(arguments);
        return *this;
    }

    CallExpressionBuilder &SetArguments(Expression *arguments)
    {
        arguments_.emplace_back(arguments);
        return *this;
    }

    void SetParent(AstNode *const parent)
    {
        parent_ = parent;
    }

    CallExpression *Build()
    {
        ir::CallExpression *node = AllocNode<ir::CallExpression>(callee_, arguments_, nullptr, false);
        node->SetParent(parent_);
        return node;
    }

private:
    Expression *callee_;
    ArenaVector<Expression *> arguments_;
    AstNode *parent_ {};
};

}  // namespace ark::es2panda::ir
