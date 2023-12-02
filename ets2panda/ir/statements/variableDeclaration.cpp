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

#include "variableDeclaration.h"

#include "varbinder/scope.h"
#include "varbinder/variable.h"
#include "checker/TSchecker.h"
#include "checker/ETSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/base/decorator.h"
#include "ir/expressions/arrayExpression.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/objectExpression.h"
#include "ir/statements/variableDeclarator.h"

namespace panda::es2panda::ir {
void VariableDeclaration::TransformChildren(const NodeTransformer &cb)
{
    for (auto *&it : decorators_) {
        it = cb(it)->AsDecorator();
    }

    for (auto *&it : declarators_) {
        it = cb(it)->AsVariableDeclarator();
    }
}

void VariableDeclaration::Iterate(const NodeTraverser &cb) const
{
    for (auto *it : decorators_) {
        cb(it);
    }

    for (auto *it : declarators_) {
        cb(it);
    }
}

void VariableDeclaration::Dump(ir::AstDumper *dumper) const
{
    const char *kind = nullptr;

    switch (kind_) {
        case VariableDeclarationKind::CONST: {
            kind = "const";
            break;
        }
        case VariableDeclarationKind::LET: {
            kind = "let";
            break;
        }
        case VariableDeclarationKind::VAR: {
            kind = "var";
            break;
        }
        default: {
            UNREACHABLE();
        }
    }

    dumper->Add({{"type", "VariableDeclaration"},
                 {"declarations", declarators_},
                 {"kind", kind},
                 {"decorators", AstDumper::Optional(decorators_)},
                 {"declare", AstDumper::Optional(declare_)}});
}

void VariableDeclaration::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void VariableDeclaration::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *VariableDeclaration::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *VariableDeclaration::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}
}  // namespace panda::es2panda::ir
