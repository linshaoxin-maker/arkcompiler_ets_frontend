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

#include "metaProperty.h"

#include "es2panda.h"
#include "compiler/core/pandagen.h"
#include "checker/TSchecker.h"
#include "ir/astDump.h"

namespace panda::es2panda::ir {
void MetaProperty::TransformChildren([[maybe_unused]] const NodeTransformer &cb) {}
void MetaProperty::Iterate([[maybe_unused]] const NodeTraverser &cb) const {}

void MetaProperty::Dump(ir::AstDumper *dumper) const
{
    const char *kind = nullptr;

    switch (kind_) {
        case MetaPropertyKind::NEW_TARGET: {
            kind = "new.Target";
            break;
        }
        case MetaPropertyKind::IMPORT_META: {
            kind = "import.Meta";
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    dumper->Add({{"type", "MetaProperty"}, {"kind", kind}});
}

void MetaProperty::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    if (kind_ == ir::MetaProperty::MetaPropertyKind::NEW_TARGET) {
        pg->GetNewTarget(this);
        return;
    }

    if (kind_ == ir::MetaProperty::MetaPropertyKind::IMPORT_META) {
        // NOTE
        pg->Unimplemented();
    }
}

checker::Type *MetaProperty::Check([[maybe_unused]] checker::TSChecker *checker)
{
    // NOTE: aszilagyi.
    return checker->GlobalAnyType();
}

checker::Type *MetaProperty::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}

// NOLINTNEXTLINE(google-default-arguments)
Expression *MetaProperty::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    if (auto *const clone = allocator->New<MetaProperty>(kind_); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
