/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "breakStatement.h"

#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "ir/astDump.h"
#include "ir/expressions/identifier.h"
#include "checker/ETSchecker.h"

namespace panda::es2panda::ir {
void BreakStatement::TransformChildren(const NodeTransformer &cb)
{
    if (ident_ != nullptr) {
        ident_ = cb(ident_)->AsIdentifier();
    }
}

void BreakStatement::Iterate(const NodeTraverser &cb) const
{
    if (ident_ != nullptr) {
        cb(ident_);
    }
}

void BreakStatement::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "BreakStatement"}, {"label", AstDumper::Nullable(ident_)}});
}

template <typename CodeGen>
void CompileImpl(const BreakStatement *self, [[maybe_unused]] CodeGen *cg)
{
    compiler::Label *target = cg->ControlFlowChangeBreak(self->Ident());
    cg->Branch(self, target);
}

void BreakStatement::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    CompileImpl(this, pg);
}

void BreakStatement::Compile([[maybe_unused]] compiler::ETSGen *etsg) const
{
    if (etsg->ExtendWithFinalizer(parent_, this)) {
        return;
    }
    CompileImpl(this, etsg);
}

checker::Type *BreakStatement::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *BreakStatement::Check(checker::ETSChecker *checker)
{
    target_ = checker->FindJumpTarget(Type(), this, ident_);
    return nullptr;
}
}  // namespace panda::es2panda::ir
