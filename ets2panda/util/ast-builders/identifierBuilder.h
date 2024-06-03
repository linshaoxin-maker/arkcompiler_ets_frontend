#pragma once

#include "mem/arena_allocator.h"
#include "astBuilder.h"
#include "ir/expressions/identifier.h"

namespace ark::es2panda::ir {

class IdentifierBuilder : public AstBuilder {
public:
    IdentifierBuilder(ark::ArenaAllocator *allocator) : AstBuilder(allocator) {}

    IdentifierBuilder &SetName(util::StringView name)
    {
        name_ = name;
        return *this;
    }

    Identifier *Build()
    {
        return AllocNode<ir::Identifier>(name_, Allocator());
    }

private:
    util::StringView name_;
};

}  // namespace ark::es2panda::ir
