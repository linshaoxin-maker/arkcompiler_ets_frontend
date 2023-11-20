/**
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

#include "tsIndexSignature.h"

#include "ir/astDump.h"
#include "ir/expressions/identifier.h"

#include "checker/TSchecker.h"

namespace panda::es2panda::ir {
TSIndexSignature::TSIndexSignatureKind TSIndexSignature::Kind() const noexcept
{
    return param_->AsIdentifier()->TypeAnnotation()->IsTSNumberKeyword() ? TSIndexSignatureKind::NUMBER
                                                                         : TSIndexSignatureKind::STRING;
}

void TSIndexSignature::TransformChildren(const NodeTransformer &cb)
{
    param_ = cb(param_)->AsExpression();
    type_annotation_ = static_cast<TypeNode *>(cb(type_annotation_));
}

void TSIndexSignature::Iterate(const NodeTraverser &cb) const
{
    cb(param_);
    cb(type_annotation_);
}

void TSIndexSignature::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "TSIndexSignature"},
                 {"parameters", param_},
                 {"typeAnnotation", type_annotation_},
                 {"readonly", readonly_}});
}

void TSIndexSignature::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *TSIndexSignature::Check([[maybe_unused]] checker::TSChecker *checker)
{
    if (TsType() != nullptr) {
        return TsType();
    }

    const util::StringView &param_name = param_->AsIdentifier()->Name();
    type_annotation_->Check(checker);
    checker::Type *index_type = type_annotation_->GetType(checker);
    checker::IndexInfo *info =
        checker->Allocator()->New<checker::IndexInfo>(index_type, param_name, readonly_, this->Start());
    checker::ObjectDescriptor *desc = checker->Allocator()->New<checker::ObjectDescriptor>(checker->Allocator());
    checker::ObjectType *placeholder = checker->Allocator()->New<checker::ObjectLiteralType>(desc);

    if (Kind() == ir::TSIndexSignature::TSIndexSignatureKind::NUMBER) {
        placeholder->Desc()->number_index_info = info;
    } else {
        placeholder->Desc()->string_index_info = info;
    }

    SetTsType(placeholder);
    return placeholder;
}

checker::Type *TSIndexSignature::Check([[maybe_unused]] checker::ETSChecker *checker)
{
    return nullptr;
}

// NOLINTNEXTLINE(google-default-arguments)
TSIndexSignature *TSIndexSignature::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    auto *const param = param_ != nullptr ? param_->Clone(allocator)->AsExpression() : nullptr;
    auto *const type_annotation = type_annotation_->Clone(allocator);

    if (auto *const clone = allocator->New<TSIndexSignature>(param, type_annotation, readonly_); clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }
        if (param != nullptr) {
            param->SetParent(clone);
        }
        type_annotation->SetParent(clone);
        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace panda::es2panda::ir
