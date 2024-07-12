#pragma once

#include "ir/base/classDefinition.h"
#include "mem/arena_allocator.h"
#include "astBuilder.h"

namespace ark::es2panda::ir {

class ETSTypeReferencePartBuilder : public AstBuilder {
public:
    ETSTypeReferencePartBuilder(ark::ArenaAllocator *allocator)
        : AstBuilder(allocator)
    {
    }

    ETSTypeReferencePartBuilder &SetName(Expression *name)
    {
        name_ = name;
        return *this;
    }

    ETSTypeReferencePartBuilder &SetTypeParams(TSTypeParameterInstantiation *typeParams)
    {
        typeParams_ = typeParams;
        return *this;
    }

    ETSTypeReferencePartBuilder &SetPrev(ETSTypeReferencePart *prev)
    {
        prev_ = prev;
        return *this;
    }

    void SetParent(AstNode *const parent)
    {
        parent_ = parent;
    }

    ETSTypeReferencePart *Build()
    {
        auto etsTypeReference = AllocNode<ir::ETSTypeReferencePart>(name_, typeParams_, prev_);
        return etsTypeReference;
    }

private:
    ir::Expression *name_;
    ir::TSTypeParameterInstantiation *typeParams_ {};
    ir::ETSTypeReferencePart *prev_ {};
    AstNode *parent_ {};
};

}  // namespace ark::es2panda::ir
