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

#include "booleanLiteral.h"

#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "checker/TSchecker.h"
#include "checker/ETSchecker.h"
#include "ir/astDump.h"

namespace panda::es2panda::ir {
void BooleanLiteral::TransformChildren([[maybe_unused]] const NodeTransformer &cb) {}
void BooleanLiteral::Iterate([[maybe_unused]] const NodeTraverser &cb) const {}

void BooleanLiteral::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "BooleanLiteral"}, {"value", boolean_}});
}

void BooleanLiteral::Compile(compiler::PandaGen *pg) const
{
    pg->LoadConst(this, boolean_ ? compiler::Constant::JS_TRUE : compiler::Constant::JS_FALSE);
}

void BooleanLiteral::Compile(compiler::ETSGen *etsg) const
{
    etsg->LoadAccumulatorBoolean(this, boolean_);
}

checker::Type *BooleanLiteral::Check(checker::TSChecker *checker)
{
    return boolean_ ? checker->GlobalTrueType() : checker->GlobalFalseType();
}

checker::Type *BooleanLiteral::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    if (TsType() == nullptr) {
        SetTsType(checker->CreateETSBooleanType(boolean_));
    }
    return TsType();
}

// NOLINTNEXTLINE(google-default-arguments)
Expression *BooleanLiteral::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    if (auto *const clone = allocator->New<BooleanLiteral>(boolean_); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
