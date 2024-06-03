#pragma once

#include "ir/base/classDefinition.h"
#include "mem/arena_allocator.h"
#include "astBuilder.h"

namespace ark::es2panda::ir {

class ClassDefinitionBuilder : public AstBuilder {
public:
    ClassDefinitionBuilder(ark::ArenaAllocator *allocator)
        : AstBuilder(allocator), ident_(nullptr), ctor_(nullptr), superClass_(nullptr), body_(Allocator()->Adapter())
    {
    }

    ClassDefinitionBuilder &SetIdentifier(util::StringView id)
    {
        ident_ = AllocNode<ir::Identifier>(id, Allocator());
        return *this;
    }

    ClassDefinitionBuilder &SetConstructor(MethodDefinition *ctor)
    {
        ctor_ = ctor;
        return *this;
    }

    ClassDefinitionBuilder &SetSuperClass(Expression *superClass)
    {
        superClass_ = superClass;
        return *this;
    }

    ClassDefinitionBuilder &AddProperty(AstNode *property)
    {
        body_.push_back(property);
        return *this;
    }

    ClassDefinition *Build()
    {
        // TODO: use full constructor
        auto classDef = AllocNode<ir::ClassDefinition>(Allocator(), ident_, ir::ClassDefinitionModifiers::CLASS_DECL,
                                                       ir::ModifierFlags::NONE, Language(Language::Id::ETS));
        classDef->AddProperties(std::move(body_));
        return classDef;
    }

private:
    util::StringView privateId_;
    Identifier *ident_;
    MethodDefinition *ctor_;
    Expression *superClass_;
    ArenaVector<AstNode *> body_;
};

}  // namespace ark::es2panda::ir
