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

#include "tsInterfaceDeclaration.h"

#include "macros.h"
#include "varbinder/declaration.h"
#include "varbinder/scope.h"
#include "varbinder/variable.h"
#include "checker/TSchecker.h"
#include "checker/ETSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"
#include "ir/base/decorator.h"
#include "ir/expressions/identifier.h"
#include "ir/ts/tsInterfaceBody.h"
#include "ir/ts/tsInterfaceHeritage.h"
#include "ir/ts/tsTypeParameter.h"
#include "ir/ts/tsTypeParameterDeclaration.h"

namespace ark::es2panda::ir {
void TSInterfaceDeclaration::TransformChildren(const NodeTransformer &cb)
{
    for (auto *&it : decorators_) {
        it = cb(it)->AsDecorator();
    }

    id_ = cb(id_)->AsIdentifier();

    if (typeParams_ != nullptr) {
        typeParams_ = cb(typeParams_)->AsTSTypeParameterDeclaration();
    }

    for (auto *&it : extends_) {
        it = cb(it)->AsTSInterfaceHeritage();
    }

    body_ = cb(body_)->AsTSInterfaceBody();
}

void TSInterfaceDeclaration::Iterate(const NodeTraverser &cb) const
{
    for (auto *it : decorators_) {
        cb(it);
    }

    cb(id_);

    if (typeParams_ != nullptr) {
        cb(typeParams_);
    }

    for (auto *it : extends_) {
        cb(it);
    }

    cb(body_);
}

void TSInterfaceDeclaration::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSInterfaceDeclaration"},
                 {"decorators", AstDumper::Optional(decorators_)},
                 {"body", body_},
                 {"id", id_},
                 {"extends", extends_},
                 {"typeParameters", AstDumper::Optional(typeParams_)}});
}

void TSInterfaceDeclaration::Dump(ir::SrcDumper *dumper) const
{
    ASSERT(id_);

    dumper->Add("interface ");
    id_->Dump(dumper);

    if (typeParams_ != nullptr) {
        dumper->Add("<");
        typeParams_->Dump(dumper);
        dumper->Add(">");
    }

    if (!extends_.empty()) {
        dumper->Add(" extends ");
        for (auto ext : extends_) {
            ext->Dump(dumper);
            if (ext != extends_.back()) {
                dumper->Add(", ");
            }
        }
    }

    dumper->Add(" {");
    if (body_ != nullptr) {
        dumper->IncrIndent();
        dumper->Endl();
        body_->Dump(dumper);
        dumper->DecrIndent();
        dumper->Endl();
    }
    dumper->Add("}");
    dumper->Endl();
}

void TSInterfaceDeclaration::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void TSInterfaceDeclaration::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *TSInterfaceDeclaration::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *TSInterfaceDeclaration::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace ark::es2panda::ir
