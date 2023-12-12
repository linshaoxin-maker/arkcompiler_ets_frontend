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

#ifndef ES2PANDA_IR_TS_AS_EXPRESSION_H
#define ES2PANDA_IR_TS_AS_EXPRESSION_H

#include "ir/astDump.h"
#include "ir/expression.h"
namespace panda::es2panda::checker {
class ETSAnalyzer;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::compiler {
class ETSCompiler;
}  // namespace panda::es2panda::compiler
namespace panda::es2panda::ir {
class TSAsExpression : public AnnotatedExpression {
public:
    explicit TSAsExpression(Expression *expression, TypeNode *type_annotation, bool is_const)
        : AnnotatedExpression(AstNodeType::TS_AS_EXPRESSION, type_annotation),
          expression_(expression),
          is_const_(is_const)
    {
    }
    // NOTE (vivienvoros): these friend relationships can be removed once there are getters for private fields
    friend class checker::ETSAnalyzer;
    friend class compiler::ETSCompiler;
    const Expression *Expr() const
    {
        return expression_;
    }

    Expression *Expr();
    void SetExpr(Expression *expr);

    bool IsConst() const
    {
        return is_const_;
    }

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile([[maybe_unused]] compiler::PandaGen *pg) const override;
    void Compile([[maybe_unused]] compiler::ETSGen *etsg) const override;
    checker::Type *Check([[maybe_unused]] checker::TSChecker *checker) override;
    checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) override;

    void Accept(ASTVisitorT *v) override
    {
        v->Accept(this);
    }

private:
    Expression *expression_;
    bool is_unchecked_cast_ {true};
    bool is_const_;
};
}  // namespace panda::es2panda::ir

#endif
