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
void ETSBigIntType::Identical(TypeRelation *relation, Type *other)
{
    bool bothConstants = IsConstantType() && other->IsConstantType() && other->IsETSBigIntType();
    bool bothNonConstants = !IsConstantType() && !other->IsConstantType();
    if ((bothConstants && value_ == other->AsETSBigIntType()->GetValue()) ||
        (bothNonConstants && other->IsETSBigIntType())) {
        relation->Result(true);
    }
}

void ETSBigIntType::AssignmentTarget([[maybe_unused]] TypeRelation *relation, [[maybe_unused]] Type *source)
{
    if (source->IsETSBigIntType()) {
        Identical(relation, source);
        relation->Result(relation->IsTrue() || !IsConstantType());
        return;
    }

    relation->Result(false);
}

void ETSBigIntType::IsSupertypeOf(TypeRelation *relation, Type *source)
{
    if (IsConstantType()) {
        relation->Result(false);
        return;
    }
    ETSObjectType::IsSupertypeOf(relation, source);
}

Type *ETSBigIntType::Instantiate([[maybe_unused]] ArenaAllocator *allocator, [[maybe_unused]] TypeRelation *relation,
                                 [[maybe_unused]] GlobalTypesHolder *globalTypes)
{
    return this;
}
}  // namespace ark::es2panda::checker
