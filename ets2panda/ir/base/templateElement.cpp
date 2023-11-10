/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "templateElement.h"

#include "es2panda.h"
#include "compiler/core/ETSGen.h"
#include "ir/astDump.h"
#include "util/ustring.h"

namespace panda::es2panda::ir {
void TemplateElement::TransformChildren([[maybe_unused]] const NodeTransformer &cb) {}
void TemplateElement::Iterate([[maybe_unused]] const NodeTraverser &cb) const {}

void TemplateElement::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({
        {"type", "TemplateElement"},
        {"value", {{"raw", ""}, {"cooked", ""}}},
    });
}

void TemplateElement::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

void TemplateElement::Compile([[maybe_unused]] compiler::ETSGen *etsg) const
{
    etsg->LoadAccumulatorString(this, raw_);
}

checker::Type *TemplateElement::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *TemplateElement::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    SetTsType(checker->CreateETSStringLiteralType(raw_));
    return TsType();
}

// NOLINTNEXTLINE(google-default-arguments)
Expression *TemplateElement::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    if (auto *const clone = allocator->New<TemplateElement>(raw_, cooked_); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
