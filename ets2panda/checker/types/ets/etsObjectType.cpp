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

#include "etsObjectType.h"

#include "varbinder/declaration.h"
#include "checker/ETSchecker.h"
#include "checker/ets/conversion.h"
#include "checker/types/typeFlag.h"
#include "checker/types/typeRelation.h"
#include "ir/base/methodDefinition.h"
#include "ir/base/scriptFunction.h"
#include "ir/expressions/identifier.h"

namespace ark::es2panda::checker {

void ETSObjectType::Iterate(const PropertyTraverser &cb) const
{
    for (const auto *prop : GetAllProperties()) {
        cb(prop);
    }

    if (superType_ != nullptr) {
        superType_->Iterate(cb);
    }

    for (const auto *interface : interfaces_) {
        interface->Iterate(cb);
    }
}

varbinder::LocalVariable *ETSObjectType::GetProperty(const util::StringView &name, PropertySearchFlags flags) const
{
    varbinder::LocalVariable *res {};
    if ((flags & PropertySearchFlags::SEARCH_INSTANCE_FIELD) != 0) {
        res = GetOwnProperty<PropertyType::INSTANCE_FIELD>(name);
    }

    if (res == nullptr && ((flags & PropertySearchFlags::SEARCH_STATIC_FIELD) != 0)) {
        res = GetOwnProperty<PropertyType::STATIC_FIELD>(name);
    }

    if (res == nullptr && ((flags & PropertySearchFlags::SEARCH_INSTANCE_DECL) != 0)) {
        res = GetOwnProperty<PropertyType::INSTANCE_DECL>(name);
    }

    if (res == nullptr && ((flags & PropertySearchFlags::SEARCH_STATIC_DECL) != 0)) {
        res = GetOwnProperty<PropertyType::STATIC_DECL>(name);
    }

    if (res == nullptr && (flags & PropertySearchFlags::SEARCH_METHOD) != 0) {
        res = GetOwnProperty<PropertyType::INSTANCE_FIELD>(name);
        if (res != nullptr && res->TsType() != nullptr && res->TsType()->IsETSDynamicType()) {
            return res;
        }
        res = nullptr;
        if ((flags & PropertySearchFlags::DISALLOW_SYNTHETIC_METHOD_CREATION) != 0) {
            if ((flags & PropertySearchFlags::SEARCH_INSTANCE_METHOD) != 0) {
                res = GetOwnProperty<PropertyType::INSTANCE_METHOD>(name);
            }

            if (res == nullptr && ((flags & PropertySearchFlags::SEARCH_STATIC_METHOD) != 0)) {
                res = GetOwnProperty<PropertyType::STATIC_METHOD>(name);
            }
        } else {
            res = CreateSyntheticVarFromEverySignature(name, flags);
        }
    }

    if ((flags & (PropertySearchFlags::SEARCH_IN_INTERFACES | PropertySearchFlags::SEARCH_IN_BASE)) == 0) {
        return res;
    }

    if (res != nullptr) {
        return res;
    }

    if ((flags & PropertySearchFlags::SEARCH_IN_INTERFACES) != 0) {
        for (auto *interface : interfaces_) {
            res = interface->GetProperty(name, flags);
            if (res != nullptr) {
                return res;
            }
        }
    }

    if (superType_ != nullptr && ((flags & PropertySearchFlags::SEARCH_IN_BASE) != 0)) {
        res = superType_->GetProperty(name, flags);
    }

    return res;
}

varbinder::LocalVariable *ETSObjectType::CreateSyntheticVarFromEverySignature(const util::StringView &name,
                                                                              PropertySearchFlags flags) const
{
    varbinder::LocalVariable *res = allocator_->New<varbinder::LocalVariable>(varbinder::VariableFlags::SYNTHETIC |
                                                                              varbinder::VariableFlags::METHOD);
    ETSFunctionType *funcType = CreateETSFunctionType(name);
    funcType->AddTypeFlag(TypeFlag::SYNTHETIC);

    varbinder::LocalVariable *functionalInterface = CollectSignaturesForSyntheticType(funcType, name, flags);

    if (functionalInterface != nullptr) {
        return functionalInterface;
    }

    if (funcType->CallSignatures().empty()) {
        return nullptr;
    }

    res->SetTsType(funcType);
    funcType->SetVariable(res);
    return res;
}

ETSFunctionType *ETSObjectType::CreateETSFunctionType(const util::StringView &name) const
{
    return allocator_->New<ETSFunctionType>(name, allocator_);
}

varbinder::LocalVariable *ETSObjectType::CollectSignaturesForSyntheticType(ETSFunctionType *funcType,
                                                                           const util::StringView &name,
                                                                           PropertySearchFlags flags) const
{
    auto const addSignature = [funcType, flags](varbinder::LocalVariable *found) -> void {
        for (auto *it : found->TsType()->AsETSFunctionType()->CallSignatures()) {
            if (((flags & PropertySearchFlags::IGNORE_ABSTRACT) != 0) &&
                it->HasSignatureFlag(SignatureFlags::ABSTRACT)) {
                continue;
            }

            funcType->AddCallSignature(it);
        }
    };

    // During function reference resolution, if the found properties type is not a function type, then it is a
    // functional interface, because no other property can be found in the methods of the class. We have to
    // return the found property, because we doesn't need to create a synthetic variable for functional
    // interfaces due to the fact, that by nature they behave as fields, and can't have overloads, and they are
    // subjected to hiding
    if ((flags & PropertySearchFlags::SEARCH_STATIC_METHOD) != 0) {
        if (auto *found = GetOwnProperty<PropertyType::STATIC_METHOD>(name); found != nullptr) {
            if (found->HasFlag(varbinder::VariableFlags::METHOD_REFERENCE)) {
                // Functional interface found
                return found;
            }

            ASSERT(found->TsType()->IsETSFunctionType());
            addSignature(found);
        }
    }

    if ((flags & PropertySearchFlags::SEARCH_INSTANCE_METHOD) != 0) {
        if (auto *found = GetOwnProperty<PropertyType::INSTANCE_METHOD>(name); found != nullptr) {
            if (found->HasFlag(varbinder::VariableFlags::METHOD_REFERENCE)) {
                // Functional interface found
                return found;
            }

            ASSERT(found->TsType()->IsETSFunctionType());
            addSignature(found);
        }
    }

    if (superType_ != nullptr && ((flags & PropertySearchFlags::SEARCH_IN_BASE) != 0)) {
        return superType_->CollectSignaturesForSyntheticType(funcType, name, flags);
    }

    return nullptr;
}

std::vector<varbinder::LocalVariable *> ETSObjectType::GetAllProperties() const
{
    std::vector<varbinder::LocalVariable *> allProperties;
    for (const auto &[_, prop] : InstanceFields()) {
        (void)_;
        allProperties.push_back(prop);
    }

    for (const auto &[_, prop] : StaticFields()) {
        (void)_;
        allProperties.push_back(prop);
    }

    for (const auto &[_, prop] : InstanceMethods()) {
        (void)_;
        allProperties.push_back(prop);
    }

    for (const auto &[_, prop] : StaticMethods()) {
        (void)_;
        allProperties.push_back(prop);
    }

    for (const auto &[_, prop] : InstanceDecls()) {
        (void)_;
        allProperties.push_back(prop);
    }

    for (const auto &[_, prop] : StaticDecls()) {
        (void)_;
        allProperties.push_back(prop);
    }

    return allProperties;
}

std::vector<varbinder::LocalVariable *> ETSObjectType::Methods() const
{
    std::vector<varbinder::LocalVariable *> methods;
    for (const auto &[_, prop] : InstanceMethods()) {
        (void)_;
        methods.push_back(prop);
    }

    for (const auto &[_, prop] : StaticMethods()) {
        (void)_;
        methods.push_back(prop);
    }

    return methods;
}

std::vector<varbinder::LocalVariable *> ETSObjectType::Fields() const
{
    std::vector<varbinder::LocalVariable *> fields;
    for (const auto &[_, prop] : InstanceFields()) {
        (void)_;
        fields.push_back(prop);
    }

    for (const auto &[_, prop] : StaticFields()) {
        (void)_;
        fields.push_back(prop);
    }

    return fields;
}

std::vector<const varbinder::LocalVariable *> ETSObjectType::ForeignProperties() const
{
    std::vector<const varbinder::LocalVariable *> foreignProps;
    std::unordered_set<util::StringView> ownProps;

    EnsurePropertiesInstantiated();
    ownProps.reserve(properties_.size());

    for (const auto *prop : GetAllProperties()) {
        ownProps.insert(prop->Name());
    }

    auto allProps = CollectAllProperties();
    for (const auto &[name, var] : allProps) {
        if (ownProps.find(name) == ownProps.end()) {
            foreignProps.push_back(var);
        }
    }

    return foreignProps;
}

ArenaMap<util::StringView, const varbinder::LocalVariable *> ETSObjectType::CollectAllProperties() const
{
    ArenaMap<util::StringView, const varbinder::LocalVariable *> propMap(allocator_->Adapter());
    EnsurePropertiesInstantiated();
    Iterate([&propMap](const varbinder::LocalVariable *var) { propMap.emplace(var->Name(), var); });

    return propMap;
}

void ETSObjectType::ToString(std::stringstream &ss, bool precise) const
{
    if (HasObjectFlag(ETSObjectFlags::FUNCTIONAL)) {
        if (IsNullish() && this != GetConstOriginalBaseType() && !name_.Is("NullType") && !IsETSNullLike() &&
            !name_.Empty()) {
            ss << lexer::TokenToString(lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS);
        }
        GetFunctionalInterfaceInvokeType()->ToString(ss, precise);
    } else if (precise) {
        ss << assemblerName_;  // NOTE(gogabr): need full qualified name
    } else {
        ss << name_;
    }

    if (!typeArguments_.empty()) {
        ss << compiler::Signatures::GENERIC_BEGIN;
        for (auto arg = typeArguments_.cbegin(); arg != typeArguments_.cend(); ++arg) {
            (*arg)->ToString(ss, precise);

            if (next(arg) != typeArguments_.cend()) {
                ss << lexer::TokenToString(lexer::TokenType::PUNCTUATOR_COMMA);
            }
        }
        ss << compiler::Signatures::GENERIC_END;
    }

    if (IsNullish() && this != GetConstOriginalBaseType() && !name_.Is("NullType") && !IsETSNullLike() &&
        !name_.Empty()) {
        if (HasObjectFlag(ETSObjectFlags::FUNCTIONAL)) {
            ss << lexer::TokenToString(lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS);
        }
        if (ContainsNull()) {
            ss << lexer::TokenToString(lexer::TokenType::PUNCTUATOR_BITWISE_OR)
               << lexer::TokenToString(lexer::TokenType::LITERAL_NULL);
        }
        if (ContainsUndefined()) {
            ss << lexer::TokenToString(lexer::TokenType::PUNCTUATOR_BITWISE_OR)
               << lexer::TokenToString(lexer::TokenType::KEYW_UNDEFINED);
        }
    }
}

void ETSObjectType::IdenticalUptoNullabilityAndTypeArguments(TypeRelation *relation, Type *other)
{
    relation->Result(false);
    if (!other->IsETSObjectType() || !CheckIdenticalFlags(other->AsETSObjectType()->ObjectFlags())) {
        return;
    }

    auto *thisBase = GetOriginalBaseType();
    auto *otherBase = other->AsETSObjectType()->GetOriginalBaseType();
    if (thisBase->Variable() != otherBase->Variable()) {
        return;
    }

    if ((relation->IgnoreTypeParameters() && !HasObjectFlag(ETSObjectFlags::FUNCTIONAL)) || (this == other)) {
        relation->Result(true);
        return;
    }

    if (IsNullish()) {
        relation->Result(true);
        return;
    }

    auto const sourceTypeArguments = other->AsETSObjectType()->TypeArguments();
    if (typeArguments_.empty() != sourceTypeArguments.empty()) {
        return;
    }

    relation->Result(true);
}

void ETSObjectType::IdenticalUptoNullability(TypeRelation *relation, Type *other)
{
    IdenticalUptoNullabilityAndTypeArguments(relation, other);

    if (!relation->IsTrue() || !HasTypeFlag(TypeFlag::GENERIC)) {
        return;
    }

    auto const otherTypeArguments = other->AsETSObjectType()->TypeArguments();

    auto const argsNumber = typeArguments_.size();
    ASSERT(argsNumber == otherTypeArguments.size());

    for (size_t idx = 0U; idx < argsNumber; ++idx) {
        if (typeArguments_[idx]->IsWildcardType() || otherTypeArguments[idx]->IsWildcardType()) {
            continue;
        }

        // checking the nullishness of type args before getting their original base types
        // because most probably GetOriginalBaseType will return the non-nullish version of the type
        if ((!typeArguments_[idx]->IsNullish() && otherTypeArguments[idx]->IsNullish()) ||
            (typeArguments_[idx]->IsNullish() && !otherTypeArguments[idx]->IsNullish())) {
            relation->Result(false);
            return;
        }

        const auto getOriginalBaseTypeOrType = [&relation](Type *const originalType) {
            auto *const baseType = relation->GetChecker()->AsETSChecker()->GetOriginalBaseType(originalType);
            return baseType == nullptr ? originalType : baseType;
        };

        auto *const typeArgType = getOriginalBaseTypeOrType(typeArguments_[idx]);
        auto *const otherTypeArgType = getOriginalBaseTypeOrType(otherTypeArguments[idx]);

        typeArgType->Identical(relation, otherTypeArgType);
        if (!relation->IsTrue()) {
            return;
        }
    }

    relation->Result(true);
}

void ETSObjectType::Identical(TypeRelation *relation, Type *other)
{
    if ((ContainsNull() != other->ContainsNull()) || (ContainsUndefined() != other->ContainsUndefined())) {
        return;
    }
    IdenticalUptoNullability(relation, other);
}

bool ETSObjectType::CheckIdenticalFlags(const ETSObjectFlags target) const
{
    constexpr auto FLAGS_TO_REMOVE = ETSObjectFlags::COMPLETELY_RESOLVED | ETSObjectFlags::INCOMPLETE_INSTANTIATION |
                                     ETSObjectFlags::CHECKED_COMPATIBLE_ABSTRACTS |
                                     ETSObjectFlags::CHECKED_INVOKE_LEGITIMACY;

    auto cleanedTargetFlags = target;
    cleanedTargetFlags &= ~FLAGS_TO_REMOVE;

    auto cleanedSelfFlags = ObjectFlags();
    cleanedSelfFlags &= ~FLAGS_TO_REMOVE;

    return cleanedSelfFlags == cleanedTargetFlags;
}

bool ETSObjectType::AssignmentSource(TypeRelation *const relation, Type *const target)
{
    relation->Result((IsETSNullType() && target->ContainsNull()) ||
                     (IsETSUndefinedType() && target->ContainsUndefined()));

    return relation->IsTrue();
}

void ETSObjectType::AssignmentTarget(TypeRelation *const relation, Type *source)
{
    if (source->IsETSNullType()) {
        relation->Result(ContainsNull());
        return;
    }
    if (source->IsETSUndefinedType()) {
        relation->Result(ContainsUndefined());
        return;
    }

    if ((source->ContainsNull() && !ContainsNull()) || (source->ContainsUndefined() && !ContainsUndefined())) {
        return;
    }

    if (HasObjectFlag(ETSObjectFlags::FUNCTIONAL)) {
        EnsurePropertiesInstantiated();
        auto found = properties_[static_cast<size_t>(PropertyType::INSTANCE_METHOD)].find(
            FUNCTIONAL_INTERFACE_INVOKE_METHOD_NAME);
        ASSERT(found != properties_[static_cast<size_t>(PropertyType::INSTANCE_METHOD)].end());
        if (source->IsETSFunctionType()) {
            source = source->AsETSFunctionType()->BoxPrimitives(relation->GetChecker()->AsETSChecker());
        }
        relation->IsAssignableTo(source, found->second->TsType());
        return;
    }

    relation->IsSupertypeOf(this, source);
}

bool ETSObjectType::CastWideningNarrowing(TypeRelation *const relation, Type *const target, TypeFlag unboxFlags,
                                          TypeFlag wideningFlags, TypeFlag narrowingFlags)
{
    if (target->HasTypeFlag(unboxFlags)) {
        conversion::Unboxing(relation, this);
        return true;
    }
    if (target->HasTypeFlag(wideningFlags)) {
        conversion::UnboxingWideningPrimitive(relation, this, target);
        return true;
    }
    if (target->HasTypeFlag(narrowingFlags)) {
        conversion::UnboxingNarrowingPrimitive(relation, this, target);
        return true;
    }
    return false;
}

bool ETSObjectType::CastNumericObject(TypeRelation *const relation, Type *const target)
{
    if (!target->HasTypeFlag(TypeFlag::BYTE | TypeFlag::SHORT | TypeFlag::CHAR | TypeFlag::INT | TypeFlag::LONG |
                             TypeFlag::FLOAT | TypeFlag::DOUBLE | TypeFlag::ETS_BOOLEAN)) {
        return false;
    }
    IdenticalUptoNullability(relation, target);
    if (relation->IsTrue()) {
        return true;
    }
    if (this->HasObjectFlag(ETSObjectFlags::BUILTIN_BYTE)) {
        if (target->HasTypeFlag(TypeFlag::BYTE)) {
            conversion::Unboxing(relation, this);
            return true;
        }
        if (target->HasTypeFlag(TypeFlag::SHORT | TypeFlag::INT | TypeFlag::LONG | TypeFlag::FLOAT |
                                TypeFlag::DOUBLE)) {
            conversion::UnboxingWideningPrimitive(relation, this, target);
            return true;
        }
        if (target->HasTypeFlag(TypeFlag::CHAR)) {
            conversion::UnboxingWideningNarrowingPrimitive(relation, this, target);
            return true;
        }
    }
    if (this->HasObjectFlag(ETSObjectFlags::BUILTIN_SHORT) &&
        CastWideningNarrowing(relation, target, TypeFlag::SHORT,
                              TypeFlag::INT | TypeFlag::LONG | TypeFlag::FLOAT | TypeFlag::DOUBLE,
                              TypeFlag::BYTE | TypeFlag::CHAR)) {
        return true;
    }
    if (this->HasObjectFlag(ETSObjectFlags::BUILTIN_CHAR) &&
        CastWideningNarrowing(relation, target, TypeFlag::CHAR,
                              TypeFlag::INT | TypeFlag::LONG | TypeFlag::FLOAT | TypeFlag::DOUBLE,
                              TypeFlag::BYTE | TypeFlag::SHORT)) {
        return true;
    }
    if (this->HasObjectFlag(ETSObjectFlags::BUILTIN_INT) &&
        CastWideningNarrowing(relation, target, TypeFlag::INT, TypeFlag::LONG | TypeFlag::FLOAT | TypeFlag::DOUBLE,
                              TypeFlag::BYTE | TypeFlag::SHORT | TypeFlag::CHAR)) {
        return true;
    }
    if (this->HasObjectFlag(ETSObjectFlags::BUILTIN_LONG) &&
        CastWideningNarrowing(relation, target, TypeFlag::LONG, TypeFlag::FLOAT | TypeFlag::DOUBLE,
                              TypeFlag::BYTE | TypeFlag::SHORT | TypeFlag::CHAR | TypeFlag::INT)) {
        return true;
    }
    if (this->HasObjectFlag(ETSObjectFlags::BUILTIN_FLOAT) &&
        CastWideningNarrowing(relation, target, TypeFlag::FLOAT, TypeFlag::DOUBLE,
                              TypeFlag::BYTE | TypeFlag::SHORT | TypeFlag::CHAR | TypeFlag::INT | TypeFlag::LONG)) {
        return true;
    }
    if (auto narrowingFlags =
            TypeFlag::BYTE | TypeFlag::SHORT | TypeFlag::CHAR | TypeFlag::INT | TypeFlag::LONG | TypeFlag::FLOAT;
        this->HasObjectFlag(ETSObjectFlags::BUILTIN_DOUBLE) &&
        CastWideningNarrowing(relation, target, TypeFlag::DOUBLE, TypeFlag::NONE, narrowingFlags)) {
        return true;
    }
    if (this->HasObjectFlag(ETSObjectFlags::BUILTIN_BOOLEAN) && target->HasTypeFlag(TypeFlag::ETS_BOOLEAN)) {
        conversion::Unboxing(relation, this);
        return true;
    }
    if (this->HasObjectFlag(ETSObjectFlags::UNBOXABLE_TYPE)) {
        if (target->HasTypeFlag(TypeFlag::ETS_OBJECT)) {
            if (!target->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::UNBOXABLE_TYPE)) {
                conversion::WideningReference(relation, this, target->AsETSObjectType());
                return true;
            }
            auto unboxedTarget = relation->GetChecker()->AsETSChecker()->ETSBuiltinTypeAsPrimitiveType(target);
            CastNumericObject(relation, unboxedTarget);
            if (relation->IsTrue()) {
                conversion::Boxing(relation, unboxedTarget);
                return true;
            }
            conversion::WideningReference(relation, this, target->AsETSObjectType());
            return true;
        }
        conversion::Forbidden(relation);
        return true;
    }
    if (target->HasTypeFlag(TypeFlag::BYTE | TypeFlag::SHORT | TypeFlag::CHAR | TypeFlag::INT | TypeFlag::LONG |
                            TypeFlag::FLOAT | TypeFlag::DOUBLE | TypeFlag::ETS_BOOLEAN)) {
        conversion::NarrowingReferenceUnboxing(relation, this, target);
        return true;
    }
    return false;
}

void ETSObjectType::Cast(TypeRelation *const relation, Type *const target)
{
    conversion::Identity(relation, this, target);
    if (relation->IsTrue()) {
        return;
    }

    if (this->IsETSNullLike()) {
        if (target->HasTypeFlag(TypeFlag::ETS_ARRAY_OR_OBJECT)) {
            relation->GetNode()->SetTsType(target);
            relation->Result(true);
            return;
        }

        conversion::Forbidden(relation);
        return;
    }

    if (CastNumericObject(relation, target)) {
        return;
    }

    if (target->HasTypeFlag(TypeFlag::ETS_ARRAY)) {
        conversion::NarrowingReference(relation, this, target->AsETSArrayType());
        return;
    }

    if (target->HasTypeFlag(TypeFlag::ETS_OBJECT)) {
        conversion::WideningReference(relation, this, target->AsETSObjectType());
        if (relation->IsTrue()) {
            return;
        }

        conversion::NarrowingReference(relation, this, target->AsETSObjectType());
        if (relation->IsTrue()) {
            return;
        }
    }

    conversion::Forbidden(relation);
}

bool ETSObjectType::DefaultObjectTypeChecks(const ETSChecker *const etsChecker, TypeRelation *const relation,
                                            Type *const source)
{
    // 3.8.3 Subtyping among Array Types
    auto const *const base = GetConstOriginalBaseType();
    if (base == etsChecker->GlobalETSObjectType() && source->IsETSArrayType()) {
        relation->Result(true);
        return true;
    }

    if (source->IsETSTypeParameter()) {
        IsSupertypeOf(relation, source->AsETSTypeParameter()->GetConstraintType());
        return true;
    }

    if (!source->IsETSObjectType() ||
        !source->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::CLASS | ETSObjectFlags::INTERFACE |
                                                  ETSObjectFlags::NULL_TYPE)) {
        return true;
    }

    if ((!ContainsNull() && source->ContainsNull()) || (!ContainsUndefined() && source->ContainsUndefined())) {
        return true;
    }
    // All classes and interfaces are subtypes of Object
    if (base == etsChecker->GlobalETSObjectType() || base == etsChecker->GlobalETSNullishObjectType()) {
        relation->Result(true);
        return true;
    }

    IdenticalUptoNullabilityAndTypeArguments(relation, source);
    if (relation->IsTrue() && HasTypeFlag(TypeFlag::GENERIC)) {
        IsGenericSupertypeOf(relation, source);
    }
    return relation->IsTrue();
}

static void IsSupertypeOfUnion(TypeRelation *relation, ETSObjectType *self, ETSUnionType *unionType)
{
    bool res = std::all_of(unionType->ConstituentTypes().begin(), unionType->ConstituentTypes().end(),
                           [self, relation](Type *ct) {
                               relation->Result(false);
                               self->IsSupertypeOf(relation, ct);
                               return relation->IsTrue();
                           });
    relation->Result(res);
}

void ETSObjectType::IsSupertypeOf(TypeRelation *relation, Type *source)
{
    relation->Result(false);
    auto *const etsChecker = relation->GetChecker()->AsETSChecker();

    if (source->IsETSUnionType()) {
        IsSupertypeOfUnion(relation, this, source->AsETSUnionType());
        return;
    }

    if (DefaultObjectTypeChecks(etsChecker, relation, source)) {
        return;
    }

    ETSObjectType *sourceObj = source->AsETSObjectType();
    if (auto *sourceSuper = sourceObj->SuperType(); sourceSuper != nullptr) {
        if (relation->IsSupertypeOf(this, sourceSuper)) {
            return;
        }
    }

    if (HasObjectFlag(ETSObjectFlags::INTERFACE)) {
        for (auto *itf : sourceObj->Interfaces()) {
            if (relation->IsSupertypeOf(this, itf)) {
                return;
            }
        }
    }
}

void ETSObjectType::IsGenericSupertypeOf(TypeRelation *relation, Type *source)
{
    ASSERT(HasTypeFlag(TypeFlag::GENERIC));

    auto *sourceType = source->AsETSObjectType();
    auto const sourceTypeArguments = sourceType->TypeArguments();
    ASSERT(typeArguments_.size() == sourceTypeArguments.size());

    ASSERT(declNode_ == sourceType->GetDeclNode());

    auto *typeParamsDecl = GetTypeParams();
    ASSERT(typeParamsDecl != nullptr || typeArguments_.empty());

    if (typeParamsDecl == nullptr) {
        return;
    }

    auto &typeParams = typeParamsDecl->Params();
    ASSERT(typeParams.size() == typeArguments_.size());

    for (size_t idx = 0; idx < typeArguments_.size(); idx++) {
        auto *typeArg = typeArguments_[idx];
        auto *sourceTypeArg = sourceTypeArguments[idx];
        auto *typeParam = typeParams[idx];

        relation->Result(false);

        if (!(typeArg->IsWildcardType() || sourceTypeArg->IsWildcardType())) {
            if (typeParam->IsOut()) {
                typeArg->IsSupertypeOf(relation, sourceTypeArg);
            } else if (typeParam->IsIn()) {
                sourceTypeArg->IsSupertypeOf(relation, typeArg);
            } else {
                typeArg->Identical(relation, sourceTypeArg);
            }

            if (!relation->IsTrue()) {
                return;
            }
        }
    }

    relation->Result(true);
}

Type *ETSObjectType::AsSuper(Checker *checker, varbinder::Variable *sourceVar)
{
    if (sourceVar == nullptr) {
        return nullptr;
    }

    if (variable_ == sourceVar) {
        return this;
    }

    if (HasObjectFlag(ETSObjectFlags::INTERFACE)) {
        Type *res = nullptr;
        for (auto *const it : checker->AsETSChecker()->GetInterfaces(this)) {
            res = it->AsSuper(checker, sourceVar);
            if (res != nullptr) {
                return res;
            }
        }
        return checker->GetGlobalTypesHolder()->GlobalETSObjectType()->AsSuper(checker, sourceVar);
    }

    Type *const superType = checker->AsETSChecker()->GetSuperType(this);

    if (superType == nullptr) {
        return nullptr;
    }

    if (!superType->IsETSObjectType()) {
        return nullptr;
    }

    if (ETSObjectType *const superObj = superType->AsETSObjectType(); superObj->HasObjectFlag(ETSObjectFlags::CLASS)) {
        Type *const res = superObj->AsSuper(checker, sourceVar);
        if (res != nullptr) {
            return res;
        }
    }

    if (sourceVar->TsType()->IsETSObjectType() &&
        sourceVar->TsType()->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::INTERFACE)) {
        for (auto *const it : checker->AsETSChecker()->GetInterfaces(this)) {
            Type *const res = it->AsSuper(checker, sourceVar);
            if (res != nullptr) {
                return res;
            }
        }
    }

    return nullptr;
}

varbinder::LocalVariable *ETSObjectType::CopyProperty(varbinder::LocalVariable *prop, ArenaAllocator *allocator,
                                                      TypeRelation *relation, GlobalTypesHolder *globalTypes)
{
    auto *const copiedProp = prop->Copy(allocator, prop->Declaration());
    auto *const copiedPropType = ETSChecker::TryToInstantiate(
        relation->GetChecker()->AsETSChecker()->GetTypeOfVariable(prop), allocator, relation, globalTypes);
    // NOTE: don't change type variable if it differs from copying one!
    if (copiedPropType->Variable() == prop) {
        copiedPropType->SetVariable(copiedProp);
    }
    copiedProp->SetTsType(copiedPropType);
    return copiedProp;
}

Type *ETSObjectType::Instantiate(ArenaAllocator *const allocator, TypeRelation *const relation,
                                 GlobalTypesHolder *const globalTypes)
{
    auto *const checker = relation->GetChecker()->AsETSChecker();
    std::lock_guard guard {*checker->Mutex()};
    auto *const base = GetOriginalBaseType();

    if (!relation->TypeInstantiationPossible(base) || IsETSNullLike()) {
        return this;
    }
    relation->IncreaseTypeRecursionCount(base);

    auto *const copiedType = checker->CreateNewETSObjectType(name_, declNode_, flags_);
    copiedType->typeFlags_ = typeFlags_;
    copiedType->RemoveObjectFlag(ETSObjectFlags::CHECKED_COMPATIBLE_ABSTRACTS |
                                 ETSObjectFlags::INCOMPLETE_INSTANTIATION | ETSObjectFlags::CHECKED_INVOKE_LEGITIMACY);
    copiedType->SetAssemblerName(assemblerName_);
    copiedType->SetVariable(variable_);
    copiedType->SetSuperType(superType_);

    for (auto *const it : interfaces_) {
        copiedType->AddInterface(it);
    }

    for (auto *const typeArgument : TypeArguments()) {
        copiedType->TypeArguments().emplace_back(typeArgument->Instantiate(allocator, relation, globalTypes));
    }
    copiedType->SetBaseType(this);
    copiedType->propertiesInstantiated_ = false;
    copiedType->relation_ = relation;
    copiedType->substitution_ = nullptr;

    relation->DecreaseTypeRecursionCount(base);

    return copiedType;
}

static varbinder::LocalVariable *CopyPropertyWithTypeArguments(varbinder::LocalVariable *prop, TypeRelation *relation,
                                                               const Substitution *substitution)
{
    auto *const checker = relation->GetChecker()->AsETSChecker();
    auto *const varType = ETSChecker::IsVariableGetterSetter(prop) ? prop->TsType() : checker->GetTypeOfVariable(prop);
    auto *const copiedPropType = varType->Substitute(relation, substitution);
    auto *const copiedProp = prop->Copy(checker->Allocator(), prop->Declaration());
    copiedPropType->SetVariable(copiedProp);
    copiedProp->SetTsType(copiedPropType);
    return copiedProp;
}

ETSObjectType const *ETSObjectType::GetConstOriginalBaseType() const noexcept
{
    if (auto *baseIter = GetBaseType(); baseIter != nullptr) {
        auto *baseIterNext = baseIter->GetBaseType();
        while (baseIterNext != nullptr && baseIterNext != baseIter) {
            baseIter = baseIterNext;
            baseIterNext = baseIter->GetBaseType();
        }
        return baseIter;
    }
    return this;
}

bool ETSObjectType::SubstituteTypeArgs(TypeRelation *const relation, ArenaVector<Type *> &newTypeArgs,
                                       const Substitution *const substitution)
{
    bool anyChange = false;
    newTypeArgs.reserve(typeArguments_.size());

    for (auto *const arg : typeArguments_) {
        auto *const newArg = arg->Substitute(relation, substitution);
        newTypeArgs.push_back(newArg);
        anyChange = anyChange || (newArg != arg);
    }

    return anyChange;
}

void ETSObjectType::SetCopiedTypeProperties(TypeRelation *const relation, ETSObjectType *const copiedType,
                                            ArenaVector<Type *> &newTypeArgs, const Substitution *const substitution)
{
    copiedType->typeFlags_ = typeFlags_;
    copiedType->RemoveObjectFlag(ETSObjectFlags::CHECKED_COMPATIBLE_ABSTRACTS |
                                 ETSObjectFlags::INCOMPLETE_INSTANTIATION | ETSObjectFlags::CHECKED_INVOKE_LEGITIMACY);
    copiedType->SetVariable(variable_);
    copiedType->SetBaseType(this);

    copiedType->SetTypeArguments(std::move(newTypeArgs));
    copiedType->relation_ = relation;
    copiedType->substitution_ = substitution;
}

ETSObjectType *ETSObjectType::Substitute(TypeRelation *relation, const Substitution *substitution, bool cache)
{
    if (substitution == nullptr || substitution->empty()) {
        return this;
    }

    auto *const checker = relation->GetChecker()->AsETSChecker();
    auto *base = GetOriginalBaseType();

    ArenaVector<Type *> newTypeArgs {checker->Allocator()->Adapter()};
    const bool anyChange = SubstituteTypeArgs(relation, newTypeArgs, substitution);
    // Lambda types can capture type params in their bodies, normal classes cannot.
    // NOTE: gogabr. determine precise conditions where we do not need to copy.
    // Perhaps keep track of captured type parameters for each type.
    if (!anyChange && !HasObjectFlag(ETSObjectFlags::FUNCTIONAL)) {
        return this;
    }

    const util::StringView hash = checker->GetHashFromSubstitution(substitution);
    if (cache) {
        if (auto *inst = GetInstantiatedType(hash); inst != nullptr) {
            return inst;
        }
    }

    if (!relation->TypeInstantiationPossible(base) || IsETSNullLike()) {
        return this;
    }
    relation->IncreaseTypeRecursionCount(base);

    auto *const copiedType = checker->CreateNewETSObjectType(name_, declNode_, flags_);
    SetCopiedTypeProperties(relation, copiedType, newTypeArgs, substitution);

    if (cache) {
        GetInstantiationMap().try_emplace(hash, copiedType);
    }

    if (superType_ != nullptr) {
        copiedType->SetSuperType(superType_->Substitute(relation, substitution)->AsETSObjectType());
    }
    for (auto *itf : interfaces_) {
        auto *newItf = itf->Substitute(relation, substitution)->AsETSObjectType();
        copiedType->AddInterface(newItf);
    }

    relation->DecreaseTypeRecursionCount(base);

    return copiedType;
}

ETSObjectType *ETSObjectType::Substitute(TypeRelation *relation, const Substitution *substitution)
{
    return Substitute(relation, substitution, true);
}

void ETSObjectType::InstantiateProperties() const
{
    if (baseType_ == nullptr || baseType_ == this) {
        return;
    }
    ASSERT(!propertiesInstantiated_);
    ASSERT(relation_ != nullptr);

    for (auto *const it : baseType_->ConstructSignatures()) {
        auto *newSig = it->Substitute(relation_, substitution_);
        constructSignatures_.push_back(newSig);
    }

    for (auto const &[_, prop] : baseType_->InstanceFields()) {
        (void)_;
        auto *copiedProp = CopyPropertyWithTypeArguments(prop, relation_, substitution_);
        properties_[static_cast<size_t>(PropertyType::INSTANCE_FIELD)].emplace(prop->Name(), copiedProp);
    }

    for (auto const &[_, prop] : baseType_->StaticFields()) {
        (void)_;
        auto *copiedProp = CopyPropertyWithTypeArguments(prop, relation_, substitution_);
        properties_[static_cast<size_t>(PropertyType::STATIC_FIELD)].emplace(prop->Name(), copiedProp);
    }

    for (auto const &[_, prop] : baseType_->InstanceMethods()) {
        (void)_;
        auto *copiedProp = CopyPropertyWithTypeArguments(prop, relation_, substitution_);
        properties_[static_cast<size_t>(PropertyType::INSTANCE_METHOD)].emplace(prop->Name(), copiedProp);
    }

    for (auto const &[_, prop] : baseType_->StaticMethods()) {
        (void)_;
        auto *copiedProp = CopyPropertyWithTypeArguments(prop, relation_, substitution_);
        properties_[static_cast<size_t>(PropertyType::STATIC_METHOD)].emplace(prop->Name(), copiedProp);
    }

    for (auto const &[_, prop] : baseType_->InstanceDecls()) {
        (void)_;
        auto *copiedProp = CopyPropertyWithTypeArguments(prop, relation_, substitution_);
        properties_[static_cast<size_t>(PropertyType::INSTANCE_DECL)].emplace(prop->Name(), copiedProp);
    }

    for (auto const &[_, prop] : baseType_->StaticDecls()) {
        (void)_;
        auto *copiedProp = CopyPropertyWithTypeArguments(prop, relation_, substitution_);
        properties_[static_cast<size_t>(PropertyType::STATIC_DECL)].emplace(prop->Name(), copiedProp);
    }
}

void ETSObjectType::DebugInfoTypeFromName(std::stringstream &ss, util::StringView asmName)
{
    ss << compiler::Signatures::CLASS_REF_BEGIN;
    auto copied = asmName.Mutf8();
    std::replace(copied.begin(), copied.end(), *compiler::Signatures::METHOD_SEPARATOR.begin(),
                 *compiler::Signatures::NAMESPACE_SEPARATOR.begin());
    ss << copied;
    ss << compiler::Signatures::MANGLE_SEPARATOR;
}

}  // namespace ark::es2panda::checker
