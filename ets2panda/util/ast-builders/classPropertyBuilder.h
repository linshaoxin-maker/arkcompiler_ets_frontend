#pragma once

#include "mem/arena_allocator.h"
#include "astBuilder.h"
#include "ir/base/classProperty.h"

namespace ark::es2panda::ir {

class ClassPropertyBuilder : public AstBuilder {
public:
    ClassPropertyBuilder(ark::ArenaAllocator *allocator) : AstBuilder(allocator), modifiers_(ModifierFlags::NONE) {}

    ClassPropertyBuilder &SetKey(Expression *key)
    {
        key_ = key;
        return *this;
    }

    ClassPropertyBuilder &SetValue(Expression *value)
    {
        value_ = value;
        return *this;
    }

    ClassPropertyBuilder &AddModifier(ModifierFlags modifier)
    {
        modifiers_ = modifiers_ | modifier;
        return *this;
    }

    ClassProperty *Build()
    {
        return AllocNode<ir::ClassProperty>(key_, value_, nullptr, modifiers_, Allocator(), false);
    }

private:
    Expression *key_;
    Expression *value_;
    ModifierFlags modifiers_;
};

}  // namespace ark::es2panda::ir
