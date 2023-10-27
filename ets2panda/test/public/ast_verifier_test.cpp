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
#include "checker/ETSAnalyzer.h"
#include "ir/expressions/literals/numberLiteral.h"
#include "ir/expressions/sequenceExpression.h"
#include "macros.h"

#include "compiler/core/ASTVerifier.h"
#include "varbinder/ETSBinder.h"
#include "checker/ETSchecker.h"
#include "ir/astDump.h"
#include "ir/expressions/literals/stringLiteral.h"

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define TREE(node)                           \
    ([&]() {                                 \
        using namespace panda::es2panda::ir; \
        return node;                         \
    }())

#define NODE(Type, ...) allocator->New<Type>(__VA_ARGS__)
#define NODES(Type, ...)                                     \
    ([&]() -> ArenaVector<Type *> {                          \
        auto v = ArenaVector<Type *> {allocator->Adapter()}; \
        v.insert(v.end(), {__VA_ARGS__});                    \
        return v;                                            \
    }())
// NOLINTEND(cppcoreguidelines-macro-usage)

namespace panda::es2panda {

class ASTVerifierTest : public testing::Test {
public:
    ASTVerifierTest()
    {
        allocator_ = std::make_unique<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER);
    }
    ~ASTVerifierTest() override = default;

    static void SetUpTestCase()
    {
        constexpr auto COMPILER_SIZE = 256_MB;

        mem::MemConfig::Initialize(0, 0, COMPILER_SIZE, 0, 0, 0);
        PoolManager::Initialize();
    }

    ArenaAllocator *Allocator()
    {
        return allocator_.get();
    }

    NO_COPY_SEMANTIC(ASTVerifierTest);
    NO_MOVE_SEMANTIC(ASTVerifierTest);

private:
    std::unique_ptr<ArenaAllocator> allocator_;
};

TEST_F(ASTVerifierTest, NullParent)
{
    compiler::ASTVerifier verifier {Allocator()};
    ir::StringLiteral empty_node;

    auto checks = compiler::ASTVerifier::CheckSet {Allocator()->Adapter()};
    checks.insert("HasParent");
    bool has_parent = verifier.Verify(&empty_node, checks);
    const auto &errors = verifier.GetErrors();
    const auto [name, error] = errors[0];

    ASSERT_EQ(has_parent, false);
    ASSERT_NE(errors.size(), 0);
    ASSERT_EQ(name, "HasParent");
    ASSERT_EQ(error.message, "NULL_PARENT: STR_LITERAL <null>");
}

TEST_F(ASTVerifierTest, NullType)
{
    compiler::ASTVerifier verifier {Allocator()};
    ir::StringLiteral empty_node;

    auto checks = compiler::ASTVerifier::CheckSet {Allocator()->Adapter()};
    checks.insert("HasType");
    bool has_type = verifier.Verify(&empty_node, checks);
    const auto &errors = verifier.GetErrors();
    const auto [name, error] = errors[0];

    ASSERT_EQ(has_type, false);
    ASSERT_NE(errors.size(), 0);
    ASSERT_EQ(name, "HasType");
    ASSERT_EQ(error.message, "NULL_TS_TYPE: STR_LITERAL <null>");
}

TEST_F(ASTVerifierTest, WithoutScope)
{
    compiler::ASTVerifier verifier {Allocator()};
    ir::StringLiteral empty_node;

    auto checks = compiler::ASTVerifier::CheckSet {Allocator()->Adapter()};
    checks.insert("HasScope");
    bool has_scope = verifier.Verify(&empty_node, checks);
    const auto &errors = verifier.GetErrors();

    ASSERT_EQ(has_scope, true);
    ASSERT_EQ(errors.size(), 0);
}

}  // namespace panda::es2panda
