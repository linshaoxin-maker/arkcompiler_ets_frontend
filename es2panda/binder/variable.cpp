/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "variable.h"

#include <binder/scope.h>
#include <util/hotfix.h>

#include <utility>

namespace panda::es2panda::binder {

LocalVariable::LocalVariable(Decl *decl, VariableFlags flags) : Variable(decl, flags)
{
    if (decl->IsConstDecl()) {
        flags_ |= VariableFlags::READONLY;
    }
}

const util::StringView &Variable::Name() const
{
    return decl_->Name();
}

LocalVariable *LocalVariable::Copy(ArenaAllocator *allocator, Decl *decl) const
{
    auto *var = allocator->New<LocalVariable>(decl, flags_);
    var->vreg_ = vreg_;
    return var;
}

void LocalVariable::SetLexical(Scope *scope)
{
    if (LexicalBound()) {
        return;
    }

    VariableScope *varScope = scope->EnclosingVariableScope();
    uint32_t slot = 0;
    auto name = Declaration()->Name();

    if (util::Hotfix::GetInstance()->NeedSetLexicalForPatch(varScope)) {
        slot = util::Hotfix::GetInstance()->SetLexicalForPatch(varScope, std::string(name));
    } else {
        slot = varScope->NextSlot();
    }

    BindLexEnvSlot(slot);
    varScope->AddLexicalVarName(slot, name); // gather lexical variables for debuginfo
    varScope->AddLexicalVarTypes(slot,
        static_cast<typename std::underlying_type<binder::DeclType>::type>(Declaration()->Type()));
}

void GlobalVariable::SetLexical([[maybe_unused]] Scope *scope) {}
void ModuleVariable::SetLexical([[maybe_unused]] Scope *scope) {}
void EnumVariable::SetLexical([[maybe_unused]] Scope *scope) {}

void EnumVariable::ResetDecl(Decl *decl)
{
    decl_ = decl;
}

}  // namespace panda::es2panda::binder
