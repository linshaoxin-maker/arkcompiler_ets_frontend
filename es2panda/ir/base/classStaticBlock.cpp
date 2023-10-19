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

#include "classStaticBlock.h"

#include <ir/astDump.h>
#include <ir/base/decorator.h>
#include <ir/base/scriptFunction.h>
#include <ir/expression.h>
#include <ir/expressions/functionExpression.h>

#include <utility>

namespace panda::es2panda::ir {

const ScriptFunction *ClassStaticBlock::Function() const
{
    return bodyFunction_->Function();
}

ScriptFunction *ClassStaticBlock::Function()
{
    return bodyFunction_->Function();
}

void ClassStaticBlock::Iterate(const NodeTraverser &cb) const
{
    cb(bodyFunction_);

    for (auto *it : decorators_) {
        cb(it);
    }
}

void ClassStaticBlock::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ClassStaticBlock"},
                 {"accessibility", AstDumper::Optional(AstDumper::ModifierToString(modifiers_))},
                 {"static", true},
                 {"bodyFunction", bodyFunction_},
                 {"decorators", decorators_}});
}

void ClassStaticBlock::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *ClassStaticBlock::Check([[maybe_unused]] checker::Checker *checker) const
{
    return nullptr;
}

void ClassStaticBlock::UpdateSelf(const NodeUpdater &cb, [[maybe_unused]] binder::Binder *binder)
{
    bodyFunction_ = std::get<ir::AstNode *>(cb(bodyFunction_))->AsFunctionExpression();

    for (auto iter = decorators_.begin(); iter != decorators_.end(); iter++) {
        *iter = std::get<ir::AstNode *>(cb(*iter))->AsDecorator();
    }
}

}  // namespace panda::es2panda::ir

