/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "etsBigIntType.h"

namespace ark::es2panda::checker {
bool ETSBigIntType::IsAssignableTo(Type const *target) const noexcept
{
    if (!target->IsETSBigIntType()) {
        return false;
    }

    if (target->IsConstantType()) {
        return value_ == target->AsETSBigIntType()->GetValue();
    }

    return true;
}

void ETSBigIntType::Identical(TypeRelation *relation, Type *other)
{
    if (!other->IsETSBigIntType()) {
        relation->Result(false);
    } else if (!other->IsConstantType()) {
        relation->Result(!IsConstantType());
    } else {
        relation->Result(IsConstantType() && value_ == other->AsETSBigIntType()->GetValue());
    }
}

void ETSBigIntType::AssignmentTarget(TypeRelation *relation, Type *source)
{
    relation->Result(source->IsETSBigIntType() && source->AsETSBigIntType()->IsAssignableTo(this));
}

void ETSBigIntType::IsSupertypeOf(TypeRelation *relation, Type *source)
{
    relation->Result(false);
    if (!IsConstantType()) {
        ETSObjectType::IsSupertypeOf(relation, source);
    }
}

Type *ETSBigIntType::Instantiate([[maybe_unused]] ArenaAllocator *allocator, [[maybe_unused]] TypeRelation *relation,
                                 [[maybe_unused]] GlobalTypesHolder *globalTypes)
{
    return this;
}
}  // namespace ark::es2panda::checker
