/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "chainExpression.h"
#include <compiler/core/pandagen.h>

#include <ir/astDump.h>
#include <ir/expressions/memberExpression.h>
#include <ir/expressions/callExpression.h>

namespace panda::es2panda::ir {

void ChainExpression::Iterate(const NodeTraverser &cb) const
{
    cb(expression_);
}

void ChainExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ChainExpression"}, {"expression", expression_}});
}

void ChainExpression::Compile([[maybe_unused]] compiler::PandaGen *pg) const
{
    const MemberExpression *memberExpr = nullptr;
    if (this->GetExpression()->IsMemberExpression()) {
        memberExpr = this->GetExpression()->AsMemberExpression();
    } else {
        auto callExpr = this->GetExpression()->AsCallExpression();
        memberExpr = callExpr->Callee()->AsMemberExpression();
    }
    compiler::Label *resultLabel = nullptr;
    CompileLogical(pg, memberExpr, resultLabel);
    this->GetExpression()->Compile(pg);
    pg->SetLabel(this, resultLabel);
}

void ChainExpression::CompileLogical(compiler::PandaGen *pg, const MemberExpression *memberExpr,
                                     compiler::Label *&resultLabel) const
{
    compiler::RegScope rs(pg);
    compiler::VReg falseReg = pg->AllocReg();
    compiler::VReg trueReg = pg->AllocReg();
    compiler::VReg nullReg = pg->AllocReg();
    compiler::VReg undefinedReg = pg->AllocReg();
    compiler::VReg objReg = pg->AllocReg();

    pg->LoadConst(this, compiler::Constant::JS_FALSE);
    pg->StoreAccumulator(this, falseReg);
    pg->LoadConst(this, compiler::Constant::JS_TRUE);
    pg->StoreAccumulator(this, trueReg);
    pg->LoadConst(this, compiler::Constant::JS_NULL);
    pg->StoreAccumulator(this, nullReg);
    pg->LoadConst(this, compiler::Constant::JS_UNDEFINED);
    pg->StoreAccumulator(this, undefinedReg);

    CheckMemberExpressionObjIfNull(pg, memberExpr, std::vector<compiler::VReg>{objReg, trueReg, falseReg, nullReg});
    auto *isNullRes = pg->AllocLabel();
    pg->BranchIfNotFalse(this, isNullRes);

    CheckMemberExpressionObjIfUndefined(pg, memberExpr,
                                        std::vector<compiler::VReg>{objReg, trueReg, falseReg, undefinedReg});
    auto *isUndefinedRes = pg->AllocLabel();
    pg->BranchIfNotTrue(this, isUndefinedRes);

    pg->SetLabel(this, isNullRes);
    pg->LoadAccumulatorInt(this, 0);
    pg->LoadAccumulator(this, undefinedReg);
    resultLabel = pg->AllocLabel();
    pg->Branch(this, resultLabel);
    pg->SetLabel(this, isUndefinedRes);
}

void ChainExpression::CheckMemberExpressionObjIfNull(compiler::PandaGen *pg, const MemberExpression *memberExpr,
                                                     std::vector<compiler::VReg> regs) const
{
    auto *notNullLabel = pg->AllocLabel();
    auto *isNullLabel = pg->AllocLabel();

    memberExpr->CompileObject(pg, regs[0]);
    pg->LoadAccumulator(this, regs[3]);
    pg->Condition(this, lexer::TokenType::PUNCTUATOR_STRICT_EQUAL, regs[0], notNullLabel);
    pg->LoadAccumulator(this, regs[1]);
    pg->Branch(this, isNullLabel);
    pg->SetLabel(this, notNullLabel);
    pg->LoadAccumulator(this, regs[2]);
    pg->SetLabel(this, isNullLabel);
}

void ChainExpression::CheckMemberExpressionObjIfUndefined(compiler::PandaGen *pg, const MemberExpression *memberExpr,
                                                          std::vector<compiler::VReg> regs) const
{
    auto *notUndefinedLabel = pg->AllocLabel();
    auto *isUndefinedLabel = pg->AllocLabel();

    memberExpr->CompileObject(pg, regs[0]);
    pg->LoadAccumulatorInt(this, 0);
    pg->LoadAccumulator(this, regs[3]);
    pg->Condition(this, lexer::TokenType::PUNCTUATOR_STRICT_EQUAL, regs[0], notUndefinedLabel);
    pg->LoadAccumulator(this, regs[1]);
    pg->Branch(this, isUndefinedLabel);
    pg->SetLabel(this, notUndefinedLabel);
    pg->LoadAccumulator(this, regs[2]);
    pg->SetLabel(this, isUndefinedLabel);
}

checker::Type *ChainExpression::Check([[maybe_unused]] checker::Checker *checker) const
{
    return expression_->Check(checker);
}

}  // namespace panda::es2panda::ir
