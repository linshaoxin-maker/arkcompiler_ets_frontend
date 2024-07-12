#pragma once

#include "mem/arena_allocator.h"
#include "astBuilder.h"
#include "ir/base/MethodDefinitionBuilder.h"

namespace ark::es2panda::ir {

class MethodDefinitionBuilder : public AstBuilder {
public:
    MethodDefinitionBuilder(ark::ArenaAllocator *allocator) : AstBuilder(allocator)
    {
    }

    MethodDefinitionBuilder &SetMethodDefinitionBuilderKind(MethodDefinitionBuilderKind const kind)
    {
        kind_ = kind;
        return *this;
    }

    MethodDefinitionBuilder &SetKey(Expression *const key)
    {
        key_ = key;
        return *this;
    }

    MethodDefinitionBuilder &SetValue(Expression *const value)
    {
        value_ = value;
        return *this;
    }

    MethodDefinitionBuilder &SetModifierFlags(ModifierFlags const flags)
    {
        flags_ = flags;
        return *this;
    }

    void SetParent(AstNode *const parent)
    {
        parent_ = parent;
    }

    MethodDefinition *Build()
    {
        ir::MethodDefinition *node = AllocNode<ir::MethodDefinition>(kind_, key_, va;ue_, flags_, Allocator(), false);
        node->SetParent(parent_);
        return node;
    }

private:
    MethodDefinitionBuilderKind kind_;
    Expression *key_;
    Expression *value_;
    ModifierFlags flags_ {};
    AstNode *parent_ {};
};

}  // namespace ark::es2panda::ir
