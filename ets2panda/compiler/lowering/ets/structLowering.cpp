/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "structLowering.h"
#include "checker/ETSchecker.h"
#include "compiler/core/compilerContext.h"
#include "ir/base/classDefinition.h"
#include "ir/base/classProperty.h"
#include "ir/astNode.h"
#include "ir/expression.h"
#include "ir/opaqueTypeNode.h"
#include "ir/expressions/identifier.h"
#include "ir/statements/classDeclaration.h"
#include "ir/ts/tsAsExpression.h"
#include "type_helper.h"

namespace panda::es2panda::compiler {

const char *const STRUCT_CLASS_NAME = "CommonStruct0";

std::string_view StructLowering::Name()
{
    static std::string const NAME = "struct-class-extention";
    return NAME;
}

ir::ETSTypeReference *CreateStructTypeReference(checker::ETSChecker *checker,
                                                ir::ETSStructDeclaration *ets_struc_declaration)
{
    auto *allocator = checker->Allocator();

    ArenaVector<ir::TypeNode *> params(allocator->Adapter());

    ir::TSTypeParameterInstantiation *type_param_self_inst = nullptr;

    if (ets_struc_declaration->Definition()->TypeParams() != nullptr &&
        !ets_struc_declaration->Definition()->TypeParams()->Params().empty()) {
        ArenaVector<ir::TypeNode *> self_params(allocator->Adapter());
        ir::ETSTypeReferencePart *reference_part = nullptr;

        for (const auto &param : ets_struc_declaration->Definition()->TypeParams()->Params()) {
            auto *ident_ref = checker->AllocNode<ir::Identifier>(param->AsTSTypeParameter()->Name()->Name(), allocator);
            ident_ref->AsIdentifier()->SetReference();

            reference_part = checker->AllocNode<ir::ETSTypeReferencePart>(ident_ref, nullptr, nullptr);

            auto *type_reference = checker->AllocNode<ir::ETSTypeReference>(reference_part);

            self_params.push_back(type_reference);
        }

        type_param_self_inst = checker->AllocNode<ir::TSTypeParameterInstantiation>(std::move(self_params));
    }

    auto *ident_self_ref =
        checker->AllocNode<ir::Identifier>(ets_struc_declaration->Definition()->Ident()->Name(), allocator);
    ident_self_ref->AsIdentifier()->SetReference();

    auto *reference_self_part =
        checker->AllocNode<ir::ETSTypeReferencePart>(ident_self_ref, type_param_self_inst, nullptr);

    auto *self_type_reference = checker->AllocNode<ir::ETSTypeReference>(reference_self_part);

    params.push_back(self_type_reference);

    auto *type_param_inst = checker->AllocNode<ir::TSTypeParameterInstantiation>(std::move(params));

    auto *ident_ref = checker->AllocNode<ir::Identifier>(util::StringView(STRUCT_CLASS_NAME), allocator);
    ident_ref->AsIdentifier()->SetReference();
    auto *reference_part = checker->AllocNode<ir::ETSTypeReferencePart>(ident_ref, type_param_inst, nullptr);

    auto *type_reference = checker->AllocNode<ir::ETSTypeReference>(reference_part);

    return type_reference;
}

bool StructLowering::Perform(public_lib::Context *ctx, parser::Program *program)
{
    for (auto &[_, ext_programs] : program->ExternalSources()) {
        (void)_;
        for (auto *ext_prog : ext_programs) {
            Perform(ctx, ext_prog);
        }
    }

    checker::ETSChecker *checker = ctx->checker->AsETSChecker();

    program->Ast()->TransformChildrenRecursively([checker](ir::AstNode *ast) -> ir::AstNode * {
        if (ast->IsETSStructDeclaration()) {
            auto *type_ref = CreateStructTypeReference(checker, ast->AsETSStructDeclaration());
            ast->AsETSStructDeclaration()->Definition()->SetSuper(type_ref);
            ast->AsETSStructDeclaration()->Definition()->AddModifier(ir::ModifierFlags::FINAL);
        }

        return ast;
    });

    return true;
}

}  // namespace panda::es2panda::compiler
