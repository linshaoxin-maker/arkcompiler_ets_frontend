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

#include "tsMappedType.h"

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/typeNode.h"
#include "ir/ts/tsTypeParameter.h"

namespace panda::es2panda::ir {
void TSMappedType::TransformChildren(const NodeTransformer &cb)
{
    type_parameter_ = cb(type_parameter_)->AsTSTypeParameter();
    if (type_annotation_ != nullptr) {
        type_annotation_ = static_cast<TypeNode *>(cb(type_annotation_));
    }
}

void TSMappedType::Iterate(const NodeTraverser &cb) const
{
    cb(type_parameter_);
    if (type_annotation_ != nullptr) {
        cb(type_annotation_);
    }
}

void TSMappedType::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSMappedType"},
                 {"typeParameter", type_parameter_},
                 {"typeAnnotation", AstDumper::Optional(type_annotation_)},
                 {"readonly", readonly_ == MappedOption::NO_OPTS ? AstDumper::Optional(false)
                              : readonly_ == MappedOption::PLUS  ? AstDumper::Optional("+")
                                                                 : AstDumper::Optional("-")},
                 {"optional", optional_ == MappedOption::NO_OPTS ? AstDumper::Optional(false)
                              : optional_ == MappedOption::PLUS  ? AstDumper::Optional("+")
                                                                 : AstDumper::Optional("-")}});
}

void TSMappedType::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}
void TSMappedType::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *TSMappedType::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *TSMappedType::GetType([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *TSMappedType::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace panda::es2panda::ir
