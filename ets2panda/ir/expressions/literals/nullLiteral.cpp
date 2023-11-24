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

#include "nullLiteral.h"

#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "checker/TSchecker.h"
#include "checker/ETSchecker.h"
#include "ir/astDump.h"

namespace panda::es2panda::ir {
void NullLiteral::TransformChildren([[maybe_unused]] const NodeTransformer &cb) {}
void NullLiteral::Iterate([[maybe_unused]] const NodeTraverser &cb) const {}

void NullLiteral::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "NullLiteral"}, {"value", AstDumper::Property::Constant::PROP_NULL}});
}

void NullLiteral::Compile(compiler::PandaGen *pg) const
{
    pg->LoadConst(this, compiler::Constant::JS_NULL);
}

void NullLiteral::Compile(compiler::ETSGen *etsg) const
{
    etsg->LoadAccumulatorNull(this, TsType());
}

checker::Type *NullLiteral::Check(checker::TSChecker *checker)
{
    return checker->GlobalNullType();
}

checker::Type *NullLiteral::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    if (TsType() == nullptr) {
        SetTsType(checker->GlobalETSNullType());
    }
    return TsType();
}

// NOLINTNEXTLINE(google-default-arguments)
NullLiteral *NullLiteral::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    if (auto *const clone = allocator->New<NullLiteral>(); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
