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

    void SetParent(AstNode *const parent)
    {
        parent_ = parent;
    }

    Identifier *Build()
    {
        ir::Identifier *node = AllocNode<ir::Identifier>(name_, Allocator());
        node->SetParent(parent_);
        return node;
    }

private:
    util::StringView name_;
    AstNode *parent_ {};
};

}  // namespace ark::es2panda::ir
