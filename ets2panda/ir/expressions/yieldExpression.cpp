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

#include "yieldExpression.h"

#include "compiler/core/pandagen.h"
#include "compiler/function/generatorFunctionBuilder.h"
#include "checker/TSchecker.h"
#include "ir/astDump.h"

namespace panda::es2panda::ir {
void YieldExpression::TransformChildren(const NodeTransformer &cb)
{
    if (argument_ != nullptr) {
        argument_ = cb(argument_)->AsExpression();
    }
}

void YieldExpression::Iterate(const NodeTraverser &cb) const
{
    if (argument_ != nullptr) {
        cb(argument_);
    }
}

void YieldExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "YieldExpression"}, {"delegate", delegate_}, {"argument", AstDumper::Nullable(argument_)}});
}

void YieldExpression::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    compiler::RegScope rs(pg);

    if (argument_ != nullptr) {
        argument_->Compile(pg);
    } else {
        pg->LoadConst(this, compiler::Constant::JS_UNDEFINED);
    }

    if (delegate_) {
        ASSERT(argument_);
        pg->FuncBuilder()->YieldStar(this);
    } else {
        pg->FuncBuilder()->Yield(this);
    }
}

checker::Type *YieldExpression::Check([[maybe_unused]] checker::TSChecker *checker)
{
    // TODO(aszilagyi)
    return checker->GlobalAnyType();
}

checker::Type *YieldExpression::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}
}  // namespace panda::es2panda::ir
