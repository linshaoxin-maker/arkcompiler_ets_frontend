/*
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

#include <numeric>
#include "etsUnionType.h"
#include "checker/ets/boxingConverter.h"
#include "checker/ets/conversion.h"
#include "checker/types/globalTypesHolder.h"
#include "checker/ETSchecker.h"

namespace ark::es2panda::checker {
void ETSUnionType::ToString(std::stringstream &ss, bool precise) const
{
    for (auto it = constituentTypes_.begin(); it != constituentTypes_.end(); it++) {
        (*it)->ToString(ss, precise);
        if (std::next(it) != constituentTypes_.end()) {
            ss << "|";
        }
    }
}

void ETSUnionType::ToAssemblerType(std::stringstream &ss) const
{
    assemblerLub_->ToAssemblerTypeWithRank(ss);
}

void ETSUnionType::ToDebugInfoType(std::stringstream &ss) const
{
    assemblerLub_->ToDebugInfoType(ss);
}

ETSUnionType::ETSUnionType(ETSChecker *checker, ArenaVector<Type *> &&constituentTypes)
    : Type(TypeFlag::ETS_UNION), constituentTypes_(std::move(constituentTypes))
{
    ASSERT(constituentTypes_.size() > 1);
    assemblerLub_ = ComputeAssemblerLUB(checker, this);
}

bool ETSUnionType::EachTypeRelatedToSomeType(TypeRelation *relation, ETSUnionType *source, ETSUnionType *target)
{
    return std::all_of(source->constituentTypes_.begin(), source->constituentTypes_.end(),
                       [relation, target](auto *s) { return TypeRelatedToSomeType(relation, s, target); });
}

bool ETSUnionType::TypeRelatedToSomeType(TypeRelation *relation, Type *source, ETSUnionType *target)
{
    return std::any_of(target->constituentTypes_.begin(), target->constituentTypes_.end(),
                       [relation, source](auto *t) { return relation->IsIdenticalTo(source, t); });
}

static auto constexpr ETS_NORMALIZABLE_NUMERIC = TypeFlag(TypeFlag::ETS_NUMERIC & ~TypeFlag::CHAR);

static Type *LargestNumeric(Type *t1, Type *t2)
{
    static_assert(TypeFlag::DOUBLE > TypeFlag::FLOAT);
    static_assert(TypeFlag::FLOAT > TypeFlag::LONG);
    static_assert(TypeFlag::LONG > TypeFlag::INT);
    static_assert(TypeFlag::INT > TypeFlag::SHORT);
    static_assert(TypeFlag::SHORT > TypeFlag::BYTE);

    auto v1 = t1->TypeFlags() & ETS_NORMALIZABLE_NUMERIC;
    auto v2 = t2->TypeFlags() & ETS_NORMALIZABLE_NUMERIC;
    ASSERT(helpers::math::IsPowerOfTwo(v1));
    ASSERT(helpers::math::IsPowerOfTwo(v2));
    return v1 > v2 ? t1 : t2;
}

static Type *LargestPrimitive(Type *t1, Type *t2)
{
    if (t1->IsCharType()) {
        return t2->IsByteType() || t2->IsETSBooleanType() ? t1 : t2;
    }
    if (t2->IsCharType()) {
        return t1->IsByteType() || t1->IsETSBooleanType() ? t2 : t1;
    }
    return t1->IsETSBooleanType() ? t2 : t2->IsETSBooleanType() ? t1 : LargestNumeric(t1, t2);
}

// This function computes effective runtime representation of union type
Type *ETSUnionType::ComputeAssemblerLUB(ETSChecker *checker, ETSUnionType *un)
{
    auto *const apparent = checker->GetApparentType(un);
    if (!apparent->IsETSUnionType()) {
        return apparent;
    }
    if (apparent != un) {
        return apparent->AsETSUnionType()->assemblerLub_;
    }
    un = apparent->AsETSUnionType();

    Type *lub = nullptr;
    for (auto *t : un->ConstituentTypes()) {
        if (t->IsETSNullType()) {
            continue;
        }
        if (t->IsETSUndefinedType()) {
            return checker->GetGlobalTypesHolder()->GlobalETSObjectType();
        }
        if (lub == nullptr) {
            lub = t;
            continue;
        }
        if (t->IsETSObjectType() && lub->IsETSObjectType()) {
            lub = checker->GetClosestCommonAncestor(lub->AsETSObjectType(), t->AsETSObjectType());
        } else if (t->IsETSArrayType() && lub->IsETSArrayType()) {
            // NOTE: can compute "common(lub, t)[]"
            return checker->GetGlobalTypesHolder()->GlobalETSObjectType();
        } else if (t->IsETSPrimitiveType()) {
            if (lub->IsETSObjectType()) {
                lub = checker->GetClosestCommonAncestor(lub->AsETSObjectType(),
                                                        BoxingConverter::ETSTypeFromSource(checker, t));
            } else if (lub->HasTypeFlag(TypeFlag::ETS_PRIMITIVE)) {
                lub = LargestPrimitive(t, lub);
            } else {
                // NOTE: if lub is array
                return checker->GetGlobalTypesHolder()->GlobalETSObjectType();
            }
        } else {
            return checker->GetGlobalTypesHolder()->GlobalETSObjectType();
        }
    }
    return checker->GetNonConstantTypeFromPrimitiveType(lub);
}

void ETSUnionType::Identical(TypeRelation *relation, Type *other)
{
    if (other->IsETSUnionType()) {
        if (EachTypeRelatedToSomeType(relation, this, other->AsETSUnionType()) &&
            EachTypeRelatedToSomeType(relation, other->AsETSUnionType(), this)) {
            relation->Result(true);
            return;
        }
    }

    relation->Result(false);
}

template <typename RelFN>
void ETSUnionType::RelationSource(TypeRelation *relation, Type *target, RelFN const &relFn)
{
    auto *const checker = relation->GetChecker()->AsETSChecker();
    if (IsNumericUnion()) {
        if (relation->IsAssignableTo(assemblerLub_, target)) {
            //        if (assemblerLub_ == target) {
            relation->Result(true);
            return;
        }
        //        if (relation->IsAssignableTo(assemblerLub_, checker->MaybePrimitiveBuiltinType(target))) {
        //            relation->GetNode()->SetBoxingUnboxingFlags(checker->GetBoxingFlag(target));
        //        }
        //        return;
    }

    auto *const refTarget = checker->MaybePromotedBuiltinType(target);
    if (target != refTarget && !relation->ApplyUnboxing()) {
        relation->Result(false);
        return;
    }
    if (relation->IsSupertypeOf(refTarget, this)) {
        if (refTarget != target) {
            relation->GetNode()->SetBoxingUnboxingFlags(checker->GetUnboxingFlag(refTarget));
        }
        return;
    }
    if (target == refTarget && !target->IsETSFunctionType() && !target->IsETSEnumType()) {
        relation->Result(false);
        return;
    }
    if (!relation->OnlyCheckWidening()) {
        relation->SetFlags(relation->GetFlags() | TypeRelationFlag::ONLY_CHECK_WIDENING);
    }

    int related = 0;
    for (auto *ct : ConstituentTypes()) {
        if (!relFn(relation, checker->MaybePrimitiveBuiltinType(ct), target)) {
            continue;
        }
        if (ct->IsETSUnboxableObject() && !IsNumericUnion() && !target->IsETSFunctionType() &&
            !target->IsETSEnumType()) {
            relation->GetNode()->SetBoxingUnboxingFlags(
                checker->GetUnboxingFlag(checker->MaybePrimitiveBuiltinType(ct)));
        }
        if (related == 0 || !ct->IsConstantType()) {
            related++;
        }
    }
    relation->Result(related >= 1);
}

template <typename RelFN>
void ETSUnionType::RelationTarget(TypeRelation *relation, Type *source, RelFN const &relFn)
{
    auto *const checker = relation->GetChecker()->AsETSChecker();
    auto *const refSource = checker->MaybePromotedBuiltinType(source);

    if (source != refSource && !relation->ApplyBoxing()) {
        relation->Result(false);
        return;
    }
    if (relation->IsSupertypeOf(this, refSource)) {
        if (refSource != source) {
            relation->GetNode()->SetBoxingUnboxingFlags(checker->GetBoxingFlag(refSource));
        }
        return;
    }

    if (source == refSource && !source->IsETSFunctionType() && !source->IsETSEnumType() && !source->IsETSArrayType()) {
        relation->Result(false);
        return;
    }

    int related = 0;
    for (auto *ct : ConstituentTypes()) {  // NOTE(vpukhov): just test if union is supertype of any numeric
        if (!relFn(relation, checker->MaybePrimitiveBuiltinType(ct), source)) {
            continue;
        }
        if (!IsNumericUnion() && !source->IsETSFunctionType() && !source->IsETSEnumType() &&
            !source->IsETSArrayType() && !ct->IsETSStringType()) {
            relation->GetNode()->SetBoxingUnboxingFlags(checker->GetBoxingFlag(ct));
        }
        if (related == 0 || !ct->IsConstantType()) {
            related++;
        }
    }
    relation->Result(related >= 1);
}

bool ETSUnionType::AssignmentSource(TypeRelation *relation, Type *target)
{
    auto const relFn = []([[maybe_unused]] TypeRelation *rel, [[maybe_unused]] Type *ct, [[maybe_unused]] Type *tgt) {
        return false;
    };
    RelationSource(relation, target, relFn);
    return relation->IsTrue();
}

void ETSUnionType::AssignmentTarget(TypeRelation *relation, Type *source)
{
    auto const relFn = [](TypeRelation *rel, Type *ct, Type *src) { return rel->IsAssignableTo(src, ct); };
    RelationTarget(relation, source, relFn);
}

void ETSUnionType::Cast(TypeRelation *relation, Type *target)
{
    if (relation->InCastingContext() && target->IsETSReferenceType()) {
        relation->Result(true);  // NOTE(vpukhov): check if types intersect at least
        return;
    }
    if (IsNumericUnion()) {
        relation->IsCastableTo(assemblerLub_, target);
        return;
    }
    auto const relFn = [](TypeRelation *rel, Type *ct, Type *tgt) { return rel->IsCastableTo(ct, tgt); };
    RelationSource(relation, target, relFn);
}

void ETSUnionType::CastTarget(TypeRelation *relation, Type *source)
{
    if (relation->InCastingContext() && source->IsETSReferenceType()) {
        relation->Result(true);  // NOTE(vpukhov): check if types intersect at least
        return;
    }
    auto const relFn = [](TypeRelation *rel, Type *ct, Type *src) { return rel->IsCastableTo(src, ct); };
    RelationTarget(relation, source, relFn);
}

template <typename T>
static bool CompareValueToLimits(Type *constant)
{
    if (std::is_integral_v<T> && constant->HasTypeFlag(TypeFlag::ETS_FLOATING_POINT)) {
        return false;
    }
    T low = std::numeric_limits<T>::min();
    T high = std::numeric_limits<T>::max();
    T epsilon = std::numeric_limits<T>::epsilon();
    if (constant->IsByteType()) {
        auto val = constant->AsByteType()->GetValue();
        return (val >= low - epsilon) && (val <= high + epsilon);
    }
    if (constant->IsShortType()) {
        auto val = constant->AsShortType()->GetValue();
        return (val >= low - epsilon) && (val <= high + epsilon);
    }
    if (constant->IsCharType()) {
        auto val = constant->AsCharType()->GetValue();
        return (val >= low - epsilon) && (val <= high + epsilon);
    }
    if (constant->IsIntType()) {
        auto val = constant->AsIntType()->GetValue();
        return (val >= low - epsilon) && (val <= high + epsilon);
    }
    if (constant->IsLongType()) {
        auto val = constant->AsLongType()->GetValue();
        return (val >= low - epsilon) && (val <= high + epsilon);
    }
    if (constant->IsFloatType()) {
        auto val = constant->AsFloatType()->GetValue();
        return (val >= low - epsilon) && (val <= high + epsilon);
    }
    if (constant->IsDoubleType()) {
        auto val = constant->AsDoubleType()->GetValue();
        return (val >= low - epsilon) && (val <= high + epsilon);
    }
    UNREACHABLE();
}

static bool IsConstantFitsToType(Type *constant, Type *target)
{
    ASSERT(constant->IsETSPrimitiveType() && target->IsETSPrimitiveType());
    ASSERT(constant->IsConstantType());
    if (target->IsConstantType()) {
        // NOTE: Non-identical constants
        return false;
    }
    if (constant->IsETSBooleanType()) {
        return target->IsETSBooleanType();
    }
    switch (ETSChecker::ETSChecker::ETSType(target)) {
        case TypeFlag::BYTE: {
            return CompareValueToLimits<int8_t>(constant);
        }
        case TypeFlag::SHORT: {
            return CompareValueToLimits<int16_t>(constant);
        }
        case TypeFlag::CHAR: {
            return CompareValueToLimits<char16_t>(constant);
        }
        case TypeFlag::INT: {
            return CompareValueToLimits<int32_t>(constant);
        }
        case TypeFlag::LONG: {
            return CompareValueToLimits<int64_t>(constant);
        }
        case TypeFlag::FLOAT: {
            return CompareValueToLimits<float>(constant);
        }
        case TypeFlag::DOUBLE: {
            return CompareValueToLimits<double>(constant);
        }
        default: {
            return false;
        }
    }
}

static bool IsConstantAssignableTo(TypeRelation *relation, Type *constant, Type *target)
{
    ASSERT(constant->IsConstantType());
    auto checker = relation->GetChecker()->AsETSChecker();
    if (target == checker->GetGlobalTypesHolder()->GlobalETSObjectType()) {
        return true;
    }
    if (constant->IsETSStringType()) {
        return target->IsETSStringType() && (!target->IsConstantType() || relation->IsIdenticalTo(constant, target));
    }
    if (constant->IsETSBigIntType()) {
        return target->IsETSBigIntType() && (!target->IsConstantType() || relation->IsIdenticalTo(constant, target));
    }
    ASSERT(constant->IsETSPrimitiveType());
    if (!target->IsETSPrimitiveType()) {
        if (target->IsETSUnboxableObject()) {
            target = relation->GetChecker()->AsETSChecker()->MaybePrimitiveBuiltinType(target);
        } else {
            return false;
        }
    }
    return relation->IsIdenticalTo(constant, target) || IsConstantFitsToType(constant, target);
}

static std::optional<Type *> TryMergeTypes(TypeRelation *relation, Type *const t1, Type *const t2)
{
    if (relation->IsSupertypeOf(t1, t2) || (t2->IsConstantType() && IsConstantAssignableTo(relation, t2, t1))) {
        return t1;
    }
    if (relation->IsSupertypeOf(t2, t1) || (t1->IsConstantType() && IsConstantAssignableTo(relation, t1, t2))) {
        return t2;
    }
    return std::nullopt;
}

void ETSUnionType::LinearizeAndEraseIdentical(TypeRelation *relation, ArenaVector<Type *> &types)
{
    // Linearize
    size_t const initialSz = types.size();
    for (size_t i = 0; i < initialSz; ++i) {
        auto *const ct = types[i];
        if (ct->IsETSUnionType()) {
            auto const &otherTypes = ct->AsETSUnionType()->ConstituentTypes();
            types.insert(types.end(), otherTypes.begin(), otherTypes.end());
            types[i] = nullptr;
        } else if (ct->IsNeverType()) {
            types[i] = nullptr;
        }
    }
    size_t insPos = 0;
    for (size_t i = 0; i < types.size(); ++i) {
        auto *const ct = types[i];
        if (ct != nullptr) {
            types[insPos++] = ct;
        }
    }
    types.resize(insPos);

    if (!IsNumericUnion(types)) {
        PromoteTypes(relation, types);
    }
    // Reduce subtypes
    for (auto cmpIt = types.begin(); cmpIt != types.end(); ++cmpIt) {
        for (auto it = std::next(cmpIt); it != types.end();) {
            if (auto merged = TryMergeTypes(relation, *cmpIt, *it); merged) {
                if (merged == *cmpIt) {
                    it = types.erase(it);
                } else {
                    cmpIt = types.erase(cmpIt);
                    it = std::next(cmpIt);
                }
            } else {
                it++;
            }
        }
    }
}

void ETSUnionType::EliminateNever(TypeRelation *relation, ArenaVector<Type *> &types)
{
    auto checker = relation->GetChecker()->AsETSChecker();
    auto const isNever = [checker](Type *ct) {
        return ct == checker->GetGlobalTypesHolder()->GlobalBuiltinNeverType();
    };
    auto it = std::remove_if(types.begin(), types.end(), isNever);
    types.erase(it, types.end());
}

void ETSUnionType::NormalizeTypes(TypeRelation *relation, ArenaVector<Type *> &types)
{
    if (types.size() == 1) {
        return;
    }
    EliminateNever(relation, types);
    LinearizeAndEraseIdentical(relation, types);
    if (IsNumericUnion(types)) {
        auto const isNotNormalizableNumeric = [](Type *ct) {
            return !ct->HasTypeFlag(ETS_NORMALIZABLE_NUMERIC) || ct->IsConstantType();
        };
        auto bound = std::partition(types.begin(), types.end(), isNotNormalizableNumeric);
        if (bound != types.end()) {
            size_t newEnd = std::distance(types.begin(), bound);
            types[newEnd] = std::accumulate(std::next(bound), types.end(), *bound, LargestNumeric);
            types.resize(newEnd + 1);
        }
    }
}

void ETSUnionType::PromoteTypes(TypeRelation *relation, ArenaVector<Type *> &types)
{
    auto *const checker = relation->GetChecker()->AsETSChecker();
    for (auto &ct : types) {
        ct = !ct->IsConstantType() ? checker->MaybePromotedBuiltinType(ct) : ct;
    }
}

Type *ETSUnionType::Instantiate(ArenaAllocator *allocator, TypeRelation *relation, GlobalTypesHolder *globalTypes)
{
    auto *const checker = relation->GetChecker()->AsETSChecker();
    ArenaVector<Type *> copiedConstituents(allocator->Adapter());
    for (auto *it : constituentTypes_) {
        copiedConstituents.push_back(it->Instantiate(allocator, relation, globalTypes));
    }
    return checker->CreateETSUnionType(std::move(copiedConstituents));
}

Type *ETSUnionType::Substitute(TypeRelation *relation, const Substitution *substitution)
{
    auto *const checker = relation->GetChecker()->AsETSChecker();
    ArenaVector<Type *> substitutedConstituents(checker->Allocator()->Adapter());
    for (auto *ctype : constituentTypes_) {
        substitutedConstituents.push_back(ctype->Substitute(relation, substitution));
    }
    return checker->CreateETSUnionType(std::move(substitutedConstituents));
}

void ETSUnionType::IsSupertypeOf(TypeRelation *relation, Type *source)
{
    for (auto const &ctype : ConstituentTypes()) {
        if (relation->IsSupertypeOf(ctype, source)) {
            return;
        }
    }
}

void ETSUnionType::IsSubtypeOf(TypeRelation *relation, Type *target)
{
    for (auto const &ctype : ConstituentTypes()) {
        if (!relation->IsSupertypeOf(target, ctype)) {
            return;
        }
    }
}

static checker::Type *FindAssignableSmartNumericType(checker::ETSChecker *checker,
                                                     const std::vector<checker::Type *> &types,
                                                     checker::Type *sourceType, bool isRefUnion)
{
    auto *relation = checker->Relation();
    auto flags = relation->GetFlags();
    if (isRefUnion) {
        flags |= TypeRelationFlag::ASSIGNMENT_CONTEXT | TypeRelationFlag::ONLY_CHECK_BOXING_UNBOXING |
                 TypeRelationFlag::ONLY_CHECK_WIDENING;
    }
    checker::SavedTypeRelationFlagsContext savedTypeRelationFlagCtx(relation, flags);
    auto foundType = std::find_if(types.begin(), types.end(),
                                  [relation, sourceType](Type *t) { return relation->IsAssignableTo(sourceType, t); });
    auto *const assignableType =
        foundType != types.end() ? *foundType : checker->GetNonConstantTypeFromPrimitiveType(sourceType);
    ASSERT(assignableType != nullptr);
    return isRefUnion ? checker->MaybePromotedBuiltinType(assignableType) : assignableType;
}

//  NOTE! When calling this method we assume that 'AssignmentTarget(...)' check was passes successfully,
//  thus the required assignable type always exists.
checker::Type *ETSUnionType::GetAssignableType(checker::ETSChecker *checker, checker::Type *sourceType) const noexcept
{
    if (sourceType->IsETSTypeParameter() || sourceType->IsETSUnionType() || sourceType->IsETSArrayType() ||
        sourceType->IsETSFunctionType() || sourceType->IsETSEnumType()) {
        return sourceType;
    }

    auto *objectType = sourceType->IsETSObjectType() ? sourceType->AsETSObjectType() : nullptr;
    if (objectType != nullptr && (!objectType->HasObjectFlag(ETSObjectFlags::BUILTIN_TYPE) ||
                                  objectType->HasObjectFlag(ETSObjectFlags::BUILTIN_STRING))) {
        //  NOTE: here wo don't cast the actual type to possible base type using in the union, but use it as is!
        return sourceType;
    }

    std::map<std::uint32_t, std::vector<checker::Type *>> numericTypes {};
    bool const isBool = objectType != nullptr ? objectType->HasObjectFlag(ETSObjectFlags::BUILTIN_BOOLEAN)
                                              : sourceType->HasTypeFlag(TypeFlag::ETS_BOOLEAN);
    bool const isChar = objectType != nullptr ? objectType->HasObjectFlag(ETSObjectFlags::BUILTIN_CHAR)
                                              : sourceType->HasTypeFlag(TypeFlag::CHAR);
    if (checker::Type *assignableType =
            GetAssignableTypeAndCollectNumerics(checker, sourceType, isBool, isChar, numericTypes);
        assignableType != nullptr) {
        return assignableType;
    }

    if (auto const sourceId =
            objectType != nullptr ? ETSObjectType::GetPrecedence(objectType) : Type::GetPrecedence(sourceType);
        sourceId > 0U) {
        for (auto const &[id, types] : numericTypes) {
            if (id >= sourceId) {
                return FindAssignableSmartNumericType(checker, types, sourceType, IsReferenceUnion());
            }
        }
    }

    for (auto *constituentType : constituentTypes_) {
        if (constituentType->IsETSObjectType() && constituentType->AsETSObjectType()->IsGlobalETSObjectType()) {
            return constituentType;
        }
    }

    return nullptr;
}

checker::Type *ETSUnionType::GetAssignableTypeAndCollectNumerics(
    checker::ETSChecker *checker, checker::Type *sourceType, bool const isBool, bool const isChar,
    std::map<std::uint32_t, std::vector<checker::Type *>> &numericTypes) const noexcept
{
    checker::Type *assignableType = nullptr;
    auto *const objectType = sourceType->IsETSObjectType() ? sourceType->AsETSObjectType() : nullptr;

    for (auto *constituentType : constituentTypes_) {
        bool isTypeBool = false;
        bool isTypeChar = false;
        std::uint32_t id;
        if (constituentType->IsETSObjectType()) {
            auto *const type = constituentType->AsETSObjectType();
            isTypeBool = type->HasObjectFlag(ETSObjectFlags::BUILTIN_BOOLEAN);
            isTypeChar = type->HasObjectFlag(ETSObjectFlags::BUILTIN_CHAR);
            id = ETSObjectType::GetPrecedence(type);
        } else if (constituentType->IsETSPrimitiveType()) {
            isTypeBool = constituentType->IsETSBooleanType();
            isTypeChar = constituentType->IsCharType();
            id = Type::GetPrecedence(constituentType);
        } else {
            continue;
        }

        if (isTypeBool) {
            if (isBool) {
                assignableType = constituentType;
            }
        } else if (isTypeChar) {
            if (isChar) {
                assignableType = constituentType;
            }
        } else if (id > 0U) {
            numericTypes[id].push_back(constituentType);
        } else if (assignableType == nullptr && objectType != nullptr &&
                   checker->Relation()->IsSupertypeOf(constituentType, objectType)) {
            assignableType = constituentType;
        }

        if (assignableType != nullptr && sourceType != nullptr &&
            checker->Relation()->IsIdenticalTo(sourceType, assignableType)) {
            break;
        }
    }

    if (IsReferenceUnion() && assignableType != nullptr) {
        return checker->MaybePromotedBuiltinType(assignableType);
    }
    return assignableType;
}

bool ETSUnionType::ExtractType(checker::ETSChecker *checker, checker::ETSObjectType *sourceType) noexcept
{
    std::map<std::uint32_t, ArenaVector<checker::Type *>::const_iterator> numericTypes {};
    bool const isBool = sourceType->HasObjectFlag(ETSObjectFlags::BUILTIN_BOOLEAN);
    bool const isChar = sourceType->HasObjectFlag(ETSObjectFlags::BUILTIN_CHAR);

    auto it = constituentTypes_.cbegin();
    while (it != constituentTypes_.cend()) {
        auto *const constituentType = *it;

        if (checker->Relation()->IsIdenticalTo(constituentType, sourceType) ||
            //  NOTE: just a temporary solution because now Relation()->IsIdenticalTo(...) returns
            //  'false' for the types like 'ArrayLike<T>'
            constituentType->ToString() == static_cast<Type *>(sourceType)->ToString()) {
            constituentTypes_.erase(it);
            return true;
        }

        if (checker->Relation()->IsSupertypeOf(constituentType, sourceType)) {
            return true;
        }
        if (checker->Relation()->IsSupertypeOf(sourceType, constituentType)) {
            return true;
        }

        if (constituentType->IsETSObjectType()) {
            auto *const objectType = (*it)->AsETSObjectType();
            if (isBool && objectType->HasObjectFlag(ETSObjectFlags::BUILTIN_BOOLEAN)) {
                constituentTypes_.erase(it);
                return true;
            }
            if (isChar && objectType->HasObjectFlag(ETSObjectFlags::BUILTIN_CHAR)) {
                constituentTypes_.erase(it);
                return true;
            }
            if (auto const id = ETSObjectType::GetPrecedence(objectType); id > 0U) {
                numericTypes.emplace(id, it);
            }
        }

        ++it;
    }

    if (auto const sourceId = ETSObjectType::GetPrecedence(sourceType); sourceId > 0U) {
        for (auto const [id, it1] : numericTypes) {
            if (id >= sourceId) {
                constituentTypes_.erase(it1);
                return true;
            }
        }
    }

    return false;
}

bool ETSUnionType::ExtractType(checker::ETSChecker *checker, checker::ETSArrayType *sourceType) noexcept
{
    auto it = constituentTypes_.cbegin();
    while (it != constituentTypes_.cend()) {
        auto *const constituentType = *it;
        if (constituentType != nullptr && constituentType->IsETSArrayType()) {
            if (checker->Relation()->IsIdenticalTo(constituentType, sourceType) ||
                //  NOTE: just a temporary solution because now Relation()->IsIdenticalTo(...) returns
                //  'false' for the types like 'ArrayLike<T>'
                constituentType->ToString() == static_cast<Type *>(sourceType)->ToString()) {
                constituentTypes_.erase(it);
                return true;
            }

            if (checker->Relation()->IsSupertypeOf(constituentType, sourceType)) {
                return true;
            }
            if (checker->Relation()->IsSupertypeOf(sourceType, constituentType)) {
                return true;
            }
        }
        ++it;
    }

    for (auto const &constituentType : constituentTypes_) {
        if (constituentType != nullptr && constituentType->IsETSObjectType() &&
            constituentType->AsETSObjectType()->IsGlobalETSObjectType()) {
            return true;
        }
    }

    return false;
}

std::pair<checker::Type *, checker::Type *> ETSUnionType::GetComplimentaryType(ETSChecker *const checker,
                                                                               checker::Type *sourceType)
{
    checker::Type *clone = Clone(checker);
    bool ok = true;

    if (sourceType->IsETSUnionType()) {
        for (auto *const constituentType : sourceType->AsETSUnionType()->ConstituentTypes()) {
            if (ok = clone->AsETSUnionType()->ExtractType(checker, constituentType->AsETSObjectType()); !ok) {
                break;
            }
        }
    } else if (sourceType->IsETSArrayType()) {
        ok = clone->AsETSUnionType()->ExtractType(checker, sourceType->AsETSArrayType());
    } else {
        if (sourceType->HasTypeFlag(TypeFlag::ETS_PRIMITIVE) && !sourceType->IsETSVoidType()) {
            sourceType = checker->PrimitiveTypeAsETSBuiltinType(sourceType);
        }

        if (sourceType->IsETSObjectType()) {
            ok = clone->AsETSUnionType()->ExtractType(checker, sourceType->AsETSObjectType());
        }
    }

    if (!ok) {
        return std::make_pair(checker->GetGlobalTypesHolder()->GlobalNeverType(), this);
    }

    if (clone->AsETSUnionType()->ConstituentTypes().size() == 1U) {
        clone = clone->AsETSUnionType()->ConstituentTypes().front();
    }

    return std::make_pair(sourceType, clone);
}

Type *ETSUnionType::FindTypeIsCastableToThis(ir::Expression *node, TypeRelation *relation, Type *source) const
{
    ASSERT(node);
    bool nodeWasSet = false;
    if (relation->GetNode() == nullptr) {
        nodeWasSet = true;
        relation->SetNode(node);
    }
    // Prioritize object to object conversion
    auto it = std::find_if(constituentTypes_.begin(), constituentTypes_.end(), [relation, source](Type *target) {
        relation->IsCastableTo(source, target);
        return relation->IsTrue() && source->IsETSReferenceType() && target->IsETSReferenceType();
    });
    if (it != constituentTypes_.end()) {
        if (nodeWasSet) {
            relation->SetNode(nullptr);
        }
        return *it;
    }
    it = std::find_if(constituentTypes_.begin(), constituentTypes_.end(), [relation, source](Type *target) {
        relation->IsCastableTo(source, target);
        return relation->IsTrue();
    });
    if (nodeWasSet) {
        relation->SetNode(nullptr);
    }
    if (it != constituentTypes_.end()) {
        return *it;
    }
    return nullptr;
}

Type *ETSUnionType::FindTypeIsCastableToSomeType(ir::Expression *node, TypeRelation *relation, Type *target) const
{
    ASSERT(node);
    bool nodeWasSet = false;
    if (relation->GetNode() == nullptr) {
        nodeWasSet = true;
        relation->SetNode(node);
        relation->SetFlags(TypeRelationFlag::CASTING_CONTEXT | checker::TypeRelationFlag::ONLY_CHECK_WIDENING);
    }
    auto isCastablePred = [](TypeRelation *r, Type *sourceType, Type *targetType) {
        if (targetType->IsETSUnionType()) {
            auto *foundTargetType = targetType->AsETSUnionType()->FindTypeIsCastableToThis(r->GetNode(), r, sourceType);
            r->Result(foundTargetType != nullptr);
        } else {
            r->IsCastableTo(sourceType, targetType);
        }
        return r->IsTrue();
    };
    // Prioritize object to object conversion
    auto it = std::find_if(constituentTypes_.begin(), constituentTypes_.end(),
                           [relation, target, &isCastablePred](Type *source) {
                               return isCastablePred(relation, source, target) && source->IsETSReferenceType() &&
                                      target->IsETSReferenceType();
                           });
    if (it != constituentTypes_.end()) {
        if (nodeWasSet) {
            relation->SetNode(nullptr);
            relation->RemoveFlags(TypeRelationFlag::CASTING_CONTEXT);
        }
        return *it;
    }
    it = std::find_if(
        constituentTypes_.begin(), constituentTypes_.end(),
        [relation, target, &isCastablePred](Type *source) { return isCastablePred(relation, source, target); });
    if (nodeWasSet) {
        relation->SetNode(nullptr);
        relation->RemoveFlags(TypeRelationFlag::CASTING_CONTEXT);
    }
    if (it != constituentTypes_.end()) {
        return *it;
    }
    return nullptr;
}

Type *ETSUnionType::FindUnboxableType() const
{
    auto it = std::find_if(constituentTypes_.begin(), constituentTypes_.end(),
                           [](Type *t) { return t->IsETSUnboxableObject(); });
    if (it != constituentTypes_.end()) {
        return *it;
    }
    return nullptr;
}

bool ETSUnionType::HasObjectType(ETSObjectFlags flag) const
{
    auto it = std::find_if(constituentTypes_.begin(), constituentTypes_.end(),
                           [flag](Type *t) { return t->AsETSObjectType()->HasObjectFlag(flag); });
    return it != constituentTypes_.end();
}

bool ETSUnionType::IsNumericUnion() const
{
    return std::all_of(constituentTypes_.begin(), constituentTypes_.end(),
                       [](Type *t) { return t->HasTypeFlag(ETS_NORMALIZABLE_NUMERIC); });
}

bool ETSUnionType::IsNumericUnion(ArenaVector<Type *> &types)
{
    return std::all_of(types.begin(), types.end(), [](Type *t) { return t->HasTypeFlag(ETS_NORMALIZABLE_NUMERIC); });
}

Type *ETSUnionType::FindExactOrBoxedType(ETSChecker *checker, Type *const type) const
{
    auto it = std::find_if(constituentTypes_.begin(), constituentTypes_.end(), [checker, type](Type *ct) {
        if (ct->IsETSUnboxableObject()) {
            auto *const unboxedCt = checker->ETSBuiltinTypeAsPrimitiveType(ct);
            return unboxedCt == type;
        }
        return ct == type;
    });
    if (it != constituentTypes_.end()) {
        return *it;
    }
    return nullptr;
}

std::tuple<bool, bool> ETSUnionType::ResolveConditionExpr() const
{
    if (PossiblyETSString()) {
        return {false, false};
    }
    if (std::all_of(ConstituentTypes().begin(), ConstituentTypes().end(),
                    [](checker::Type const *ct) { return ct->DefinitelyETSNullish(); })) {
        return {true, false};
    }
    // We have to test if union can contain builtin numerics or string types to infer "true"
    return {false, false};
}

bool ETSUnionType::HasUndefinedType() const
{
    for (const auto &type : constituentTypes_) {
        if (type->IsETSUndefinedType()) {
            return true;
        }
    }
    return false;
}
}  // namespace ark::es2panda::checker
