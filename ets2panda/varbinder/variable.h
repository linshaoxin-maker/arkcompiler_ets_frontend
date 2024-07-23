/**
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

#ifndef ES2PANDA_COMPILER_SCOPES_VARIABLE_H
#define ES2PANDA_COMPILER_SCOPES_VARIABLE_H

#include "varbinder/enumMemberResult.h"
#include "varbinder/variableFlags.h"
#include "ir/irnode.h"
#include "macros.h"
#include "util/ustring.h"

#include <limits>

namespace ark::es2panda::checker {
class Type;
enum class PropertyType;
}  // namespace ark::es2panda::checker

namespace ark::es2panda::varbinder {
class Decl;
class Scope;
class VariableScope;

class Variable {
public:
    virtual ~Variable() = default;
    NO_COPY_SEMANTIC(Variable);
    NO_MOVE_SEMANTIC(Variable);

    VariableType virtual Type() const = 0;

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
        return const_cast<T *>(const_cast<const Variable *>(this)->As<T>());
    }

    [[nodiscard]] const Decl *Declaration() const noexcept
    {
        return decl_;
    }

    [[nodiscard]] Decl *Declaration() noexcept
    {
        return decl_;
    }

    [[nodiscard]] VariableFlags Flags() const noexcept
    {
        return flags_;
    }

    [[nodiscard]] checker::Type *TsType() const noexcept
    {
        return tsType_;
    }

    [[nodiscard]] Scope *GetScope() const noexcept
    {
        return scope_;
    }

    void SetTsType(checker::Type *tsType) noexcept
    {
        tsType_ = tsType;
    }

    void SetScope(varbinder::Scope *scope) noexcept
    {
        scope_ = scope;
    }

    void AddFlag(VariableFlags flag) noexcept
    {
        flags_ |= flag;
    }

    [[nodiscard]] bool HasFlag(VariableFlags flag) const noexcept
    {
        return (flags_ & flag) != 0;
    }

    void RemoveFlag(VariableFlags flag) noexcept
    {
        flags_ &= ~flag;
    }

    void Reset(Decl *decl, VariableFlags flags) noexcept
    {
        decl_ = decl;
        flags_ = flags;
    }

    [[nodiscard]] bool LexicalBound() const noexcept
    {
        return HasFlag(VariableFlags::LEXICAL_BOUND);
    }

    [[nodiscard]] const util::StringView &Name() const;
    virtual void SetLexical(Scope *scope) = 0;

protected:
    explicit Variable(Decl *decl, VariableFlags flags) : decl_(decl), flags_(flags) {}
    explicit Variable(VariableFlags flags) : flags_(flags) {}

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    Decl *decl_ {};
    VariableFlags flags_ {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)

private:
    checker::Type *tsType_ {};
    Scope *scope_ {};
};

template <VariableType V>
class TypedVariable : public Variable {
public:
    static constexpr VariableType TYPE = V;

    VariableType Type() const override
    {
        return TYPE;
    }

protected:
    using Variable::Variable;
};

class LocalVariable : public TypedVariable<VariableType::LOCAL> {
public:
    explicit LocalVariable(Decl *decl, VariableFlags flags);
    explicit LocalVariable(VariableFlags flags);

    void BindVReg(compiler::VReg vreg)
    {
        ASSERT(!LexicalBound());
        vreg_ = vreg;
    }

    void BindLexEnvSlot(uint32_t slot)
    {
        ASSERT(!LexicalBound());
        AddFlag(VariableFlags::LEXICAL_BOUND);
        vreg_.SetIndex(slot);
    }

    compiler::VReg Vreg() const
    {
        return vreg_;
    }

    compiler::VReg &Vreg()
    {
        return vreg_;
    }

    uint32_t LexIdx() const
    {
        ASSERT(LexicalBound());
        return vreg_.GetIndex();
    }

    void SetLexical([[maybe_unused]] Scope *scope) override;
    LocalVariable *Copy(ArenaAllocator *allocator, Decl *decl) const;

private:
    compiler::VReg vreg_ {};
};

class GlobalVariable : public TypedVariable<VariableType::GLOBAL> {
public:
    explicit GlobalVariable(Decl *decl, VariableFlags flags) : TypedVariable(decl, flags) {}

    void SetLexical([[maybe_unused]] Scope *scope) override;
};

class ModuleVariable : public TypedVariable<VariableType::MODULE> {
public:
    explicit ModuleVariable(Decl *decl, VariableFlags flags) : TypedVariable(decl, flags) {}

    compiler::VReg &ModuleReg()
    {
        return moduleReg_;
    }

    compiler::VReg ModuleReg() const
    {
        return moduleReg_;
    }

    const util::StringView &ExoticName() const
    {
        return exoticName_;
    }

    util::StringView &ExoticName()
    {
        return exoticName_;
    }

    void SetLexical([[maybe_unused]] Scope *scope) override;

private:
    compiler::VReg moduleReg_ {};
    util::StringView exoticName_ {};
};

class EnumVariable : public TypedVariable<VariableType::ENUM> {
public:
    explicit EnumVariable(Decl *decl, bool backReference = false)
        : TypedVariable(decl, VariableFlags::NONE), backReference_(backReference)
    {
    }

    void SetValue(EnumMemberResult value)
    {
        value_ = value;
    }

    const EnumMemberResult &Value() const
    {
        return value_;
    }

    bool BackReference() const
    {
        return backReference_;
    }

    void SetBackReference()
    {
        backReference_ = true;
    }

    void ResetDecl(Decl *decl);

    void SetLexical([[maybe_unused]] Scope *scope) override;

private:
    EnumMemberResult value_ {};
    bool backReference_ {};
};
}  // namespace ark::es2panda::varbinder
#endif
