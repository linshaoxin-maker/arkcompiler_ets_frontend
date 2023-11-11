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

#include "etsWildcardType.h"

#include "ir/astDump.h"
#include "ir/ets/etsTypeReference.h"
#include "checker/TSchecker.h"
#include "checker/ETSchecker.h"
#include "compiler/core/ETSGen.h"

namespace panda::es2panda::ir {
void ETSWildcardType::TransformChildren(const NodeTransformer &cb)
{
    if (type_reference_ != nullptr) {
        type_reference_ = cb(type_reference_)->AsETSTypeReference();
    }
}

void ETSWildcardType::Iterate(const NodeTraverser &cb) const
{
    if (type_reference_ != nullptr) {
        cb(type_reference_);
    }
}

void ETSWildcardType::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ETSWildcardType"},
                 {"typeReference", AstDumper::Optional(type_reference_)},
                 {"in", AstDumper::Optional(IsIn())},
                 {"out", AstDumper::Optional(IsOut())}});
}

void ETSWildcardType::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

void ETSWildcardType::Compile([[maybe_unused]] compiler::ETSGen *etsg) const
{
    etsg->Unimplemented();
}

checker::Type *ETSWildcardType::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *ETSWildcardType::GetType([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *ETSWildcardType::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}

checker::Type *ETSWildcardType::GetType([[maybe_unused]] checker::ETSChecker *checker)
{
    return checker->GlobalWildcardType();
}
}  // namespace panda::es2panda::ir
