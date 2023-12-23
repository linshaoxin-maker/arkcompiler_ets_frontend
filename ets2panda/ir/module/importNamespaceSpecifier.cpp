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

#include "importNamespaceSpecifier.h"

#include "varbinder/ETSBinder.h"
#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"

namespace panda::es2panda::ir {
void ImportNamespaceSpecifier::TransformChildren(const NodeTransformer &cb)
{
    local_ = cb(local_)->AsIdentifier();
}

void ImportNamespaceSpecifier::Iterate(const NodeTraverser &cb) const
{
    cb(local_);
}

void ImportNamespaceSpecifier::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ImportNamespaceSpecifier"}, {"local", local_}});
}

void ImportNamespaceSpecifier::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void ImportNamespaceSpecifier::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *ImportNamespaceSpecifier::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *ImportNamespaceSpecifier::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace panda::es2panda::ir
