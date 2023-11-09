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

#ifndef ES2PANDA_IR_STATEMENT_FOR_UPDATE_STATEMENT_H
#define ES2PANDA_IR_STATEMENT_FOR_UPDATE_STATEMENT_H

#include "ir/statements/loopStatement.h"

namespace panda::es2panda::varbinder {
class LoopScope;
}  // namespace panda::es2panda::varbinder

namespace panda::es2panda::ir {
class Expression;

class ForUpdateStatement : public LoopStatement {
public:
    explicit ForUpdateStatement(varbinder::LoopScope *scope, AstNode *init, Expression *test, Expression *update,
                                Statement *body)
        : LoopStatement(AstNodeType::FOR_UPDATE_STATEMENT, scope),
          init_(init),
          test_(test),
          update_(update),
          body_(body)
    {
    }

    AstNode *Init()
    {
        return init_;
    }

    const AstNode *Init() const
    {
        return init_;
    }

    Expression *Test()
    {
        return test_;
    }

    const Expression *Test() const
    {
        return test_;
    }

    Expression *Update()
    {
        return update_;
    }

    const Expression *Update() const
    {
        return update_;
    }

    Statement *Body()
    {
        return body_;
    }

    const Statement *Body() const
    {
        return body_;
    }

    void TransformChildren(const NodeTransformer &cb) override;
    void SetReturnType(checker::ETSChecker *checker, checker::Type *type) override
    {
        if (body_ != nullptr) {
            body_->SetReturnType(checker, type);
        }
    }

    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile([[maybe_unused]] compiler::PandaGen *pg) const override;
    void Compile([[maybe_unused]] compiler::ETSGen *etsg) const override;
    checker::Type *Check([[maybe_unused]] checker::TSChecker *checker) override;
    checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) override;

private:
    AstNode *init_;
    Expression *test_;
    Expression *update_;
    Statement *body_;
};
}  // namespace panda::es2panda::ir

#endif
