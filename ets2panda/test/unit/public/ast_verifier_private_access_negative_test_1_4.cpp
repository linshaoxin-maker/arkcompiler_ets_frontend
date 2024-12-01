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
#include "util/ast-builders/classBuilder.h"
#include "util/ast-builders/numberLiteralBuilder.h"
#include "util/ast-builders/identifierBuilder.h"
#include "util/ast-builders/classPropertyBuilder.h"
#include "util/ast-builders/memberExpressionBuilder.h"
#include "util/ast-builders/thisExpressionBuilder.h"
#include "varbinder/variable.h"
#include "varbinder/varbinder.h"
#include "util/ustring.h"

using ark::es2panda::compiler::ast_verifier::ASTVerifier;
using ark::es2panda::compiler::ast_verifier::InvariantNameSet;
using ark::es2panda::ir::ETSScript;

namespace {
using ark::es2panda::ir::ClassDeclaration;
using ark::es2panda::ir::ClassPropertyBuilder;
using ark::es2panda::ir::IdentifierBuilder;
using ark::es2panda::ir::MemberExpression;
using ark::es2panda::ir::MemberExpressionBuilder;
using ark::es2panda::ir::MemberExpressionKind;
using ark::es2panda::ir::ModifierFlags;
using ark::es2panda::ir::NumberLiteralBuilder;
using ark::es2panda::ir::ThisExpressionBuilder;
using ark::es2panda::util::StringView;

MemberExpression *CreateMemberExpression(ark::ArenaAllocator *allocator, StringView name,
                                         ark::es2panda::ir::Expression *property)
{
    auto decl = allocator->New<ark::es2panda::varbinder::PropertyDecl>(name);
    decl->BindNode(property);
    auto var = allocator->New<ark::es2panda::varbinder::LocalVariable>(
        decl, ark::es2panda::varbinder::VariableFlags::PROPERTY | ark::es2panda::varbinder::VariableFlags::PRIVATE |
                  ark::es2panda::varbinder::VariableFlags::LEXICAL |
                  ark::es2panda::varbinder::VariableFlags::INITIALIZED);
    auto ident = IdentifierBuilder(allocator).SetName(name).Build();
    ident->SetVariable(var);
    return MemberExpressionBuilder(allocator)
        .SetKind(MemberExpressionKind::PROPERTY_ACCESS)
        .SetProperty(ident)
        .SetObject(ThisExpressionBuilder(allocator).Build())
        .Build();
}

ClassDeclaration *CreateBaseClassWithPrivateProperty(ark::ArenaAllocator *allocator)
{
    auto prop = ClassPropertyBuilder(allocator)
                    .SetKey(IdentifierBuilder(allocator).SetName("a").Build())
                    .SetValue(NumberLiteralBuilder(allocator).SetValue("1").Build())
                    .AddModifier(ModifierFlags::PRIVATE)
                    .Build();
    return ark::es2panda::ir::ClassBuilder(allocator).SetId(StringView("Base")).AddProperty(prop).Build();
}

ClassDeclaration *CreateDerivedClassWithPrivateProperty(ark::ArenaAllocator *allocator,
                                                        ark::es2panda::ir::Expression *property = nullptr)
{
    auto prop = ClassPropertyBuilder(allocator)
                    .SetKey(IdentifierBuilder(allocator).SetName("b").Build())
                    .SetValue(CreateMemberExpression(allocator, StringView("a"), property))
                    .AddModifier(ModifierFlags::PUBLIC)
                    .Build();
    auto derivedClassBuilder = ark::es2panda::ir::ClassBuilder(allocator)
                                   .SetId(StringView("Derived"))
                                   .SetSuperClass(StringView("Base"))
                                   .AddProperty(prop)
                                   .Build();
    return derivedClassBuilder;
}
}  // namespace

TEST_F(ASTVerifierTest, PrivateAccessTestNegative1)
{
    ASTVerifier verifier {Allocator()};

    char const *text = R"(
        class Base {
            public a: int = 1;
        }
        class Derived extends Base {
            public b: int = this.a;
        }
    )";
    auto cls = CreateBaseClassWithPrivateProperty(Allocator());
    auto derivedCls = CreateDerivedClassWithPrivateProperty(
        Allocator(),
        cls->AsClassDeclaration()->Definition()->AsClassDefinition()->Body()[0]->AsClassProperty()->Value());

    es2panda_Context *ctx = impl_->CreateContextFromString(cfg_, text, "dummy.sts");
    impl_->ProceedToState(ctx, ES2PANDA_STATE_CHECKED);
    ASSERT_EQ(impl_->ContextState(ctx), ES2PANDA_STATE_CHECKED);

    auto *ast = reinterpret_cast<ETSScript *>(impl_->ProgramAst(impl_->ContextProgram(ctx)));

    cls->SetParent(ast);
    derivedCls->SetParent(ast);
    ark::ArenaVector<ark::es2panda::ir::Statement *> statement(Allocator()->Adapter());
    statement.emplace_back(ast->AsETSScript()->Statements()[0]);
    statement.emplace_back(cls);
    statement.emplace_back(derivedCls);

    ast->AsETSScript()->Statements() = statement;

    InvariantNameSet checks;
    checks.insert("ModifierAccessValidForAll");
    const auto &messages = verifier.Verify(ast, checks);
    ASSERT_EQ(messages.size(), 1);

    ASSERT_NE(checks.find(messages[0].Invariant()), checks.end());

    impl_->DestroyContext(ctx);
}

TEST_F(ASTVerifierTest, PrivateAccessTestNegative2)
{
    ASTVerifier verifier {Allocator()};

    char const *text = R"(
        class Base {
            public a: int = 1;
        }
        function main(): void {
            let base: Base = new Base();
            let a = base.a;
        }
    )";
    es2panda_Context *ctx = impl_->CreateContextFromString(cfg_, text, "dummy.sts");
    impl_->ProceedToState(ctx, ES2PANDA_STATE_CHECKED);
    ASSERT_EQ(impl_->ContextState(ctx), ES2PANDA_STATE_CHECKED);

    auto *ast = reinterpret_cast<ETSScript *>(impl_->ProgramAst(impl_->ContextProgram(ctx)));

    ast->AsETSScript()
        ->Statements()[1]
        ->AsClassDeclaration()
        ->Definition()
        ->AsClassDefinition()
        ->Body()[0]
        ->AsClassProperty()
        ->AddModifier(ark::es2panda::ir::ModifierFlags::PRIVATE);

    InvariantNameSet checks;
    checks.insert("ModifierAccessValidForAll");
    const auto &messages = verifier.Verify(ast, checks);
    ASSERT_EQ(messages.size(), 1);

    ASSERT_NE(checks.find(messages[0].Invariant()), checks.end());

    impl_->DestroyContext(ctx);
}

TEST_F(ASTVerifierTest, PrivateAccessTestNegative3)
{
    ASTVerifier verifier {Allocator()};

    char const *text = R"(
        class Base {
            public a: int = 1;
        }
        class Derived extends Base {}
        function main(): void {
            let derived: Derived = new Derived();
            let a = derived.a;
        }
    )";
    es2panda_Context *ctx = impl_->CreateContextFromString(cfg_, text, "dummy.sts");
    impl_->ProceedToState(ctx, ES2PANDA_STATE_CHECKED);
    ASSERT_EQ(impl_->ContextState(ctx), ES2PANDA_STATE_CHECKED);

    auto *ast = reinterpret_cast<ETSScript *>(impl_->ProgramAst(impl_->ContextProgram(ctx)));

    ast->AsETSScript()
        ->Statements()[1]
        ->AsClassDeclaration()
        ->Definition()
        ->AsClassDefinition()
        ->Body()[0]
        ->AsClassProperty()
        ->AddModifier(ark::es2panda::ir::ModifierFlags::PRIVATE);

    InvariantNameSet checks;
    checks.insert("ModifierAccessValidForAll");
    const auto &messages = verifier.Verify(ast, checks);
    ASSERT_EQ(messages.size(), 1);

    ASSERT_NE(checks.find(messages[0].Invariant()), checks.end());

    impl_->DestroyContext(ctx);
}

TEST_F(ASTVerifierTest, PrivateAccessTestNegative4)
{
    ASTVerifier verifier {Allocator()};

    char const *text = R"(
        class Base {
            public a: int = 1;
        }
        class Derived extends Base {}
        function main(): void {
            let derived: Base = new Derived();
            let a = derived.a;
        }
    )";
    es2panda_Context *ctx = impl_->CreateContextFromString(cfg_, text, "dummy.sts");
    impl_->ProceedToState(ctx, ES2PANDA_STATE_CHECKED);
    ASSERT_EQ(impl_->ContextState(ctx), ES2PANDA_STATE_CHECKED);

    auto *ast = reinterpret_cast<ETSScript *>(impl_->ProgramAst(impl_->ContextProgram(ctx)));

    ast->AsETSScript()
        ->Statements()[1]
        ->AsClassDeclaration()
        ->Definition()
        ->AsClassDefinition()
        ->Body()[0]
        ->AsClassProperty()
        ->AddModifier(ark::es2panda::ir::ModifierFlags::PRIVATE);

    InvariantNameSet checks;
    checks.insert("ModifierAccessValidForAll");
    const auto &messages = verifier.Verify(ast, checks);
    ASSERT_EQ(messages.size(), 1);

    ASSERT_NE(checks.find(messages[0].Invariant()), checks.end());

    impl_->DestroyContext(ctx);
}
