/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "enumMemberDefaultValueLowering.h"

#include "checker/ETSchecker.h"
#include "ir/astNode.h"
#include "compiler/core/compilerContext.h"

namespace ark::es2panda::compiler {

namespace {
uint32_t GetMemberValue(size_t index, ArenaVector<ir::AstNode *> &members)
{
    if (index == 0) {
        return 0;
    }
    return members[index - 1]->AsTSEnumMember()->Init()->AsNumberLiteral()->Number().GetInt() + 1;
}
}  // namespace

bool EnumMemberDefaultValueLowering::Perform(public_lib::Context *const ctx, parser::Program *const program)
{
    for (auto &[_, ext_programs] : program->ExternalSources()) {
        for (auto *extProg : ext_programs) {
            Perform(ctx, extProg);
        }
    }

    checker::ETSChecker *checker = ctx->checker->AsETSChecker();
    program->Ast()->TransformChildrenRecursively([checker](ir::AstNode *ast) -> ir::AstNode* {
        UNUSED_VAR(checker);

        if (!ast->IsTSEnumDeclaration()) {
            return ast;
        }
        auto enumMembers = ast->AsTSEnumDeclaration()->Members();
        for (size_t i = 0; i < enumMembers.size(); i++) {
            if (!enumMembers[i]->AsTSEnumMember()->GetIsAssignment()) {
                auto *allocator = checker->Allocator();
                auto number = allocator->New<ark::es2panda::ir::NumberLiteral>(
                    ark::es2panda::lexer::Number(GetMemberValue(i, enumMembers)));
                number->SetParent(ast);
                enumMembers[i]->AsTSEnumMember()->SetInit(number);
            }
        }
        return ast;
    });

    return true;
}

}  // namespace ark::es2panda::compiler