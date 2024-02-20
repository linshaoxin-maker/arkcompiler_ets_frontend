/**
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

#include "ETSchecker.h"

namespace ark::es2panda::checker {

CheckerContext::CheckerContext(Checker *checker, CheckerStatus newStatus, ETSObjectType *containingClass,
                               Signature *containingSignature)
    : parent_(checker),
      status_(newStatus),
      capturedVars_(parent_->Allocator()->Adapter()),
      smartCasts_(parent_->Allocator()->Adapter()),
      containingClass_(containingClass),
      containingSignature_(containingSignature)
{
}

SmartCastArray CheckerContext::CloneSmartCasts() const noexcept
{
    SmartCastArray smartCasts {};
    if (!smartCasts_.empty()) {
        smartCasts.reserve(smartCasts_.size());

        for (auto const [variable, type] : smartCasts_) {
            smartCasts.emplace_back(variable, type);
        }
    }
    return smartCasts;
}

void CheckerContext::RestoreSmartCasts(SmartCastArray const &prevSmartCasts) noexcept
{
    smartCasts_.clear();
    if (!prevSmartCasts.empty()) {
        for (auto [variable, type] : prevSmartCasts) {
            smartCasts_.emplace(variable, type);
        }
    }
}

void CheckerContext::CombineSmartCasts(SmartCastArray &prevSmartCasts) noexcept
{
    auto *const checker = parent_->AsETSChecker();

    auto const combineTypes = [checker](checker::Type *typeOne, checker::Type *typeTwo) -> checker::Type * {
        ASSERT(typeOne != nullptr && typeTwo != nullptr);
        if (checker->Relation()->IsIdenticalTo(typeOne, typeTwo)) {
            // no type change is required
            return nullptr;
        }

        return checker->CreateETSUnionType({typeOne, typeTwo});
    };

    auto previousCast = prevSmartCasts.cbegin();
    while (previousCast != prevSmartCasts.cend()) {
        auto const currentCast = smartCasts_.find(previousCast->first);
        if (currentCast == smartCasts_.end()) {
            // Remove smart cast that doesn't present in the current set.
            previousCast = prevSmartCasts.erase(previousCast);
            continue;
        }

        // Smart type was modified
        if (auto *const smart_type = combineTypes(previousCast->second, currentCast->second); smart_type != nullptr) {
            // Remove it or set to new value
            if (checker->Relation()->IsIdenticalTo(currentCast->first->TsType(), smart_type)) {
                smartCasts_.erase(currentCast);
                previousCast = prevSmartCasts.erase(previousCast);
                continue;
            } else {
                currentCast->second = smart_type;
            }
        }
        ++previousCast;
    }

    // Remove smart casts that don't present in the initial set.
    if (!smartCasts_.empty()) {
        auto it = smartCasts_.cbegin();
        while (it != smartCasts_.cend()) {
            if (std::find_if(prevSmartCasts.begin(), prevSmartCasts.end(), [&it](auto const &item) -> bool {
                    return item.first == it->first;
                }) == prevSmartCasts.end()) {
                it = smartCasts_.erase(it);
            } else {
                ++it;
            }
        }
    }
}
}  // namespace ark::es2panda::checker
