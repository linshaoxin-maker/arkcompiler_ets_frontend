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

#ifndef ES2PANDA_COMPILER_CHECKER_TYPES_ETS_ETS_RECURSIVE_TYPE_H
#define ES2PANDA_COMPILER_CHECKER_TYPES_ETS_ETS_RECURSIVE_TYPE_H

#include "checker/types/ets/etsObjectType.h"

namespace ark::es2panda::checker {
class ETSRecursiveType : public Type {
public:
    using InstantiationMap = ArenaUnorderedMap<util::StringView, ETSRecursiveType *>;

    explicit ETSRecursiveType(ETSChecker *checker, util::StringView name);

    util::StringView TypeName()
    {
        return name_;
    }

    Type *GetSubType()
    {
        return subType_;
    }

    void SetSubType(Type *subType)
    {
        subType_ = subType;
        AddTypeFlag(subType_->TypeFlags());
    }

    std::tuple<bool, bool> ResolveConditionExpr() const override
    {
        return {false, false};
    }

    void ToString(std::stringstream &ss, bool precise) const override;

    void ToAssemblerType(std::stringstream &ss) const override;
    void ToDebugInfoType(std::stringstream &ss) const override;

    void Identical(TypeRelation *relation, Type *other) override;
    void AssignmentTarget(TypeRelation *relation, Type *source) override;
    bool AssignmentSource(TypeRelation *relation, Type *target) override;
    void Cast(TypeRelation *relation, Type *target) override;
    void CastTarget(TypeRelation *relation, Type *source) override;
    void IsSupertypeOf(TypeRelation *relation, Type *source) override;
    void IsSubtypeOf(TypeRelation *relation, Type *target) override;
    Type *Instantiate(ArenaAllocator *allocator, TypeRelation *relation, GlobalTypesHolder *globalTypes) override;

    Type *Substitute(TypeRelation *relation, const Substitution *substitution) override;

    void ApplaySubstitution(TypeRelation *relation);

    void SetTypeArguments(ArenaVector<Type *> typeArguments);


    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
    #define TYPE_AS_CASTS(typeFlag, typeName)                \
        typeName *As##typeName() override                    \
        {                                                    \
            ASSERT(subType_->Is##typeName());                \
            return reinterpret_cast<typeName *>(subType_);   \
        }                                                    \
        const typeName *As##typeName() const override        \
        {                                                    \
            ASSERT(subType_->Is##typeName());                \
            return reinterpret_cast<const typeName *>(suvType_); \
        }
        TYPE_MAPPING(TYPE_AS_CASTS)
    #undef TYPE_AS_CASTS

private:
    ETSRecursiveType *GetInstantiatedType(util::StringView hash);
    void EmplaceInstantiatedType(util::StringView hash, ETSRecursiveType *emplaceType);
    bool SubstituteTypeArgs(TypeRelation *const relation, ArenaVector<Type *> &newTypeArgs,
                            const Substitution *const substitution);

    void IsArgumentsIdentical(TypeRelation *relation, Type *other);

    ETSRecursiveType *GetBaseType()
    {
        return base_ == nullptr ? this : base_;
    }

    util::StringView name_;
    ETSRecursiveType *base_ = nullptr;
    ETSRecursiveType *parent_ = nullptr;
    Type *subType_ = nullptr;
    Type *globalETSObjectType_;
    InstantiationMap instantiationMap_;
    ArenaVector<Type *> typeArguments_;
    const Substitution *substitution_ = nullptr;
    mutable uint8_t recursionCount_ = 0;
};
}  // namespace ark::es2panda::checker

#endif /* ES2PANDA_COMPILER_CHECKER_TYPES_ETS_ETS_RECURSIVE_TYPE_H */
