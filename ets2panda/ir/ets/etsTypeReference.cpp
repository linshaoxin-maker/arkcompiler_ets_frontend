/*
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

#include "etsTypeReference.h"

#include "checker/ETSchecker.h"
#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/ts/tsQualifiedName.h"
#include "ir/ets/etsTypeReferencePart.h"

namespace panda::es2panda::ir {
void ETSTypeReference::TransformChildren(const NodeTransformer &cb)
{
    part_ = cb(part_)->AsETSTypeReferencePart();
}

void ETSTypeReference::Iterate(const NodeTraverser &cb) const
{
    cb(part_);
}

ir::Identifier *ETSTypeReference::BaseName()
{
    ir::ETSTypeReferencePart *part_iter = part_;

    while (part_iter->Previous() != nullptr) {
        part_iter = part_iter->Previous();
    }

    ir::Expression *base_name = part_iter->Name();

    if (base_name->IsIdentifier()) {
        return base_name->AsIdentifier();
    }

    ir::TSQualifiedName *name_iter = base_name->AsTSQualifiedName();

    while (name_iter->Left()->IsTSQualifiedName()) {
        name_iter = name_iter->Left()->AsTSQualifiedName();
    }

    return name_iter->Left()->AsIdentifier();
}

void ETSTypeReference::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ETSTypeReference"}, {"part", part_}});
}

void ETSTypeReference::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}
void ETSTypeReference::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *ETSTypeReference::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *ETSTypeReference::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *ETSTypeReference::GetType(checker::ETSChecker *checker)
{
    if (TsType() != nullptr) {
        return TsType();
    }

    checker::Type *type = part_->GetType(checker);
    if (IsNullable()) {
        type = type->Instantiate(checker->Allocator(), checker->Relation(), checker->GetGlobalTypesHolder());
        type->AddTypeFlag(checker::TypeFlag::NULLABLE);
    }

    SetTsType(type);
    return type;
}
}  // namespace panda::es2panda::ir
