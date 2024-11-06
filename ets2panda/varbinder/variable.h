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
enum class SignatureFlags : uint32_t;
enum class PropertyType;
class Signature;
// NOLINTBEGIN(readability-redundant-declaration)
bool IsTypeError(Type const *tp);
[[noreturn]] void ThrowEmptyError();
// NOLINTEND(readability-redundant-declaration)
}  // namespace ark::es2panda::checker

namespace ark::es2panda::varbinder {
class Decl;
class Scope;
class VariableScope;

// CC-OFFNXT(G.PRE.09) code gen
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_CLASSES(type, className) class className;  // CC-OFF(G.PRE.02) name part
VARIABLE_TYPES(DECLARE_CLASSES)
#undef DECLARE_CLASSES

class FunctionInfoData {
    public:
    FunctionInfoData(ArenaAllocator *allocator)
        : nameUnion_(), setterInternalName_(), signatureFlags_(), callSignatures_(allocator->Adapter()) {};

    [[nodiscard]]const ArenaVector<checker::Signature *> &CallSignatures() const
    {
        return callSignatures_;
    }

    [[nodiscard]] FunctionInfoData *Copy(ArenaAllocator *allocator) const
    {
        auto *newFunctionInfoData = allocator->New<FunctionInfoData>(allocator);
        newFunctionInfoData->SetCallSignatures(CallSignatures());
        newFunctionInfoData->SetInternalName(nameUnion_.internalName_);
        newFunctionInfoData->SetInternalSetterName(setterInternalName_);
        newFunctionInfoData->SetSigFlags(SigFlags());
        newFunctionInfoData->SetSigSetterFlags(SigSetterFlags());
        return newFunctionInfoData;
    }

    void SetCallSignatures(const ArenaVector<checker::Signature *> &callSig)
    {
        callSignatures_.clear();
        callSignatures_.insert(callSignatures_.begin(),callSig.begin(),callSig.end());
    }

    [[nodiscard]] util::StringView InternalName() const
    {
        if (nameUnion_.internalName_ == "") {
            UNREACHABLE();
        }
        return nameUnion_.internalName_;
    }

    void SetInternalName(const util::StringView &internalName)
    {
        nameUnion_.internalName_ = internalName;
    }

    [[nodiscard]] util::StringView InternalGetterName() const
    {
        if (nameUnion_.getterInternalName_ == "") {
            UNREACHABLE();
        }
        return nameUnion_.getterInternalName_;
    }

    void SetInternalGetterName(const util::StringView &internalName)
    {
        nameUnion_.getterInternalName_ = internalName;
    }

    [[nodiscard]] util::StringView InternalSetterName() const
    {
        if (setterInternalName_ == "") {
            UNREACHABLE();
        }
        return setterInternalName_;
    }

    void SetInternalSetterName(const util::StringView &internalName)
    {
        setterInternalName_ = internalName;
    }

    [[nodiscard]] checker::SignatureFlags SigFlags() const
    {
        return signatureFlags_.sigFlags_;
    }

    void SetSigFlags(const checker::SignatureFlags &sigFlags)
    {
        signatureFlags_.sigFlags_ = sigFlags;
    }

    [[nodiscard]] const checker::SignatureFlags &SigGetterFlags() const
    {
        return signatureFlags_.getterSigFlags_;
    }

    void SetSigGetterFlags(const checker::SignatureFlags &sigFlags)
    {
        signatureFlags_.getterSigFlags_ = sigFlags;
    }

    [[nodiscard]] checker::SignatureFlags SigSetterFlags() const
    {
        return setterSigFlags_;
    }

    void SetSigSetterFlags(const checker::SignatureFlags &sigFlags)
    {
        setterSigFlags_ = sigFlags;
    }

    union NameUnion {
        NameUnion() : internalName_() {};
        util::StringView internalName_;
        util::StringView getterInternalName_;
    };
    union SigFlagsUnion {
        SigFlagsUnion() : sigFlags_() {};
        checker::SignatureFlags sigFlags_;
        checker::SignatureFlags getterSigFlags_;
    };

private:
    NameUnion nameUnion_;
    util::StringView setterInternalName_;
    SigFlagsUnion signatureFlags_;
    checker::SignatureFlags setterSigFlags_;

    // todo
    // to finish the whole transfer from etsfuntiontype to functionInterface , we will remove this in the future.
    // The problem exist now is that we use check() after CheckerPhase and LambdaLowering, mainly from build of new
    // types and node.
    ArenaVector<checker::Signature *> callSignatures_;
};

class Variable {
public:
    virtual ~Variable() = default;
    NO_COPY_SEMANTIC(Variable);
    NO_MOVE_SEMANTIC(Variable);

    VariableType virtual Type() const = 0;

/* CC-OFFNXT(G.PRE.06) solid logic */
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DECLARE_CHECKS_CASTS(variableType, className)                                       \
    bool Is##className() const                                                              \
    {                                                                                       \
        /* CC-OFFNXT(G.PRE.05) The macro is used to generate a function. Return is needed*/ \
        return Type() == VariableType::variableType; /* CC-OFF(G.PRE.02) name part */       \
    }                                                                                       \
    /* CC-OFFNXT(G.PRE.02) name part */                                                     \
    className *As##className()                                                              \
    {                                                                                       \
        ASSERT(Is##className());                                                            \
        /* CC-OFFNXT(G.PRE.05) The macro is used to generate a function. Return is needed*/ \
        return reinterpret_cast<className *>(this); /* CC-OFF(G.PRE.02) name part */        \
    }                                                                                       \
    const className *As##className() const                                                  \
    {                                                                                       \
        ASSERT(Is##className());                                                            \
        /* CC-OFFNXT(G.PRE.05) The macro is used to generate a function. Return is needed*/ \
        return reinterpret_cast<const className *>(this);                                   \
    }
    VARIABLE_TYPES(DECLARE_CHECKS_CASTS)
#undef DECLARE_CHECKS_CASTS

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

    [[nodiscard]] checker::Type *TsType() const
    {
        if (UNLIKELY(IsTypeError(tsType_))) {
            checker::ThrowEmptyError();
        }
        return tsType_;
    }

    [[nodiscard]] FunctionInfoData *FunctionInfo() const
    {
        return functionInfo_;
    }

    void SetFunctionInfo(FunctionInfoData *functionInfo)
    {
        functionInfo_ = functionInfo;
    }


    [[nodiscard]] checker::Type *TsTypeOrError() const noexcept
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
    explicit Variable(Decl *decl, VariableFlags flags) : decl_(decl), flags_(flags), functionInfo_(nullptr) {}
    explicit Variable(VariableFlags flags) : flags_(flags), functionInfo_(nullptr) {}

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    Decl *decl_ {};
    VariableFlags flags_ {};
    FunctionInfoData *functionInfo_;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

private:
    checker::Type *tsType_ {};
    Scope *scope_ {};
};

class LocalVariable : public Variable {
public:
    explicit LocalVariable(Decl *decl, VariableFlags flags);
    explicit LocalVariable(VariableFlags flags);

    VariableType Type() const override
    {
        return VariableType::LOCAL;
    }

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

class GlobalVariable : public Variable {
public:
    explicit GlobalVariable(Decl *decl, VariableFlags flags) : Variable(decl, flags) {}

    VariableType Type() const override
    {
        return VariableType::GLOBAL;
    }

    void SetLexical([[maybe_unused]] Scope *scope) override;
};

class ModuleVariable : public Variable {
public:
    explicit ModuleVariable(Decl *decl, VariableFlags flags) : Variable(decl, flags) {}

    VariableType Type() const override
    {
        return VariableType::MODULE;
    }

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

class EnumVariable : public Variable {
public:
    explicit EnumVariable(Decl *decl, bool backReference = false)
        : Variable(decl, VariableFlags::NONE), backReference_(backReference)
    {
    }

    VariableType Type() const override
    {
        return VariableType::ENUM;
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
