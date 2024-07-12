/**
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

#include "ast_verifier_test.h"
#include "checker/ETSchecker.h"
#include "util/ast-builders/classDefinitionBuilder.h"
#include "util/ast-builders/binaryExpressionBuilder.h"
#include "util/ast-builders/classPropertyBuilder.h"
#include "util/ast-builders/identifierBuilder.h"
#include "util/ast-builders/numberLiteralBuilder.h"
#include "util/ast-builders/classDefinitionBuilder.h"
#include "util/ast-builders/memberExpressionBuilder.h"
#include "util/ast-builders/thisExpressionBuilder.h"
#include "util/ast-builders/etsTypeReferencePartBuilder.h"
#include "util/ast-builders/etsTypeReferenceBuilder.h"
#include "util/ast-builders/expressionStatementBuilder.h"

using ark::es2panda::compiler::ast_verifier::ASTVerifier;
using ark::es2panda::compiler::ast_verifier::InvariantNameSet;
using ark::es2panda::ir::ETSScript;
using ark::es2panda::ir::BinaryExpressionBuilder;
using ark::es2panda::ir::IdentifierBuilder;
using ark::es2panda::ir::NumberLiteralBuilder;
using ark::es2panda::ir::MemberExpressionBuilder;
using ark::es2panda::ir::ThisExpressionBuilder;
using ark::es2panda::ir::ETSTypeReferencePartBuilder;
using ark::es2panda::ir::ETSTypeReferenceBuilder;
using ark::es2panda::ir::ClassPropertyBuilder;
using ark::es2panda::ir::ClassDefinitionBuilder;
using ark::es2panda::ir::ExpressionStatementBuilder;

TEST_F(ASTVerifierTest, ProtectedAccessTestCorrect)
{
    ASTVerifier verifier {Allocator()};

    char const *text = R"()";

    ark::ArenaVector<ark::es2panda::ir::TSClassImplements *> implements(Allocator()->Adapter());
    auto id = IdentifierBuilder(Allocator()).SetName("a").Build();
    auto number = NumberLiteralBuilder(Allocator()).SetValue("1").Build();
    auto classProperty = ClassPropertyBuilder(Allocator())
                             .SetKey(id)
                             .SetValue(number)
                             .AddModifier(ark::es2panda::ir::ModifierFlags::PUBLIC)
                             .Build();

    auto classDef = ClassDefinitionBuilder(Allocator())
                        .SetIdentifier("A")
                        .AddProperty(classProperty)
                        .SetImplements(implements)
                        .Build();

    auto id2 = IdentifierBuilder(Allocator()).SetName("b").Build();
    auto thisExpr = ThisExpressionBuilder(Allocator()).Build();
    auto memberExpr = MemberExpressionBuilder(Allocator()).SetKind(ark::es2panda::ir::MemberExpressionKind::PROPERTY_ACCESS).SetObject(thisExpr).Build();
    auto classProperty2 = ClassPropertyBuilder(Allocator())
                             .SetKey(id2)
                             .SetValue(memberExpr)
                             .AddModifier(ark::es2panda::ir::ModifierFlags::PROTECTED)
                             .Build();
    auto etsTypeRef = ETSTypeReferencePartBuilder(Allocator()).SetName(IdentifierBuilder(Allocator()).SetName("A").Build()).Build();

    auto classDef2 = ClassDefinitionBuilder(Allocator())
                        .SetIdentifier("A")
                        .SetSuperClass(ETSTypeReferenceBuilder(Allocator()).SetETSTypeReferencePart(etsTypeRef).Build())
                        .AddProperty(classProperty2)
                        .SetImplements(implements)
                        .Build();

    es2panda_Context *ctx = impl_->CreateContextFromString(cfg_, text, "dummy.ets");
    impl_->ProceedToState(ctx, ES2PANDA_STATE_CHECKED);
    ASSERT_EQ(impl_->ContextState(ctx), ES2PANDA_STATE_CHECKED);

    auto *ast = reinterpret_cast<ETSScript *>(impl_->ProgramAst(impl_->ContextProgram(ctx)));

    auto exprStmt = ExpressionStatementBuilder(Allocator()).SetExpression(classDef->AsExpression()).Build();
    auto exprStmt2 = ExpressionStatementBuilder(Allocator()).SetExpression(classDef2->AsExpression()).Build();
    ast->AsETSScript()->Statements().emplace_back(exprStmt);
    ast->AsETSScript()->Statements().emplace_back(exprStmt2);

    // ast->AsETSScript()
    //     ->Statements()[1]
    //     ->AsClassDeclaration()
    //     ->Definition()
    //     ->AsClassDefinition()
    //     ->Body()[0]
    //     ->AsClassProperty()
    //     ->AddModifier(ark::es2panda::ir::ModifierFlags::PROTECTED);

    InvariantNameSet checks;
    checks.insert("ModifierAccessValidForAll");
    const auto &messages = verifier.Verify(ast, checks);

    ASSERT_EQ(messages.size(), 0);

    impl_->DestroyContext(ctx);
}
