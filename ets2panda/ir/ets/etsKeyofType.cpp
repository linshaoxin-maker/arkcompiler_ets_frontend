/*
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

#include "etsKeyofType.h"

#include "checker/ETSchecker.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"
#include "utils/arena_containers.h"

namespace ark::es2panda::ir {
void ETSKeyofType::TransformChildren(const NodeTransformer &cb, std::string_view const transformationName)
{
    if (auto *transformedNode = cb(type_); type_ != transformedNode) {
        type_->SetTransformedNode(transformationName, transformedNode);
        type_ = static_cast<TypeNode *>(transformedNode);
    }
}

void ETSKeyofType::Iterate(const NodeTraverser &cb) const
{
    cb(type_);
}

void ETSKeyofType::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ETSKeyofType"}});
}

void ETSKeyofType::Dump(ir::SrcDumper *dumper) const
{
    dumper->Add("ETSKeyofType");
}

void ETSKeyofType::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *ETSKeyofType::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *ETSKeyofType::Check(checker::ETSChecker *checker)
{
    return GetType(checker);
}

checker::Type *ETSKeyofType::GetType(checker::ETSChecker *checker)
{
    if (TsType() != nullptr) {
        return TsType();
    }

    auto typeReference = type_->GetType(checker);
    if (!typeReference->IsETSObjectType()) {
        checker->LogTypeError("The `keyof` keyword can only be used for class or interface type", Start());
        SetTsType(checker->GlobalTypeError());
        return checker->GlobalTypeError();
    }

    SetTsType(checker->CreateUnionFromKeyofType(typeReference));
    return TsType();
}

ETSKeyofType *ETSKeyofType::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    TypeNode *type = type_->Clone(allocator, nullptr);
    ETSKeyofType *clone = allocator->New<ir::ETSKeyofType>(type);
    if (parent != nullptr) {
        clone->SetParent(parent);
    }
    clone->type_->SetParent(clone);
    return clone;
}
}  // namespace ark::es2panda::ir