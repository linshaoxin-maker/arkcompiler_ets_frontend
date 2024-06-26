/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "annotationUsage.h"

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"
namespace ark::es2panda::ir {
void AnnotationUsage::TransformChildren(const NodeTransformer &cb, std::string_view const transformationName)
{
    for (auto *&it : properties_) {
        if (auto *transformedNode = cb(it); it != transformedNode) {
            it->SetTransformedNode(transformationName, transformedNode);
            it = transformedNode;
        }
    }

    if (expr_ != nullptr) {
        if (auto *transformedNode = cb(expr_); expr_ != transformedNode) {
            expr_->SetTransformedNode(transformationName, transformedNode);
            expr_ = transformedNode->AsExpression();
        }
    }
}
void AnnotationUsage::Iterate(const NodeTraverser &cb) const
{
    if (expr_ != nullptr) {
        cb(expr_);
    }

    for (auto *it : properties_) {
        cb(it);
    }
}

void AnnotationUsage::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "Annotation"}, {"Expression", expr_}, {"properties", properties_}});
}
void AnnotationUsage::Dump(ir::SrcDumper *dumper) const
{  // re-understand
    ASSERT(expr_ != nullptr);
    dumper->Add("annotation ");
    expr_->Dump(dumper);
    dumper->Add(" {");

    if (!properties_.empty()) {
        dumper->IncrIndent();
        dumper->Endl();
        for (auto elem : properties_) {
            elem->Dump(dumper);
            if (elem == properties_.back()) {
                dumper->DecrIndent();
            }
            dumper->Endl();
        }
    }
    dumper->Add("}");
    dumper->Endl();
}
void AnnotationUsage::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void AnnotationUsage::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *AnnotationUsage::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *AnnotationUsage::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace ark::es2panda::ir
