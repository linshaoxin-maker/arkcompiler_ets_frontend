/**
 * Copyright (c) 2023 Shenzhen Kaihong Digital Industry Development Co., Ltd.
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

#ifndef ES2PANDA_PARSER_INCLUDE_AST_STATIC_BLOCK_H
#define ES2PANDA_PARSER_INCLUDE_AST_STATIC_BLOCK_H

#include <ir/statement.h>

namespace panda::es2panda::compiler {
class PandaGen;
}  // namespace panda::es2panda::compiler

namespace panda::es2panda::checker {
class Checker;
class Type;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::ir {

class Expression;

class ClassStaticBlock : public Statement {
public:
    explicit ClassStaticBlock(FunctionExpression *bodyFunction,
                              ModifierFlags modifiers, ArenaVector<Decorator *> &&decorators)
        : Statement(AstNodeType::CLASS_STATIC_BLOCK),
          bodyFunction_(bodyFunction),
          modifiers_(modifiers),
          decorators_(std::move(decorators))
    {
    }

    ModifierFlags Modifiers() const
    {
        return modifiers_;
    }

    const FunctionExpression *BodyFunction() const
    {
        return bodyFunction_;
    }

    bool IsStatic() const
    {
        return (modifiers_ & ModifierFlags::STATIC) != 0;
    }

    bool IsOptional() const
    {
        return (modifiers_ & ModifierFlags::OPTIONAL) != 0;
    }

    const ArenaVector<Decorator *> &Decorators() const
    {
        return decorators_;
    }

    const ScriptFunction *Function() const;

    ScriptFunction *Function();

    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile([[maybe_unused]] compiler::PandaGen *pg) const override;
    checker::Type *Check([[maybe_unused]] checker::Checker *checker) const override;
    void UpdateSelf(const NodeUpdater &cb, [[maybe_unused]] binder::Binder *binder) override;

private:
    FunctionExpression *bodyFunction_;
    ModifierFlags modifiers_;
    ArenaVector<Decorator *> decorators_;
};

}  // namespace panda::es2panda::ir

#endif
