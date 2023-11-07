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
#include "ir/expressions/literals/booleanLiteral.h"
#include "ir/expressions/literals/numberLiteral.h"
#include "macros.h"

#include "compiler/core/ASTVerifier.h"
#include "parser/ETSparser.h"
#include "ir/astDump.h"
#include "ir/expressions/literals/stringLiteral.h"
#include "checker/ETSchecker.h"
#include "varbinder/ETSBinder.h"

class ASTVerifierTest : public testing::Test {
public:
    ASTVerifierTest()
    {
        allocator_ = std::make_unique<panda::ArenaAllocator>(panda::SpaceType::SPACE_TYPE_COMPILER);
    }
    ~ASTVerifierTest() override = default;

    static void SetUpTestCase()
    {
        constexpr auto COMPILER_SIZE = panda::operator""_MB(256ULL);
        panda::mem::MemConfig::Initialize(0, 0, COMPILER_SIZE, 0, 0, 0);
        panda::PoolManager::Initialize();
    }

    panda::ArenaAllocator *GetAllocator()
    {
        return allocator_.get();
    }

    NO_COPY_SEMANTIC(ASTVerifierTest);
    NO_MOVE_SEMANTIC(ASTVerifierTest);

private:
    std::unique_ptr<panda::ArenaAllocator> allocator_;
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

TEST_F(ASTVerifierTest, ArithmeticExpressionCorrect1)
{
    panda::es2panda::checker::ETSChecker etschecker {};
    panda::es2panda::compiler::ASTVerifier verifier {};

    auto left = panda::es2panda::ir::NumberLiteral(panda::es2panda::lexer::Number {1});
    auto right = panda::es2panda::ir::NumberLiteral(panda::es2panda::lexer::Number {6});
    auto arithmetic_expression =
        panda::es2panda::ir::BinaryExpression(&left, &right, panda::es2panda::lexer::TokenType::PUNCTUATOR_PLUS);

    left.SetTsType(etschecker.GlobalIntType());
    right.SetTsType(etschecker.GlobalIntType());

    bool is_correct = verifier.CheckArithmeticExpressions(&arithmetic_expression);
    ASSERT_EQ(is_correct, true);
}

TEST_F(ASTVerifierTest, ArithmeticExpressionCorrect2)
{
    panda::es2panda::checker::ETSChecker etschecker {};
    panda::es2panda::compiler::ASTVerifier verifier {};

    auto left1 = panda::es2panda::ir::NumberLiteral(panda::es2panda::lexer::Number {1});
    auto left2 = panda::es2panda::ir::NumberLiteral(panda::es2panda::lexer::Number {12});
    auto right2 = panda::es2panda::ir::NumberLiteral(panda::es2panda::lexer::Number {6});
    auto right1 =
        panda::es2panda::ir::BinaryExpression(&left2, &right2, panda::es2panda::lexer::TokenType::PUNCTUATOR_MULTIPLY);
    auto arithmetic_expression =
        panda::es2panda::ir::BinaryExpression(&left1, &right1, panda::es2panda::lexer::TokenType::PUNCTUATOR_PLUS);

    left1.SetTsType(etschecker.GlobalIntType());
    right1.SetTsType(etschecker.GlobalIntType());
    left2.SetTsType(etschecker.GlobalIntType());
    right2.SetTsType(etschecker.GlobalIntType());

    bool is_correct = verifier.CheckArithmeticExpressions(&arithmetic_expression);
    ASSERT_EQ(is_correct, true);
}

TEST_F(ASTVerifierTest, ArithmeticExpressionNegative1)
{
    panda::es2panda::checker::ETSChecker etschecker {};
    panda::es2panda::compiler::ASTVerifier verifier {};
    auto left = panda::es2panda::ir::StringLiteral("1");
    auto right = panda::es2panda::ir::NumberLiteral(panda::es2panda::lexer::Number {1});
    auto arithmetic_expression =
        panda::es2panda::ir::BinaryExpression(&left, &right, panda::es2panda::lexer::TokenType::PUNCTUATOR_DIVIDE);

    left.SetTsType(etschecker.GlobalETSStringLiteralType());
    right.SetTsType(etschecker.GlobalIntType());

    bool is_correct = verifier.CheckArithmeticExpressions(&arithmetic_expression);
    ASSERT_EQ(is_correct, false);
}

TEST_F(ASTVerifierTest, ArithmeticExpressionNegative2)
{
    panda::es2panda::checker::ETSChecker etschecker {};
    panda::es2panda::compiler::ASTVerifier verifier {};
    auto left = panda::es2panda::ir::BooleanLiteral(true);
    auto right = panda::es2panda::ir::NumberLiteral(panda::es2panda::lexer::Number {1});
    auto arithmetic_expression =
        panda::es2panda::ir::BinaryExpression(&left, &right, panda::es2panda::lexer::TokenType::PUNCTUATOR_DIVIDE);

    left.SetTsType(etschecker.GlobalETSStringLiteralType());
    right.SetTsType(etschecker.GlobalIntType());

    bool is_correct = verifier.CheckArithmeticExpressions(&arithmetic_expression);
    ASSERT_EQ(is_correct, false);
}