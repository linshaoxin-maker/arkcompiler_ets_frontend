/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <algorithm>
#include "macros.h"

#include "compiler/core/ASTVerifier.h"
#include "ir/astDump.h"
#include "ir/expressions/literals/stringLiteral.h"
#include "ir/expressions/identifier.h"

class ASTVerifierTest : public testing::Test {
public:
    ASTVerifierTest() = default;
    ~ASTVerifierTest() override = default;

    NO_COPY_SEMANTIC(ASTVerifierTest);
    NO_MOVE_SEMANTIC(ASTVerifierTest);

private:
};

TEST_F(ASTVerifierTest, NullParent)
{
    panda::es2panda::compiler::ASTVerifier verifier {};
    panda::es2panda::ir::StringLiteral empty_node;

    bool has_parent = verifier.HasParent(&empty_node);
    auto messages = verifier.GetErrorMessages();

    ASSERT_EQ(has_parent, false);
    ASSERT_NE(messages.size(), 0);
    ASSERT_EQ(messages[0], "NULL_PARENT: STR_LITERAL <null>");
}

TEST_F(ASTVerifierTest, NullType)
{
    panda::es2panda::compiler::ASTVerifier verifier {};
    panda::es2panda::ir::StringLiteral empty_node;

    bool has_type = verifier.HasType(&empty_node);
    auto messages = verifier.GetErrorMessages();

    ASSERT_EQ(has_type, false);
    ASSERT_NE(messages.size(), 0);
    ASSERT_EQ(messages[0], "NULL_TS_TYPE: STR_LITERAL <null>");
}

TEST_F(ASTVerifierTest, WithoutScope)
{
    panda::es2panda::compiler::ASTVerifier verifier {};
    panda::es2panda::ir::StringLiteral empty_node;

    bool has_scope = verifier.HasScope(&empty_node);
    auto messages = verifier.GetErrorMessages();

    ASSERT_EQ(has_scope, true);
    ASSERT_EQ(messages.size(), 0);
}

TEST_F(ASTVerifierTest, ScopeTest)
{
    const auto memory_sz = 64'000'000;
    panda::mem::MemConfig::Initialize(0, memory_sz, memory_sz, memory_sz, 0, 0);
    panda::PoolManager::Initialize();
    auto alloc = std::make_unique<panda::ArenaAllocator>(panda::SpaceType::SPACE_TYPE_INTERNAL);

    panda::es2panda::compiler::ASTVerifier verifier {};
    panda::es2panda::ir::Identifier ident(panda::es2panda::util::StringView("var_decl"), alloc.get());
    panda::es2panda::binder::LetDecl decl("test", &ident);
    panda::es2panda::binder::LocalVariable local(&decl, panda::es2panda::binder::VariableFlags::LOCAL);
    ident.SetVariable(&local);

    panda::es2panda::binder::LocalScope scope(alloc.get(), nullptr);
    scope.AddDecl(alloc.get(), &decl, panda::es2panda::ScriptExtension::ETS);
    scope.BindNode(&ident);

    local.SetScope(&scope);

    bool is_ok = verifier.ScopeEncloseVariable(&local);

    ASSERT_EQ(is_ok, true);
}
