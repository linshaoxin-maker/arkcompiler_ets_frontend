#pragma once

#include "mem/arena_allocator.h"
#include "astBuilder.h"
#include "ir/expressions/literals/stringLiteral.h"

namespace ark::es2panda::ir {

class StringLiteralBuilder : public AstBuilder {
public:
    StringLiteralBuilder(ark::ArenaAllocator *allocator) : AstBuilder(allocator)
    {
    }

    StringLiteralBuilder &SetValue(util::StringView value) {
        value_ = value;
        return *this;
    }

    void SetParent(AstNode *const parent)
    {
        parent_ = parent;
    }

    ir::StringLiteral *Build()
    {
        ir::StringLiteral *node = AllocNode<ir::StringLiteral>(value_);
        node->SetParent(parent_); 
        return node;
    }

private:
    util::StringView value_;
    AstNode *parent_ {};
};

}  // namespace ark::es2panda::ir
