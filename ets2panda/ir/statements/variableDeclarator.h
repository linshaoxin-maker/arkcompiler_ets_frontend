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

#ifndef ES2PANDA_IR_STATEMENT_VARIABLE_DECLARATOR_H
#define ES2PANDA_IR_STATEMENT_VARIABLE_DECLARATOR_H

#include "ir/expression.h"
#include "ir/statement.h"
namespace panda::es2panda::checker {
class TSAnalyzer;
}  // namespace panda::es2panda::checker
namespace panda::es2panda::ir {
class Expression;

class VariableDeclarator : public TypedStatement {
public:
    explicit VariableDeclarator(Expression *ident) : TypedStatement(AstNodeType::VARIABLE_DECLARATOR), id_(ident) {}

    explicit VariableDeclarator(Expression *ident, Expression *init)
        : TypedStatement(AstNodeType::VARIABLE_DECLARATOR), id_(ident), init_(init)
    {
    }

    // NOTE (vivienvoros): these friend relationships can be removed once there are getters for private fields
    friend class checker::TSAnalyzer;

    Expression *Init()
    {
        return init_;
    }

    const Expression *Init() const
    {
        return init_;
    }

    Expression *Id()
    {
        return id_;
    }

    const Expression *Id() const
    {
        return id_;
    }

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile([[maybe_unused]] compiler::PandaGen *pg) const override;
    void Compile([[maybe_unused]] compiler::ETSGen *etsg) const override;
    checker::Type *Check([[maybe_unused]] checker::TSChecker *checker) override;
    checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) override;

private:
    Expression *id_;
    Expression *init_ {};
};
}  // namespace panda::es2panda::ir

#endif
