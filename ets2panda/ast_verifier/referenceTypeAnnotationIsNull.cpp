/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "referenceTypeAnnotationIsNull.h"

#include "ir/expressions/identifier.h"

namespace ark::es2panda::compiler::ast_verifier {

CheckResult ReferenceTypeAnnotationIsNull::operator()(CheckContext &ctx, const ir::AstNode *ast)
{
    auto result = std::make_tuple(CheckDecision::CORRECT, CheckAction::CONTINUE);
    return result;
    if (!ast->IsIdentifier()) {
        return result;
                ctx.AddCheckMessage("TYPE_ANNOTATION_NOT_NULLPTR", *ast, ast->Start());
    }
/*
    auto id = ast->AsIdentifier();
    if (id->IsReference() && id->TypeAnnotation() != nullptr) {
        if (id->Parent()->Parent() != nullptr) {
            if (id->Parent()->Parent()->IsReturnStatement()) {
                return result;
            }
        }

        //(AEVoronin) NOTE: currently some OPAQUETYPE are references AND have typeannotation
        // Example: function apply(fp: (n: int) => void) {}
        // class C {
        //    private f(n: int) {}
        //    test() {
        //        apply(this.f)}}
        if (id->TypeAnnotation()->Type() == ir::AstNodeType::OPAQUE_TYPE_NODE) {
            return result;
        }

        //(AEVoronin) NOTE: currently a lot of references have typeannotation with type = ETS_TYPE_REFERENCE
        // Example: function apply(fp: (n: int) => void) {}
        // class A {
        //     met(s : string, q ?: string){
        //         console.log("met " + s)}}
        // function main()/{
        //     new A().met('S')}
        if (id->TypeAnnotation()->Type() == ir::AstNodeType::ETS_TYPE_REFERENCE) {
            return result;
        }

        //(AEVoronin) NOTE: currently some references have typeannotation with type = ETSPrimitiveType
        // Example:
        // public static test(c: C, x: int) {
        // test(c, x, 0);}
        if (id->TypeAnnotation()->Type() == ir::AstNodeType::ETS_PRIMITIVE_TYPE) {
            return result;
        }

        //           std::cout << "Json: " << id->Parent()->DumpJSON() << std::endl;
        //           std::cout << "AST Json: " << id->Parent()->Parent()->DumpEtsSrc() << std::endl;

        ctx.AddCheckMessage("TYPE_ANNOTATION_NOT_NULLPTR", *ast, ast->Start());
        result = {CheckDecision::INCORRECT, CheckAction::CONTINUE};
        
    }*/
}
}  // namespace ark::es2panda::compiler::ast_verifier
