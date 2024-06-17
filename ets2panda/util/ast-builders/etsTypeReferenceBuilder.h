#pragma once

#include "ir/base/classDefinition.h"
#include "mem/arena_allocator.h"
#include "astBuilder.h"

namespace ark::es2panda::ir {

class ETSTypeReferenceBuilder : public AstBuilder {
public:
    ETSTypeReferenceBuilder(ark::ArenaAllocator *allocator)
        : AstBuilder(allocator)
    {
    }

    ETSTypeReferenceBuilder &SetETSTypeReferencePart(ETSTypeReferencePart *typeReferencePart)
    {
        etsTypeReferencePart_ = typeReferencePart;
        return *this;
    }

    void SetParent(AstNode *const parent)
    {
        parent_ = parent;
    }

    ETSTypeReference *Build()
    {
        auto etsTypeReference = AllocNode<ir::ETSTypeReference>(etsTypeReferencePart_);
        return etsTypeReference;
    }

private:
    Identifier *ident_;
    ArenaVector<AstNode *> body_;
    ETSTypeReferencePart *etsTypeReferencePart_;
    AstNode *parent_ {};
};

}  // namespace ark::es2panda::ir
