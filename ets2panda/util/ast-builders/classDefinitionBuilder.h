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

    void SetParent(AstNode *const parent)
    {
        parent_ = parent;
    }

    ClassDefinitionBuilder &SetTSTypeParameterDeclaration(TSTypeParameterDeclaration *typeParams)
    {
        typeParams_ = typeParams;
        return *this;
    }

    ClassDefinitionBuilder &SetTSTypeParameterInstantiation(TSTypeParameterInstantiation *superTypeParams)
    {
        superTypeParams_ = superTypeParams;
        return *this;
    }

    ClassDefinitionBuilder &SetImplements(ArenaVector<TSClassImplements *> &&implements)
    {
        implements_ = std::move(implements);
        return *this;
    }

    ClassDefinitionBuilder &SetImplements(TSClassImplements *implement)
    {
        implements_.emplace_back(implement);
        return *this;
    }

    ClassDefinition *Build()
    {
        // TODO: use full constructor
        // auto classDef = AllocNode<ir::ClassDefinition>(Allocator(), ident_, ir::ClassDefinitionModifiers::CLASS_DECL,
        //                                                ir::ModifierFlags::NONE, Language(Language::Id::ETS));
        auto classDef = AllocNode<ClassDefinition>(util::StringView(), ident_,
                             typeParams_, superTypeParams_,
                             std::move(implements_), ctor_,
                             superClass_, std::move(body_), ir::ClassDefinitionModifiers::CLASS_DECL,
                             ir::ModifierFlags::NONE, Language(Language::Id::ETS));
        classDef->SetParent(parent_);
        return classDef;
    }

private:
    util::StringView privateId_ {};
    Identifier *ident_ {};
    MethodDefinition *ctor_ {};
    Expression *superClass_ {};
    ArenaVector<AstNode *> body_;
    TSTypeParameterDeclaration *typeParams_ {};
    TSTypeParameterInstantiation *superTypeParams_ {};
    ArenaVector<TSClassImplements *> implements_;
    AstNode *parent_ {};
};

}  // namespace ark::es2panda::ir
