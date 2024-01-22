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

#include "etsStringType.h"

#include "varbinder/ETSBinder.h"

namespace ark::es2panda::checker {
void ETSStringType::Identical(TypeRelation *relation, Type *other)
{
    bool bothConstants = IsConstantType() && other->IsConstantType() && other->IsETSStringType();
    bool bothNonConstants = !IsConstantType() && !other->IsConstantType();
    if ((bothConstants && value_ == other->AsETSStringType()->GetValue()) ||
        (bothNonConstants && other->IsETSStringType())) {
        relation->Result(true);
    }
}

bool ETSStringType::AssignmentSource([[maybe_unused]] TypeRelation *relation, [[maybe_unused]] Type *target)
{
    bool bothConstants = IsConstantType() && target->IsConstantType();
    relation->Result(target->IsETSStringType() &&
                     ((bothConstants && value_ == target->AsETSStringType()->GetValue()) || !bothConstants));
    return relation->IsTrue();
}

void ETSStringType::AssignmentTarget([[maybe_unused]] TypeRelation *relation, [[maybe_unused]] Type *source)
{
    bool bothConstants = IsConstantType() && source->IsConstantType();
    if (source->IsETSStringType() &&
        ((bothConstants && value_ == source->AsETSStringType()->GetValue()) || !bothConstants)) {
        relation->Result(true);
    }
}

Type *ETSStringType::Instantiate([[maybe_unused]] ArenaAllocator *allocator, [[maybe_unused]] TypeRelation *relation,
                                 [[maybe_unused]] GlobalTypesHolder *globalTypes)
{
    return this;
}

void ETSStringType::IsSupertypeOf(TypeRelation *relation, Type *source)
{
    if (IsConstantType()) {
        relation->Result(false);
        return;
    }
    ETSObjectType::IsSupertypeOf(relation, source);
}

}  // namespace ark::es2panda::checker
