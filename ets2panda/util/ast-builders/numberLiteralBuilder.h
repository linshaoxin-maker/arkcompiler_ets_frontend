#pragma once

#include "mem/arena_allocator.h"
#include "astBuilder.h"
#include "ir/expressions/literals/numberLiteral.h"

namespace ark::es2panda::ir {

// TODO(psaykerone): Make inherit from LiteralBuilder, for stringliteral and numberliteral
class NumberLiteralBuilder : public AstBuilder {
public:
    NumberLiteralBuilder(ark::ArenaAllocator *allocator) : AstBuilder(allocator)
    {
    }

    NumberLiteralBuilder &SetValue(util::StringView value) {
        value_ = value;
        return *this;
    }

    void SetParent(AstNode *const parent)
    {
        parent_ = parent;
    }
    
    NumberLiteral *Build()
    {
        NumberLiteral *node = AllocNode<ir::NumberLiteral>(value_);
        node->SetParent(parent_);
        return node;
    }

private:
    util::StringView value_;
    AstNode *parent_ {};
};

}  // namespace ark::es2panda::ir
