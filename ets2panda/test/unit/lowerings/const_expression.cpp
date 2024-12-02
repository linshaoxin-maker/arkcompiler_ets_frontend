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

#include "lowering_test.h"
#include "compiler/lowering/ets/constantExpressionLowering.h"

namespace ark::es2panda {

TEST_F(LoweringTest, TestConstantExpressionConcat)
{
    char const *text = R"(
        @interface MyAnno {
            a : int
            b : long
            c : boolean
            d : int
        }
        @MyAnno({a = 1 + -1, b = 1554.4 ^ 10, c = 12 >= 10, d = 15 != 10 ? 1+4 : 5+1})
        function foo() {}
    )";
    int const expectA = 0;
    int64_t const expectB = 1560;
    bool const expectC = true;
    int const expectD = 5;

    ir::AstNode *const ast = SetupContext(text, ES2PANDA_STATE_CHECKED)->Ast();

    ASSERT_FALSE(ast->IsAnyChild([](ir::AstNode *const node) {
        return node->IsBinaryExpression() || node->IsUnaryExpression() || node->IsConditionalExpression();
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<int32_t>()) {
                return numNode.GetInt() == expectA;
            }
        }
        return false;
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<int64_t>()) {
                return numNode.GetLong() == expectB;
            }
        }
        return false;
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        return node->IsBooleanLiteral() && node->AsBooleanLiteral()->Value() == expectC;
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<int32_t>()) {
                return numNode.GetInt() == expectD;
            }
        }
        return false;
    }));
}

TEST_F(LoweringTest, TestConstantExpressionConcatNumeric)
{
    char const *text = R"(
        @interface MyAnno {
            a : long
        }
        @MyAnno({a = ((((1 + -1 + 10) * 123 / 5) ^ (~10.2) << 1) >> 2.6 >>> 33 & 141 | 12) % 53})
        function foo() {}
    )";

    int64_t const expectA = 35;
    ir::AstNode *const ast = SetupContext(text, ES2PANDA_STATE_CHECKED)->Ast();

    ASSERT_FALSE(ast->IsAnyChild([](ir::AstNode *const node) {
        return node->IsBinaryExpression() || node->IsUnaryExpression() || node->IsConditionalExpression();
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        return node->IsNumberLiteral() && node->AsNumberLiteral()->Number().GetLong() == expectA;
    }));
}

TEST_F(LoweringTest, TestConstantExpressionConcatNumeric1)
{
    char const *text = R"(
        @interface MyAnno {
            a : int: 1+1+1+1
        }
        @MyAnno()
        function foo() {}
    )";

    int32_t const expectA = 4;
    ir::AstNode *const ast = SetupContext(text, ES2PANDA_STATE_CHECKED)->Ast();

    ASSERT_FALSE(ast->IsAnyChild([](ir::AstNode *const node) {
        return node->IsBinaryExpression() || node->IsUnaryExpression() || node->IsConditionalExpression();
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        return node->IsNumberLiteral() && node->AsNumberLiteral()->Number().GetInt() == expectA;
    }));
}

TEST_F(LoweringTest, TestConstantExpressionConcatBoolean)
{
    char const *text = R"(
        @interface MyAnno {
            a : boolean
        }
        @MyAnno({a = ((20<=10) || true) ? !false && (10==10) && (30 != 10): false})
        function foo() {}
    )";

    bool const expectA = true;
    ir::AstNode *const ast = SetupContext(text, ES2PANDA_STATE_CHECKED)->Ast();

    ASSERT_FALSE(ast->IsAnyChild([](ir::AstNode *const node) {
        return node->IsBinaryExpression() || node->IsUnaryExpression() || node->IsConditionalExpression();
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        return node->IsBooleanLiteral() && node->AsBooleanLiteral()->Value() == expectA;
    }));
}

TEST_F(LoweringTest, TestConstantExpressionConcatExtendedBoolean1)
{
    char const *text = R"(
        @interface MyAnno {
            a : int
            c : int
            d : int
        }
        @MyAnno({a = null ? 1 : 0, c = "a" ? 5 : 4, d = 12 ? 7 : 6})
        function foo() {}
    )";

    int const expectA = 0;
    int const expectC = 5;
    int const expectD = 7;
    ir::AstNode *const ast = SetupContext(text, ES2PANDA_STATE_CHECKED)->Ast();

    ASSERT_FALSE(ast->IsAnyChild([](ir::AstNode *const node) {
        return node->IsBinaryExpression() || node->IsUnaryExpression() || node->IsConditionalExpression();
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<int32_t>()) {
                return numNode.GetInt() == expectA;
            }
        }
        return false;
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<int32_t>()) {
                return numNode.GetInt() == expectC;
            }
        }
        return false;
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<int32_t>()) {
                return numNode.GetInt() == expectD;
            }
        }
        return false;
    }));
}

TEST_F(LoweringTest, TestConstantExpressionConcatExtendedBoolean2)
{
    char const *text = R"(
        @interface MyAnno {
            a : int
            c : int
            d : int
        }
        @MyAnno({a = undefined ? 1 : 0, c = "" ? 5 : 4, d = 0 ? 7 : 6})
        function foo() {}
    )";

    int const expectA = 0;
    int const expectC = 4;
    int const expectD = 6;
    ir::AstNode *const ast = SetupContext(text, ES2PANDA_STATE_CHECKED)->Ast();

    ASSERT_FALSE(ast->IsAnyChild([](ir::AstNode *const node) {
        return node->IsBinaryExpression() || node->IsUnaryExpression() || node->IsConditionalExpression();
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<int32_t>()) {
                return numNode.GetInt() == expectA;
            }
        }
        return false;
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<int32_t>()) {
                return numNode.GetInt() == expectC;
            }
        }
        return false;
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<int32_t>()) {
                return numNode.GetInt() == expectD;
            }
        }
        return false;
    }));
}

TEST_F(LoweringTest, TestConstantExpressionConcatExtendedBoolean3)
{
    char const *text = R"(
        @interface MyAnno {
            a : int
            b : double
            c : int
        }
        @MyAnno({a = 12.1 && 3, b = 12.2 || 3, c = true && 0})
        function foo() {}
    )";

    int const expectA = 3;
    double const expectB = 12.2;
    int const expectC = 0;
    ir::AstNode *const ast = SetupContext(text, ES2PANDA_STATE_CHECKED)->Ast();

    ASSERT_FALSE(ast->IsAnyChild([](ir::AstNode *const node) {
        return node->IsBinaryExpression() || node->IsUnaryExpression() || node->IsConditionalExpression();
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<int32_t>()) {
                return numNode.GetInt() == expectA;
            }
        }
        return false;
    }));

    ASSERT_TRUE(ast->IsAnyChild([expectB](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<double>()) {
                return numNode.GetDouble() == expectB;
            }
        }
        return false;
    }));

    ASSERT_TRUE(ast->IsAnyChild([](ir::AstNode *const node) {
        if (node->IsNumberLiteral()) {
            auto numNode = node->AsNumberLiteral()->Number();
            if (numNode.CanGetValue<int32_t>()) {
                return numNode.GetInt() == expectC;
            }
        }
        return false;
    }));
}

}  // namespace ark::es2panda
