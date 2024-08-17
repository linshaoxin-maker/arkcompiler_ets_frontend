/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ES2PANDA_COMPILER_SCOPES_DECLARATION_H
#define ES2PANDA_COMPILER_SCOPES_DECLARATION_H

#include "varbinder/variableFlags.h"
#include "macros.h"
#include "util/ustring.h"

namespace ark::es2panda::ir {
class AstNode;
class ScriptFunction;
class TSInterfaceDeclaration;
class ImportDeclaration;
class ETSImportDeclaration;
}  // namespace ark::es2panda::ir

namespace ark::es2panda::varbinder {
class Scope;
class LocalScope;
class LetDecl;
class ConstDecl;
class ParameterDecl;

class Decl {
public:
    virtual ~Decl() = default;
    NO_COPY_SEMANTIC(Decl);
    NO_MOVE_SEMANTIC(Decl);

    virtual DeclType Type() const = 0;

    const util::StringView &Name() const
    {
        return name_;
    }

    ir::AstNode *Node()
    {
        return node_;
    }

    const ir::AstNode *Node() const
    {
        return node_;
    }

    template <class T>
    bool Is() const
    {
        return Type() == T::TYPE;
    }

    template <class T>
    const T *As() const
    {
        ASSERT(Is<T>());
        return reinterpret_cast<const T *>(this);
    }

    template <class T>
    T *As()
    {
        return const_cast<T *>(const_cast<const Decl *>(this)->As<T>());
    }

    void BindNode(ir::AstNode *node)
    {
        node_ = node;
    }

    bool IsLetOrConstDecl() const
    {
        return Is<LetDecl>() || Is<ConstDecl>();
    }

    bool PossibleTDZ() const
    {
        return IsLetOrConstDecl() || Is<ParameterDecl>();
    }

protected:
    explicit Decl(util::StringView name) : name_(name) {}
    explicit Decl(util::StringView name, ir::AstNode *declNode) : name_(name), node_(declNode) {}

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    util::StringView name_;
    ir::AstNode *node_ {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

template <DeclType D>
class TypedDecl : public Decl {
public:
    static constexpr DeclType TYPE = D;

    DeclType Type() const override
    {
        return TYPE;
    }

protected:
    using Decl::Decl;
};

template <typename T, DeclType D>
class MultiDecl : public TypedDecl<D> {
public:
    explicit MultiDecl(ArenaAllocator *allocator, util::StringView name)
        : MultiDecl::TypedDecl(name), declarations_(allocator->Adapter())
    {
    }

    explicit MultiDecl(ArenaAllocator *allocator, util::StringView name, ir::AstNode *declNode)
        : MultiDecl::TypedDecl(name, declNode), declarations_(allocator->Adapter())
    {
    }

    const ArenaVector<T *> &Decls() const
    {
        return declarations_;
    }

    void Add(T *decl)
    {
        declarations_.push_back(decl);
    }

private:
    ArenaVector<T *> declarations_;
};

class EnumLiteralDecl : public TypedDecl<DeclType::ENUM_LITERAL> {
public:
    explicit EnumLiteralDecl(util::StringView name, bool isConst) : TypedDecl(name), isConst_(isConst) {}
    explicit EnumLiteralDecl(util::StringView name, ir::AstNode *declNode, bool isConst)
        : TypedDecl(name, declNode), isConst_(isConst)
    {
    }

    bool IsConst() const
    {
        return isConst_;
    }

    void BindScope(LocalScope *scope)
    {
        scope_ = scope;
    }

    LocalScope *Scope()
    {
        return scope_;
    }

private:
    LocalScope *scope_ {};
    bool isConst_ {};
};

class InterfaceDecl : public MultiDecl<ir::TSInterfaceDeclaration, DeclType::INTERFACE> {
public:
    explicit InterfaceDecl(ArenaAllocator *allocator, util::StringView name) : MultiDecl(allocator, name) {}
    explicit InterfaceDecl(ArenaAllocator *allocator, util::StringView name, ir::AstNode *declNode)
        : MultiDecl(allocator, name, declNode)
    {
    }
};

class ClassDecl : public TypedDecl<DeclType::CLASS> {
public:
    explicit ClassDecl(util::StringView name) : TypedDecl(name) {}
    explicit ClassDecl(util::StringView name, ir::AstNode *node) : TypedDecl(name, node) {}
};

class FunctionDecl : public MultiDecl<ir::ScriptFunction, DeclType::FUNC> {
public:
    explicit FunctionDecl(ArenaAllocator *allocator, util::StringView name, ir::AstNode *node)
        : MultiDecl(allocator, name)
    {
        node_ = node;
    }
};

class TypeParameterDecl : public TypedDecl<DeclType::TYPE_PARAMETER> {
public:
    explicit TypeParameterDecl(util::StringView name) : TypedDecl(name) {}
};

class PropertyDecl : public TypedDecl<DeclType::PROPERTY> {
public:
    explicit PropertyDecl(util::StringView name) : TypedDecl(name) {}
};

class MethodDecl : public TypedDecl<DeclType::METHOD> {
public:
    explicit MethodDecl(util::StringView name) : TypedDecl(name) {}
};

class EnumDecl : public TypedDecl<DeclType::ENUM> {
public:
    explicit EnumDecl(util::StringView name) : TypedDecl(name) {}
};

class TypeAliasDecl : public TypedDecl<DeclType::TYPE_ALIAS> {
public:
    explicit TypeAliasDecl(util::StringView name) : TypedDecl(name) {}
    explicit TypeAliasDecl(util::StringView name, ir::AstNode *node) : TypedDecl(name, node) {}
};

class NameSpaceDecl : public TypedDecl<DeclType::NAMESPACE> {
public:
    explicit NameSpaceDecl(util::StringView name) : TypedDecl(name) {}
};

class VarDecl : public TypedDecl<DeclType::VAR> {
public:
    explicit VarDecl(util::StringView name) : TypedDecl(name) {}
};

class LetDecl : public TypedDecl<DeclType::LET> {
public:
    explicit LetDecl(util::StringView name) : TypedDecl(name) {}
    explicit LetDecl(util::StringView name, ir::AstNode *declNode) : TypedDecl(name, declNode) {}
};

class ConstDecl : public TypedDecl<DeclType::CONST> {
public:
    explicit ConstDecl(util::StringView name) : TypedDecl(name) {}
    explicit ConstDecl(util::StringView name, ir::AstNode *declNode) : TypedDecl(name, declNode) {}
};

class LabelDecl : public TypedDecl<DeclType::LABEL> {
public:
    explicit LabelDecl(util::StringView name) : TypedDecl(name) {}
    explicit LabelDecl(util::StringView name, ir::AstNode *declNode) : TypedDecl(name, declNode) {}
};

class ReadonlyDecl : public TypedDecl<DeclType::READONLY> {
public:
    explicit ReadonlyDecl(util::StringView name) : TypedDecl(name) {}
    explicit ReadonlyDecl(util::StringView name, ir::AstNode *declNode) : TypedDecl(name, declNode) {}
};

class ParameterDecl : public TypedDecl<DeclType::PARAM> {
public:
    explicit ParameterDecl(util::StringView name) : TypedDecl(name) {}
};

class ImportDecl : public TypedDecl<DeclType::IMPORT> {
public:
    explicit ImportDecl(util::StringView importName, util::StringView localName)
        : TypedDecl(localName), importName_(importName)
    {
    }

    explicit ImportDecl(util::StringView importName, util::StringView localName, ir::AstNode *node)
        : TypedDecl(localName), importName_(importName)
    {
        BindNode(node);
    }

    const util::StringView &ImportName() const
    {
        return importName_;
    }

    const util::StringView &LocalName() const
    {
        return name_;
    }

private:
    util::StringView importName_;
};

class ExportDecl : public TypedDecl<DeclType::EXPORT> {
public:
    explicit ExportDecl(util::StringView exportName, util::StringView localName)
        : TypedDecl(localName), exportName_(exportName)
    {
    }

    explicit ExportDecl(util::StringView exportName, util::StringView localName, ir::AstNode *node)
        : TypedDecl(localName), exportName_(exportName)
    {
        BindNode(node);
    }

    const util::StringView &ExportName() const
    {
        return exportName_;
    }

    const util::StringView &LocalName() const
    {
        return name_;
    }

private:
    util::StringView exportName_;
};
}  // namespace ark::es2panda::varbinder

#endif
