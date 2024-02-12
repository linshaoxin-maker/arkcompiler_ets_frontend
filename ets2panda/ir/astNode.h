/**
 * Copyright (c) 2021 - 2023 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_IR_AST_NODE_H
#define ES2PANDA_IR_AST_NODE_H

#include "ir/astNodeFlags.h"
#include "ir/astNodeMapping.h"
#include "ir/visitor/AstVisitor.h"
#include "lexer/token/sourceLocation.h"
#include "util/enumbitops.h"

#include <functional>
#include "macros.h"

namespace ark::es2panda::compiler {
class PandaGen;
class ETSGen;
}  // namespace ark::es2panda::compiler

namespace ark::es2panda::checker {
class TSChecker;
class ETSChecker;
class Type;
}  // namespace ark::es2panda::checker

namespace ark::es2panda::varbinder {
class Variable;
class Scope;
}  // namespace ark::es2panda::varbinder

namespace ark::es2panda::ir {
// NOLINTBEGIN(modernize-avoid-c-arrays)
inline constexpr char const CLONE_ALLOCATION_ERROR[] = "Unsuccessful allocation during cloning.";
// NOLINTEND(modernize-avoid-c-arrays)

class AstNode;
class TypeNode;

using NodeTransformer = std::function<AstNode *(AstNode *)>;
using NodeTraverser = std::function<void(AstNode *)>;
using NodePredicate = std::function<bool(AstNode *)>;

enum class AstNodeType {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_NODE_TYPES(nodeType, className) nodeType,
    AST_NODE_MAPPING(DECLARE_NODE_TYPES)
#undef DECLARE_NODE_TYPES
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_NODE_TYPES(nodeType1, nodeType2, baseClass, reinterpretClass) nodeType1, nodeType2,
        AST_NODE_REINTERPRET_MAPPING(DECLARE_NODE_TYPES)
#undef DECLARE_NODE_TYPES
};

DEFINE_BITOPS(AstNodeFlags)

DEFINE_BITOPS(ModifierFlags)

DEFINE_BITOPS(ScriptFunctionFlags)

DEFINE_BITOPS(BoxingUnboxingFlags)

// Forward declarations
class AstDumper;
class Expression;
class SrcDumper;
class Statement;
class ClassElement;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_CLASSES(nodeType, className) class className;
AST_NODE_MAPPING(DECLARE_CLASSES)
#undef DECLARE_CLASSES

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_CLASSES(nodeType1, nodeType2, baseClass, reinterpretClass) class baseClass;
AST_NODE_REINTERPRET_MAPPING(DECLARE_CLASSES)
#undef DECLARE_CLASSES

class AstNode {
public:
    explicit AstNode(AstNodeType type) : type_(type) {};
    explicit AstNode(AstNodeType type, ModifierFlags flags) : type_(type), flags_(flags) {};
    virtual ~AstNode() = default;

    AstNode() = delete;
    NO_COPY_OPERATOR(AstNode);
    NO_MOVE_SEMANTIC(AstNode);

    bool IsProgram() const
    {
        return parent_ == nullptr;
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_IS_CHECKS(nodeType, className) \
    bool Is##className() const                 \
    {                                          \
        return type_ == AstNodeType::nodeType; \
    }
    AST_NODE_MAPPING(DECLARE_IS_CHECKS)
#undef DECLARE_IS_CHECKS

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_IS_CHECKS(nodeType1, nodeType2, baseClass, reinterpretClass) \
    bool Is##baseClass() const                                               \
    {                                                                        \
        return type_ == AstNodeType::nodeType1;                              \
    }                                                                        \
    bool Is##reinterpretClass() const                                        \
    {                                                                        \
        return type_ == AstNodeType::nodeType2;                              \
    }
    AST_NODE_REINTERPRET_MAPPING(DECLARE_IS_CHECKS)
#undef DECLARE_IS_CHECKS

    [[nodiscard]] virtual bool IsStatement() const noexcept
    {
        return false;
    }

    [[nodiscard]] virtual bool IsExpression() const noexcept
    {
        return false;
    }

    virtual bool IsTyped() const
    {
        return false;
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_AS_CASTS(nodeType, className)             \
    className *As##className()                            \
    {                                                     \
        ASSERT(Is##className());                          \
        return reinterpret_cast<className *>(this);       \
    }                                                     \
    const className *As##className() const                \
    {                                                     \
        ASSERT(Is##className());                          \
        return reinterpret_cast<const className *>(this); \
    }
    AST_NODE_MAPPING(DECLARE_AS_CASTS)
#undef DECLARE_AS_CASTS

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_AS_CASTS(nodeType1, nodeType2, baseClass, reinterpretClass) \
    baseClass *As##baseClass()                                              \
    {                                                                       \
        ASSERT(Is##baseClass());                                            \
        return reinterpret_cast<baseClass *>(this);                         \
    }                                                                       \
    baseClass *As##reinterpretClass()                                       \
    {                                                                       \
        ASSERT(Is##reinterpretClass());                                     \
        return reinterpret_cast<baseClass *>(this);                         \
    }                                                                       \
    const baseClass *As##baseClass() const                                  \
    {                                                                       \
        ASSERT(Is##baseClass());                                            \
        return reinterpret_cast<const baseClass *>(this);                   \
    }                                                                       \
    const baseClass *As##reinterpretClass() const                           \
    {                                                                       \
        ASSERT(Is##reinterpretClass());                                     \
        return reinterpret_cast<const baseClass *>(this);                   \
    }
    AST_NODE_REINTERPRET_MAPPING(DECLARE_AS_CASTS)
#undef DECLARE_AS_CASTS

    Expression *AsExpression()
    {
        ASSERT(IsExpression());
        return reinterpret_cast<Expression *>(this);
    }

    const Expression *AsExpression() const
    {
        ASSERT(IsExpression());
        return reinterpret_cast<const Expression *>(this);
    }

    Statement *AsStatement()
    {
        ASSERT(IsStatement());
        return reinterpret_cast<Statement *>(this);
    }

    const Statement *AsStatement() const
    {
        ASSERT(IsStatement());
        return reinterpret_cast<const Statement *>(this);
    }

    void SetRange(const lexer::SourceRange &loc) noexcept
    {
        range_ = loc;
    }

    void SetStart(const lexer::SourcePosition &start) noexcept
    {
        range_.start = start;
    }

    void SetEnd(const lexer::SourcePosition &end) noexcept
    {
        range_.end = end;
    }

    [[nodiscard]] const lexer::SourcePosition &Start() const noexcept
    {
        return range_.start;
    }

    [[nodiscard]] const lexer::SourcePosition &End() const noexcept
    {
        return range_.end;
    }

    [[nodiscard]] const lexer::SourceRange &Range() const noexcept
    {
        return range_;
    }

    [[nodiscard]] AstNodeType Type() const noexcept
    {
        return type_;
    }

    [[nodiscard]] AstNode *Parent() noexcept
    {
        return parent_;
    }

    [[nodiscard]] const AstNode *Parent() const noexcept
    {
        return parent_;
    }

    void SetParent(AstNode *const parent) noexcept
    {
        parent_ = parent;
    }

    [[nodiscard]] varbinder::Variable *Variable() const noexcept
    {
        return variable_;
    }

    void SetVariable(varbinder::Variable *const variable) noexcept
    {
        variable_ = variable;
    }

    // When no decorators are allowed, we cannot return a reference to an empty vector.
    virtual const ArenaVector<ir::Decorator *> *DecoratorsPtr() const
    {
        return nullptr;
    }

    virtual void AddDecorators([[maybe_unused]] ArenaVector<ir::Decorator *> &&decorators)
    {
        UNREACHABLE();
    }

    virtual bool CanHaveDecorator([[maybe_unused]] bool inTs) const
    {
        return false;
    }

    [[nodiscard]] bool IsReadonly() const noexcept
    {
        return (flags_ & ModifierFlags::READONLY) != 0;
    }

    [[nodiscard]] bool IsOptionalDeclaration() const noexcept
    {
        return (flags_ & ModifierFlags::OPTIONAL) != 0;
    }

    [[nodiscard]] bool IsDefinite() const noexcept
    {
        return (flags_ & ModifierFlags::DEFINITE) != 0;
    }

    [[nodiscard]] bool IsConstructor() const noexcept
    {
        return (flags_ & ModifierFlags::CONSTRUCTOR) != 0;
    }

    [[nodiscard]] bool IsOverride() const noexcept
    {
        return (flags_ & ModifierFlags::OVERRIDE) != 0;
    }

    void SetOverride() noexcept
    {
        flags_ |= ModifierFlags::OVERRIDE;
    }

    [[nodiscard]] bool IsAsync() const noexcept
    {
        return (flags_ & ModifierFlags::ASYNC) != 0;
    }

    [[nodiscard]] bool IsSynchronized() const noexcept
    {
        return (flags_ & ModifierFlags::SYNCHRONIZED) != 0;
    }

    [[nodiscard]] bool IsNative() const noexcept
    {
        return (flags_ & ModifierFlags::NATIVE) != 0;
    }

    [[nodiscard]] bool IsNullAssignable() const noexcept
    {
        return (flags_ & ModifierFlags::NULL_ASSIGNABLE) != 0;
    }

    [[nodiscard]] bool IsUndefinedAssignable() const noexcept
    {
        return (flags_ & ModifierFlags::UNDEFINED_ASSIGNABLE) != 0;
    }

    [[nodiscard]] bool IsConst() const noexcept
    {
        return (flags_ & ModifierFlags::CONST) != 0;
    }

    [[nodiscard]] bool IsStatic() const noexcept
    {
        return (flags_ & ModifierFlags::STATIC) != 0;
    }

    [[nodiscard]] bool IsFinal() const noexcept
    {
        return (flags_ & ModifierFlags::FINAL) != 0U;
    }

    [[nodiscard]] bool IsAbstract() const noexcept
    {
        return (flags_ & ModifierFlags::ABSTRACT) != 0;
    }

    [[nodiscard]] bool IsPublic() const noexcept
    {
        return (flags_ & ModifierFlags::PUBLIC) != 0;
    }

    [[nodiscard]] bool IsProtected() const noexcept
    {
        return (flags_ & ModifierFlags::PROTECTED) != 0;
    }

    [[nodiscard]] bool IsPrivate() const noexcept
    {
        return (flags_ & ModifierFlags::PRIVATE) != 0;
    }

    [[nodiscard]] bool IsInternal() const noexcept
    {
        return (flags_ & ModifierFlags::INTERNAL) != 0;
    }

    [[nodiscard]] bool IsExported() const noexcept
    {
        if (UNLIKELY(IsClassDefinition())) {
            return parent_->IsExported();
        }

        return (flags_ & ModifierFlags::EXPORT) != 0;
    }

    [[nodiscard]] bool IsDefaultExported() const noexcept
    {
        if (UNLIKELY(IsClassDefinition())) {
            return parent_->IsDefaultExported();
        }

        return (flags_ & ModifierFlags::DEFAULT_EXPORT) != 0;
    }

    [[nodiscard]] bool IsDeclare() const noexcept
    {
        return (flags_ & ModifierFlags::DECLARE) != 0;
    }

    [[nodiscard]] bool IsIn() const noexcept
    {
        return (flags_ & ModifierFlags::IN) != 0;
    }

    [[nodiscard]] bool IsOut() const noexcept
    {
        return (flags_ & ModifierFlags::OUT) != 0;
    }

    [[nodiscard]] bool IsSetter() const noexcept
    {
        return (flags_ & ModifierFlags::SETTER) != 0;
    }

    void AddModifier(ModifierFlags const flags) noexcept
    {
        flags_ |= flags;
    }

    [[nodiscard]] ModifierFlags Modifiers() noexcept
    {
        return flags_;
    }

    [[nodiscard]] ModifierFlags Modifiers() const noexcept
    {
        return flags_;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_FLAG_OPERATIONS(flag_type, member_name)     \
    void Set##flag_type(flag_type flags) const noexcept     \
    {                                                       \
        (member_name) = flags;                              \
    }                                                       \
                                                            \
    void Add##flag_type(flag_type flag) const noexcept      \
    {                                                       \
        (member_name) |= flag;                              \
    }                                                       \
                                                            \
    [[nodiscard]] flag_type Get##flag_type() const noexcept \
    {                                                       \
        return (member_name);                               \
    }                                                       \
                                                            \
    bool Has##flag_type(flag_type flag) const noexcept      \
    {                                                       \
        return ((member_name)&flag) != 0U;                  \
    }                                                       \
    void Remove##flag_type(flag_type flag) const noexcept   \
    {                                                       \
        (member_name) &= ~flag;                             \
    }

    DECLARE_FLAG_OPERATIONS(BoxingUnboxingFlags, boxingUnboxingFlags_);
    DECLARE_FLAG_OPERATIONS(AstNodeFlags, astNodeFlags_);
#undef DECLARE_FLAG_OPERATIONS

    ir::ClassElement *AsClassElement()
    {
        ASSERT(IsMethodDefinition() || IsClassProperty() || IsClassStaticBlock());
        return reinterpret_cast<ir::ClassElement *>(this);
    }

    const ir::ClassElement *AsClassElement() const
    {
        ASSERT(IsMethodDefinition() || IsClassProperty() || IsClassStaticBlock());
        return reinterpret_cast<const ir::ClassElement *>(this);
    }

    [[nodiscard]] virtual bool IsScopeBearer() const
    {
        return false;
    }

    virtual varbinder::Scope *Scope() const
    {
        UNREACHABLE();
    }

    [[nodiscard]] ir::BlockStatement *GetTopStatement();
    [[nodiscard]] const ir::BlockStatement *GetTopStatement() const;

    [[nodiscard]] virtual AstNode *Clone([[maybe_unused]] ArenaAllocator *const allocator,
                                         [[maybe_unused]] AstNode *const parent)
    {
        UNREACHABLE();
    }

    virtual void TransformChildren(const NodeTransformer &cb) = 0;
    virtual void Iterate(const NodeTraverser &cb) const = 0;
    void TransformChildrenRecursively(const NodeTransformer &cb);
    void IterateRecursively(const NodeTraverser &cb) const;
    bool IsAnyChild(const NodePredicate &cb) const;
    AstNode *FindChild(const NodePredicate &cb) const;

    std::string DumpJSON() const;
    std::string DumpEtsSrc() const;

    virtual void Dump(ir::AstDumper *dumper) const = 0;
    virtual void Dump(ir::SrcDumper *dumper) const = 0;
    virtual void Compile([[maybe_unused]] compiler::PandaGen *pg) const = 0;
    virtual void Compile([[maybe_unused]] compiler::ETSGen *etsg) const {};
    virtual checker::Type *Check([[maybe_unused]] checker::TSChecker *checker) = 0;
    virtual checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) = 0;

    using ASTVisitorT = visitor::ASTAbstractVisitor;

    virtual void Accept(ASTVisitorT *v) = 0;

    /**
     * On each node you should implement:
     *  void accept(AV* v) override {
     *      ASTVisitorT::accept(this, v);
     *  }
     */
protected:
    AstNode(AstNode const &other);

    void SetType(AstNodeType const type) noexcept
    {
        type_ = type;
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    AstNode *parent_ {};
    lexer::SourceRange range_ {};
    AstNodeType type_;
    varbinder::Variable *variable_ {};
    ModifierFlags flags_ {};
    mutable AstNodeFlags astNodeFlags_ {};
    mutable BoxingUnboxingFlags boxingUnboxingFlags_ {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

template <typename T>
class Typed : public T {
public:
    Typed() = delete;
    ~Typed() override = default;

    NO_COPY_OPERATOR(Typed);
    NO_MOVE_SEMANTIC(Typed);

    [[nodiscard]] checker::Type *TsType() noexcept
    {
        return tsType_;
    }

    [[nodiscard]] const checker::Type *TsType() const noexcept
    {
        return tsType_;
    }

    void SetTsType(checker::Type *tsType) noexcept
    {
        tsType_ = tsType;
    }

    bool IsTyped() const override
    {
        return true;
    }

protected:
    explicit Typed(AstNodeType const type) : T(type) {}
    explicit Typed(AstNodeType const type, ModifierFlags const flags) : T(type, flags) {}

    // NOTE: when cloning node its type is not copied but removed empty so that it can be re-checked further.
    Typed(Typed const &other) : T(static_cast<T const &>(other)) {}

private:
    checker::Type *tsType_ {};
};

template <typename T>
class Annotated : public T {
public:
    Annotated() = delete;
    ~Annotated() override = default;

    NO_COPY_OPERATOR(Annotated);
    NO_MOVE_SEMANTIC(Annotated);

    [[nodiscard]] TypeNode *TypeAnnotation() const noexcept
    {
        return typeAnnotation_;
    }

    void SetTsTypeAnnotation(TypeNode *const typeAnnotation) noexcept
    {
        typeAnnotation_ = typeAnnotation;
    }

protected:
    explicit Annotated(AstNodeType const type, TypeNode *const typeAnnotation)
        : T(type), typeAnnotation_(typeAnnotation)
    {
    }
    explicit Annotated(AstNodeType const type) : T(type) {}
    explicit Annotated(AstNodeType const type, ModifierFlags const flags) : T(type, flags) {}

    Annotated(Annotated const &other) : T(static_cast<T const &>(other)) {}

private:
    TypeNode *typeAnnotation_ {};
};

class TypedAstNode : public Typed<AstNode> {
public:
    TypedAstNode() = delete;
    ~TypedAstNode() override = default;

    NO_COPY_OPERATOR(TypedAstNode);
    NO_MOVE_SEMANTIC(TypedAstNode);

protected:
    explicit TypedAstNode(AstNodeType const type) : Typed<AstNode>(type) {}
    explicit TypedAstNode(AstNodeType const type, ModifierFlags const flags) : Typed<AstNode>(type, flags) {}

    TypedAstNode(TypedAstNode const &other) : Typed<AstNode>(static_cast<Typed<AstNode> const &>(other)) {}
};

class AnnotatedAstNode : public Annotated<AstNode> {
public:
    AnnotatedAstNode() = delete;
    ~AnnotatedAstNode() override = default;

    NO_COPY_OPERATOR(AnnotatedAstNode);
    NO_MOVE_SEMANTIC(AnnotatedAstNode);

protected:
    explicit AnnotatedAstNode(AstNodeType const type, TypeNode *const typeAnnotation)
        : Annotated<AstNode>(type, typeAnnotation)
    {
    }
    explicit AnnotatedAstNode(AstNodeType const type) : Annotated<AstNode>(type) {}
    explicit AnnotatedAstNode(AstNodeType const type, ModifierFlags const flags) : Annotated<AstNode>(type, flags) {}

    AnnotatedAstNode(AnnotatedAstNode const &other) : Annotated<AstNode>(static_cast<Annotated<AstNode> const &>(other))
    {
    }
};
}  // namespace ark::es2panda::ir
#endif
