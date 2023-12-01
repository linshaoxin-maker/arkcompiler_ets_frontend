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

#include "tsImportEqualsDeclaration.h"

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/expression.h"
#include "ir/expressions/identifier.h"

namespace panda::es2panda::ir {
void TSImportEqualsDeclaration::TransformChildren(const NodeTransformer &cb)
{
    id_ = cb(id_)->AsIdentifier();
    module_reference_ = cb(module_reference_)->AsExpression();
}

void TSImportEqualsDeclaration::Iterate(const NodeTraverser &cb) const
{
    cb(id_);
    cb(module_reference_);
}

void TSImportEqualsDeclaration::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSImportEqualsDeclaration"},
                 {"id", id_},
                 {"moduleReference", module_reference_},
                 {"isExport", is_export_}});
}

void TSImportEqualsDeclaration::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}
void TSImportEqualsDeclaration::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *TSImportEqualsDeclaration::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *TSImportEqualsDeclaration::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace panda::es2panda::ir
