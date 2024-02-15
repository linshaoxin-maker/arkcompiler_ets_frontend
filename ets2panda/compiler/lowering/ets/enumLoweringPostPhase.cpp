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

#include "enumLoweringPostPhase.h"
#include "compiler/lowering/scopesInit/scopesInitPhase.h"

#include "varbinder/variableFlags.h"
#include "varbinder/ETSBinder.h"
#include "checker/ETSchecker.h"
#include "compiler/core/compilerContext.h"
#include "compiler/lowering/util.h"
#include "ir/statements/classDeclaration.h"
#include "ir/base/classDefinition.h"
#include "ir/base/classProperty.h"
#include "ir/astNode.h"
#include "ir/expression.h"
#include "util/ustring.h"

#include "ir/ts/tsEnumDeclaration.h"

namespace ark::es2panda::compiler {

bool EnumLoweringPostPhase::Perform(public_lib::Context *ctx, parser::Program *program)
{
    if (program->Extension() != ScriptExtension::ETS) {
        return true;
    }

    [[maybe_unused]] checker::ETSChecker *checker = ctx->checker->AsETSChecker();

    for (auto &[_, ext_programs] : program->ExternalSources()) {
        (void)_;
        for (auto *ext_prog : ext_programs) {
            Perform(ctx, ext_prog);
        }
    }

    program->Ast()->TransformChildrenRecursively([program](ir::AstNode *ast) -> ir::AstNode * {
        if (ast->IsCallExpression()) {
            // check & update call expression with explicit cast to new type
            // e.g. for the following
            //
            // enum Color {Red, Blue, Green, Yellow};
            // function foo(a:Color) {..}
            // function main():void {
            //   foo(1);
            // }
            //
            // so the foo(1) should be translated to foo(Color.Blue)
        } else if (ast->IsIfStatement()) {
            // check & update conditional expression with explicit cast to new type
            // e.g. for the following
            //
            // enum Color {Red, Blue, Green, Yellow};
            // function main():void {
            //   let v: Color = Color.Blue;
            //   if (v) {...}
            // }
            //
            // so the if (v) should be translated to the if (v.getValue() == true) ??
        }

        return ast;
    });

    return true;
}

}  // namespace ark::es2panda::compiler
