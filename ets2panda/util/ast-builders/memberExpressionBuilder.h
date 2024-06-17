#pragma once

#include "ir/expressions/memberExpression.h"
#include "mem/arena_allocator.h"
#include "astBuilder.h"

namespace ark::es2panda::ir {

class MemberExpressionBuilder : public AstBuilder {
public:
    MemberExpressionBuilder(ark::ArenaAllocator *allocator)
        : AstBuilder(allocator)
    {
    }

    void SetParent(AstNode *const parent)
    {
        parent_ = parent;
    }

    MemberExpressionBuilder &SetObject(Expression *obj)
    {
        object_ = obj;
        return *this;
    }

    MemberExpressionBuilder &SetKind(MemberExpressionKind kind)
    {
        kind_ = kind;
        return *this;
    }

    MemberExpression *Build()
    {
        auto etsTypeReference = AllocNode<ir::MemberExpression>(object_, property_, kind_, false, false);
        return etsTypeReference;
    }

private:
    Expression *object_ = nullptr;
    Expression *property_ = nullptr;
    MemberExpressionKind kind_;
    AstNode *parent_ {};
};

}  // namespace ark::es2panda::ir
