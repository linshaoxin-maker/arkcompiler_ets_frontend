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

#ifndef ES2PANDA_COMPILER_CHECKER_TYPES_ETS_ETS_TYPE_ALIAS_TYPE_H
#define ES2PANDA_COMPILER_CHECKER_TYPES_ETS_ETS_TYPE_ALIAS_TYPE_H

#include "checker/types/ets/etsObjectType.h"
#include "checker/types/typeMapping.h"

namespace ark::es2panda::checker {
class ETSTypeAliasType : public Type {
public:
    using InstantiationMap = ArenaUnorderedMap<util::StringView, ETSTypeAliasType *>;

    explicit ETSTypeAliasType(ETSChecker *checker, util::StringView name, bool isRecursive);

    util::StringView TypeName()
    {
        return name_;
    }

    Type *GetSubType()
    {
        return subType_;
    }

    const Type *GetSubType() const
    {
        return subType_;
    }

    void SetSubType(Type *subType)
    {
        subType_ = subType;
        AddTypeFlag(subType_->TypeFlags());
        SetVariable(subType_->Variable());
    }

    std::tuple<bool, bool> ResolveConditionExpr() const override
    {
        return {false, false};
    }

    bool IsRecursive() const {
    	return isRecursive_;
    }

    void ToString(std::stringstream &ss, bool precise) const override;

    void ToAssemblerType(std::stringstream &ss) const override;
    void ToAssemblerTypeWithRank(std::stringstream &ss) const override;
    void ToDebugInfoType(std::stringstream &ss) const override;

    void Identical(TypeRelation *relation, Type *other) override;
    void AssignmentTarget(TypeRelation *relation, Type *source) override;
    bool AssignmentSource(TypeRelation *relation, Type *target) override;
    void Cast(TypeRelation *relation, Type *target) override;
    void CastTarget(TypeRelation *relation, Type *source) override;
    void IsSupertypeOf(TypeRelation *relation, Type *source) override;
    void IsSubtypeOf(TypeRelation *relation, Type *target) override;
    Type *Instantiate(ArenaAllocator *allocator, TypeRelation *relation, GlobalTypesHolder *globalTypes) override;

    uint32_t Rank() const override;

    Type *Substitute(TypeRelation *relation, const Substitution *substitution) override;

    void ApplaySubstitution(TypeRelation *relation);

    void SetTypeArguments(ArenaVector<Type *> typeArguments);


    // NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
    #define TYPE_AS_CASTS(typeFlag, typeName)                    \
        typeName *As##typeName() override                        \
        {                                                        \
			if (typeFlag == TypeFlag::ETS_TYPE_ALIAS) {          \
			    return reinterpret_cast<typeName *>(this);       \
			}                                                    \
            return subType_->As##typeName();                     \
        }                                                        \
        const typeName *As##typeName() const override            \
        {                                                        \
			if (typeFlag == TypeFlag::ETS_TYPE_ALIAS) {          \
			  return reinterpret_cast<const typeName *>(this);   \
			}											         \
			return subType_->As##typeName();                     \
        }
        TYPE_MAPPING(TYPE_AS_CASTS)
    #undef TYPE_AS_CASTS

private:
    ETSTypeAliasType *GetInstantiatedType(util::StringView hash);
    void EmplaceInstantiatedType(util::StringView hash, ETSTypeAliasType *emplaceType);
    bool SubstituteTypeArgs(TypeRelation *const relation, ArenaVector<Type *> &newTypeArgs,
                            const Substitution *const substitution);

    void IsArgumentsIdentical(TypeRelation *relation, Type *other);

    ETSTypeAliasType *GetBaseType()
    {
        return base_ == nullptr ? this : base_;
    }

    util::StringView name_;
    bool isRecursive_;
    ETSTypeAliasType *base_ = nullptr;
    ETSTypeAliasType *parent_ = nullptr;
    Type *subType_ = nullptr;
    Type *globalETSObjectType_;
    InstantiationMap instantiationMap_;
    ArenaVector<Type *> typeArguments_;
    const Substitution *substitution_ = nullptr;
    mutable uint8_t recursionCount_ = 0;
};
}  // namespace ark::es2panda::checker

#endif /* ES2PANDA_COMPILER_CHECKER_TYPES_ETS_ETS_RECURSIVE_TYPE_H */
