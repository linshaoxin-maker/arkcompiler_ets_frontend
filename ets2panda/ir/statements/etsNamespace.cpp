/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "etsNamespace.h"

#include "checker/TSchecker.h"
#include "checker/ETSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"
#include "ir/expressions/identifier.h"

namespace ark::es2panda::ir {
void ETSNamespace::TransformChildren([[maybe_unused]] const NodeTransformer &cb,
                                             [[maybe_unused]] std::string_view const transformationName)
{
}

void ETSNamespace::Iterate([[maybe_unused]] const NodeTraverser &cb) const {}

void ETSNamespace::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "Namespace"}, {"expr", AstDumper::Nullish(expr_)}, {"statements", Statements()}});
}

void ETSNamespace::Dump([[maybe_unused]] ir::SrcDumper *dumper) const {}

void ETSNamespace::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void ETSNamespace::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *ETSNamespace::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *ETSNamespace::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

Identifier *ETSNamespace::GetBaseName() const
{
    return expr_->AsIdentifier();
    // auto *part = expr_->AsETSTypeReference()->Part();
    // if (part->Name()->IsIdentifier()){
    //     return part->Name()->AsIdentifier();
    // }
    // return part->Name()->AsTSQualifiedName()->Right();
}
}  // namespace ark::es2panda::ir
