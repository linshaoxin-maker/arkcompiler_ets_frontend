/*
 * Copyright (c) 2021 - 2024 Huawei Device Co., Ltd.
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

#include "checker/ETSchecker.h"
#include "varbinder/ETSBinder.h"

namespace ark::es2panda::checker {
bool AreStringTypesAssignable(Type *source, Type *target)
{
    if (target->IsConstantType()) {
        return source->IsConstantType() &&
               source->AsETSStringType()->GetValue() == target->AsETSStringType()->GetValue();
    }
    return true;
}

void ETSStringType::Identical(TypeRelation *relation, Type *other)
{
    if (!other->IsETSStringType()) {
        relation->Result(false);
    } else if (!other->IsConstantType()) {
        relation->Result(!IsConstantType());
    } else {
        relation->Result(IsConstantType() && value_ == other->AsETSStringType()->GetValue());
    }
}

bool ETSStringType::AssignmentSource(TypeRelation *relation, Type *target)
{
    auto node = relation->GetNode();

    if ((relation->InAssignmentContext() || relation->ApplyStringToChar()) && IsConvertibleTo(target)) {
        node->AddBoxingUnboxingFlags(ir::BoxingUnboxingFlags::UNBOX_TO_CHAR);
        return relation->Result(true);
    }

    if (target->IsETSUnionType()) {
        for (auto *const constituentType : target->AsETSUnionType()->ConstituentTypes()) {
            if (AssignmentSource(relation, constituentType)) {
                return relation->Result(true);
            }
        }
        return relation->Result(false);
    }

    return relation->Result(target->IsETSStringType() && AreStringTypesAssignable(this, target));
}

void ETSStringType::AssignmentTarget(TypeRelation *relation, Type *source)
{
    relation->Result(source->IsETSStringType() && AreStringTypesAssignable(source, this));
}

Type *ETSStringType::Instantiate([[maybe_unused]] ArenaAllocator *allocator, [[maybe_unused]] TypeRelation *relation,
                                 [[maybe_unused]] GlobalTypesHolder *globalTypes)
{
    return this;
}

void ETSStringType::IsSupertypeOf(TypeRelation *relation, Type *source)
{
    if (IsConstantType()) {
        Identical(relation, source);
        return;
    }
    auto *const checker = relation->GetChecker()->AsETSChecker();
    relation->IsSupertypeOf(checker->GlobalBuiltinETSStringType(), source);
}

void ETSStringType::IsSubtypeOf(TypeRelation *relation, Type *source)
{
    if (source->IsConstantType()) {
        Identical(relation, source);
        return;
    }
    auto *const checker = relation->GetChecker()->AsETSChecker();
    relation->IsSupertypeOf(source, checker->GlobalBuiltinETSStringType());
}

bool ETSStringType::IsConvertibleTo(Type const *to) const
{
    const bool targetIsChar =
        to->IsCharType() ||
        // ETSObjectType is used in arrow function expression call.
        (to->IsETSObjectType() && to->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::BUILTIN_CHAR));
    if (targetIsChar && IsConstantType() && value_.IsConvertibleToChar()) {
        return !to->IsConstantType() || to->AsCharType()->GetValue() == value_.Utf8()[0];
    }

    return false;
}

}  // namespace ark::es2panda::checker
