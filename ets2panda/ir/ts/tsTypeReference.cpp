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

#include "tsTypeReference.h"

#include "varbinder/declaration.h"
#include "varbinder/scope.h"
#include "varbinder/variable.h"
#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"
#include "ir/expressions/identifier.h"
#include "ir/ts/tsInterfaceDeclaration.h"
#include "ir/ts/tsTypeAliasDeclaration.h"
#include "ir/ts/tsTypeParameterInstantiation.h"
#include "ir/ts/tsEnumDeclaration.h"
#include "ir/ts/tsQualifiedName.h"

namespace panda::es2panda::ir {
void TSTypeReference::TransformChildren(const NodeTransformer &cb)
{
    if (type_params_ != nullptr) {
        type_params_ = cb(type_params_)->AsTSTypeParameterInstantiation();
    }

    type_name_ = cb(type_name_)->AsExpression();
}

void TSTypeReference::Iterate(const NodeTraverser &cb) const
{
    if (type_params_ != nullptr) {
        cb(type_params_);
    }

    cb(type_name_);
}

void TSTypeReference::Dump(ir::AstDumper *dumper) const
{
    dumper->Add(
        {{"type", "TSTypeReference"}, {"typeName", type_name_}, {"typeParameters", AstDumper::Optional(type_params_)}});
}

void TSTypeReference::Dump(ir::SrcDumper *dumper) const
{
    BaseName()->Dump(dumper);
}

void TSTypeReference::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}
void TSTypeReference::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

ir::Identifier *TSTypeReference::BaseName() const
{
    if (type_name_->IsIdentifier()) {
        return type_name_->AsIdentifier();
    }

    ir::TSQualifiedName *iter = type_name_->AsTSQualifiedName();

    while (iter->Left()->IsTSQualifiedName()) {
        iter = iter->Left()->AsTSQualifiedName();
    }

    return iter->Left()->AsIdentifier();
}

checker::Type *TSTypeReference::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *TSTypeReference::GetType([[maybe_unused]] checker::TSChecker *checker)
{
    if (TsType() != nullptr) {
        return TsType();
    }

    if (type_name_->IsTSQualifiedName()) {
        return checker->GlobalAnyType();
    }

    ASSERT(type_name_->IsIdentifier());
    varbinder::Variable *var = type_name_->AsIdentifier()->Variable();

    if (var == nullptr) {
        checker->ThrowTypeError({"Cannot find name ", type_name_->AsIdentifier()->Name()}, Start());
    }

    SetTsType(checker->GetTypeReferenceType(this, var));
    return TsType();
}

checker::Type *TSTypeReference::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace panda::es2panda::ir
