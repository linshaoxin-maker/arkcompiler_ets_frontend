#pragma once

#include "mem/arena_allocator.h"
#include "astBuilder.h"
#include "ir/expressions/literals/numberLiteral.h"

namespace ark::es2panda::ir {

class NumberLiteralBuilder : public AstBuilder {
public:
    NumberLiteralBuilder(ark::ArenaAllocator *allocator) : AstBuilder(allocator)
    {
    }

    NumberLiteralBuilder &SetValue(util::StringView value) {
        value_ = value;
        return *this;
    }

    NumberLiteral *Build()
    {
        return AllocNode<ir::NumberLiteral>(value_);
    }

private:
    util::StringView value_;
};

}  // namespace ark::es2panda::ir
