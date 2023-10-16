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

#include "tsVoidKeyword.h"

#include "ir/astDump.h"
#include "checker/TSchecker.h"

namespace panda::es2panda::ir {
void TSVoidKeyword::TransformChildren([[maybe_unused]] const NodeTransformer &cb) {}
void TSVoidKeyword::Iterate([[maybe_unused]] const NodeTraverser &cb) const {}

void TSVoidKeyword::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSVoidKeyword"}});
}

void TSVoidKeyword::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *TSVoidKeyword::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *TSVoidKeyword::GetType([[maybe_unused]] checker::TSChecker *checker)
{
    return checker->GlobalVoidType();
}

checker::Type *TSVoidKeyword::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}
}  // namespace panda::es2panda::ir
