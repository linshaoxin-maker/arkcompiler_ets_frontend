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

#ifndef ES2PANDA_IR_STATEMENT_NAMESPACE_DECLARATION_H
#define ES2PANDA_IR_STATEMENT_NAMESPACE_DECLARATION_H

#include "ir/statement.h"
#include "varbinder/scope.h"

namespace ark::es2panda::ir {

class NamespaceDeclaration : public Statement {
public:
    explicit NamespaceDeclaration(Identifier *ident, ArenaVector<Statement *> statements);

    [[nodiscard]] bool IsScopeBearer() const noexcept override
    {
        return true;
    }

    [[nodiscard]] varbinder::LocalScope *Scope() const noexcept override
    {
        return scope_;
    }

    void SetScope(varbinder::LocalScope *scope)
    {
        ASSERT(scope_ == nullptr);
        scope_ = scope;
    }

    void ClearScope() noexcept override
    {
        scope_ = nullptr;
    }

    [[nodiscard]] const Identifier *Ident() const noexcept
    {
        return ident_;
    }

    [[nodiscard]] Identifier *Ident() noexcept
    {
        return ident_;
    }

    [[nodiscard]] const ArenaVector<Statement *> &Statements() const noexcept
    {
        return statements_;
    }

    void TransformChildren(const NodeTransformer &cb, std::string_view transformationName) override;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Dump(ir::SrcDumper *dumper) const override;
    void Compile(compiler::PandaGen *pg) const override;
    void Compile(compiler::ETSGen *etsg) const override;

    checker::Type *Check(checker::TSChecker *checker) override;
    checker::Type *Check(checker::ETSChecker *checker) override;

    void Accept(ASTVisitorT *v) override
    {
        v->Accept(this);
    }

private:
    varbinder::LocalScope *scope_ {};
    Identifier *ident_;
    ArenaVector<Statement *> statements_;
};

}  // namespace ark::es2panda::ir

#endif
