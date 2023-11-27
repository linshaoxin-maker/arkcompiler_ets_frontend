/*
 * Copyright (c) 2021 - 2023 Huawei Device Co., Ltd.
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

#include "etsNewArrayInstanceExpression.h"

#include "ir/astDump.h"
#include "ir/typeNode.h"
#include "compiler/core/ETSGen.h"
#include "checker/ETSchecker.h"
#include "ir/ets/etsTypeReference.h"
#include "ir/ets/etsTypeReferencePart.h"
#include "ir/expressions/identifier.h"

namespace panda::es2panda::ir {
void ETSNewArrayInstanceExpression::TransformChildren(const NodeTransformer &cb)
{
    type_reference_ = static_cast<TypeNode *>(cb(type_reference_));
    dimension_ = cb(dimension_)->AsExpression();
}

void ETSNewArrayInstanceExpression::Iterate(const NodeTraverser &cb) const
{
    cb(type_reference_);
    cb(dimension_);
}

void ETSNewArrayInstanceExpression::Dump(ir::AstDumper *dumper) const
{
    dumper->Add(
        {{"type", "ETSNewArrayInstanceExpression"}, {"typeReference", type_reference_}, {"dimension", dimension_}});
}

void ETSNewArrayInstanceExpression::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}
void ETSNewArrayInstanceExpression::Compile([[maybe_unused]] compiler::ETSGen *etsg) const
{
    compiler::RegScope rs(etsg);
    compiler::TargetTypeContext ttctx(etsg, etsg->Checker()->GlobalIntType());

    dimension_->Compile(etsg);

    compiler::VReg arr = etsg->AllocReg();
    compiler::VReg dim = etsg->AllocReg();
    etsg->ApplyConversionAndStoreAccumulator(this, dim, dimension_->TsType());
    etsg->NewArray(this, arr, dim, TsType());
     
    std::uint32_t elem_num;
    if(dimension_->TsType()->IsIntType())
    {
        elem_num = dimension_->TsType()->AsIntType()->GetValue();
    }else{
        elem_num = 0;
    }
    const auto index_reg = etsg->AllocReg();
    for(std::uint32_t i = 0;i < elem_num;i++)
    {
        etsg->LoadAccumulatorInt(this, i);
        etsg->StoreAccumulator(this, index_reg);
        const compiler::TargetTypeContext ttctx2(etsg,type_reference_->TsType());
        if(signature_ != nullptr)
        {
            etsg->InitObject(this,signature_,arguments_);
            etsg->StoreArrayElement(this,arr,index_reg,type_reference_->TsType());
        }
    }
    etsg->SetVRegType(arr, TsType());
    etsg->LoadAccumulator(this, arr);
}

checker::Type *ETSNewArrayInstanceExpression::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *ETSNewArrayInstanceExpression::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    auto *element_type = type_reference_->GetType(checker);
    checker->ValidateArrayIndex(dimension_);
    if(element_type->IsETSObjectType()){
        auto signatures = element_type->AsETSObjectType()->ConstructSignatures();
        if(signatures.size() != 0)
        {
            signature_ = checker->ResolveConstructExpressionParameterless(element_type->AsETSObjectType(),arguments_,Start());
        }
    }
    SetTsType(checker->CreateETSArrayType(element_type));
    checker->CreateBuiltinArraySignature(TsType()->AsETSArrayType(), 1);
    return TsType();
}

// NOLINTNEXTLINE(google-default-arguments)
ETSNewArrayInstanceExpression *ETSNewArrayInstanceExpression::Clone(ArenaAllocator *const allocator,
                                                                    AstNode *const parent)
{
    auto *const type_ref = type_reference_ != nullptr ? type_reference_->Clone(allocator) : nullptr;
    auto *const dimension = dimension_ != nullptr ? dimension_->Clone(allocator)->AsExpression() : nullptr;

    if (auto *const clone = allocator->New<ETSNewArrayInstanceExpression>(allocator,type_ref, dimension); clone != nullptr) {
        if (type_ref != nullptr) {
            type_ref->SetParent(clone);
        }
        if (dimension != nullptr) {
            dimension->SetParent(clone);
        }
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
