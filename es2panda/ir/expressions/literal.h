/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_IR_EXPRESSION_LITERAL_H
#define ES2PANDA_IR_EXPRESSION_LITERAL_H

#include <ir/expression.h>

namespace panda::es2panda::compiler {
class PandaGen;
}  // namespace panda::es2panda::compiler

namespace panda::es2panda::checker {
class Checker;
class Type;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::ir {

// must be kept in sync with panda::panda_file::LiteralTag
enum class LiteralTag {
    TAGVALUE = 0,
    BOOLEAN = 1,
    INTEGER = 2,
    FLOAT,
    DOUBLE = 4,
    STRING = 5,
    METHOD = 6,
    GENERATOR_METHOD,
    ACCESSOR = 8,
    METHODAFFILIATE = 9,
    ASYNC_GENERATOR_METHOD,
    NULL_VALUE = 255,
};

class Literal : public Expression {
public:
    bool IsLiteral() const override
    {
        return true;
    }

    virtual LiteralTag Tag() const = 0;

    bool GetBoolean() const;
    uint32_t GetInt() const;
    double GetDouble() const;
    const util::StringView &GetString() const;
    const util::StringView &GetMethod() const;
    uint16_t GetMethodAffiliate() const;

protected:
    explicit Literal(AstNodeType type) : Expression(type) {}
};

}  // namespace panda::es2panda::ir

#endif
