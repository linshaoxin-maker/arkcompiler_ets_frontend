/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_IR_STATEMENT_DECLARE_ETS_NAMESPACE_H
#define ES2PANDA_IR_STATEMENT_DECLARE_ETS_NAMESPACE_H

#include "etsTopLevel.h"
#include "ir/ets/etsScript.h"

namespace ark::es2panda::ir {

class ETSNamespace : public ETSTopLevel {
public:
    explicit ETSNamespace(ArenaAllocator *allocator, ArenaVector<Statement *> &&statementList, Expression *expr)
        : ETSTopLevel(allocator, std::move(statementList)), expr_(expr)
    {
        type_ = AstNodeType::ETS_NAMESPACE;
    }

    [[nodiscard]] const Expression *Expr() const noexcept
    {
        return expr_;
    }

    [[nodiscard]] Expression *Expr() noexcept
    {
        return expr_;
    }

    [[nodiscard]] const util::StringView &PrivateId() const noexcept
    {
        return privateId_;
    }

    [[nodiscard]] const util::StringView &InternalName() const noexcept
    {
        return privateId_;
    }

    void SetInternalName(util::StringView internalName) noexcept
    {
        privateId_ = internalName;
    }

    void TransformChildren(const NodeTransformer &cb, std::string_view transformationName) override;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Dump(ir::SrcDumper *dumper) const override;
    void Compile(compiler::PandaGen *pg) const override;
    void Compile(compiler::ETSGen *etsg) const override;
    checker::Type *Check(checker::TSChecker *checker) override;
    checker::Type *Check(checker::ETSChecker *checker) override;
    Identifier *GetBaseName() const;

    void Accept(ASTVisitorT *v) override
    {
        v->Accept(this);
    }

private:
    util::StringView privateId_ {};
    Expression *expr_ {};
};
}  // namespace ark::es2panda::ir

#endif
