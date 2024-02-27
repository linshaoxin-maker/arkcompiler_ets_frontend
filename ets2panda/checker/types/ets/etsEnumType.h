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

#ifndef ES2PANDA_COMPILER_CHECKER_TYPES_ETS_ENUM_TYPE_H
#define ES2PANDA_COMPILER_CHECKER_TYPES_ETS_ENUM_TYPE_H

#include "checker/types/type.h"
#include "ir/base/property.h"
#include "ir/ts/tsEnumDeclaration.h"
#include "checker/types/ets/etsObjectType.h"

template <typename>
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr bool dependent_false_v = false;

namespace ark::es2panda::varbinder {
class LocalVariable;
}  // namespace ark::es2panda::varbinder

namespace ark::es2panda::checker {

std::string EnumDescription(util::StringView name);

// TODO(aber) rename ETSEnum2Type after removal old code
class ETSEnum2Type : public ETSObjectType {
public:
    ETSEnum2Type(ETSChecker *checker, util::StringView name, util::StringView assembler_name, ir::AstNode *decl_node,
                 ETSObjectFlags flags, TypeRelation *relation);

    ETSEnum2Type(ArenaAllocator *allocator, util::StringView name, util::StringView assembler_name,
                 ir::AstNode *decl_node, ETSObjectFlags flags, ir::Literal *value, TypeRelation *relation);

    bool IsSameEnumType(const ETSEnum2Type *other) const noexcept;

    bool IsLiteralType() const noexcept;

    bool IsSameEnumLiteralType(const ETSEnum2Type *other) const noexcept;

    bool AssignmentSource(TypeRelation *relation, Type *target) override;
    void AssignmentTarget(TypeRelation *relation, Type *source) override;
    void Identical(TypeRelation *relation, Type *other) override;

    void Cast(TypeRelation *relation, Type *target) override;

    static constexpr std::string_view GetIndexMethodName()
    {
        return "std.core.EnumConst.getIndex:i32;";  // TODO(aber) signatures.yaml
    }

private:
    void CreateLiteralTypes(ETSChecker *checker, util::StringView name, util::StringView assembler_name,
                            ir::AstNode *decl_node, ETSObjectFlags flags, TypeRelation *relation);

    ir::Literal *value_ = nullptr;
};
}  // namespace ark::es2panda::checker

#endif
