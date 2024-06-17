#pragma once

#include "ir/expressions/thisExpression.h"
#include "mem/arena_allocator.h"
#include "astBuilder.h"

namespace ark::es2panda::ir {

class ThisExpressionBuilder : public AstBuilder {
public:
    ThisExpressionBuilder(ark::ArenaAllocator *allocator)
        : AstBuilder(allocator)
    {
    }

    void SetParent(AstNode *const parent)
    {
        parent_ = parent;
    }

    ThisExpression *Build()
    {
        auto etsTypeReference = AllocNode<ir::ThisExpression>();
        return etsTypeReference;
    }

private:
    AstNode *parent_ {};
};

}  // namespace ark::es2panda::ir
