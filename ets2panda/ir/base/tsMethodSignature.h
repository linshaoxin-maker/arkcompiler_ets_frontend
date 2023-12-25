/**
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

#ifndef ES2PANDA_PARSER_INCLUDE_AST_TS_METHOD_SIGNATURE_H
#define ES2PANDA_PARSER_INCLUDE_AST_TS_METHOD_SIGNATURE_H

#include "ir/typeNode.h"
#include "ir/base/scriptFunctionSignature.h"

namespace panda::es2panda::checker {
class TSAnalyzer;
class ETSAnalyzer;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::ir {
class TSTypeParameterDeclaration;

class TSMethodSignature : public AstNode {
public:
    TSMethodSignature() = delete;
    ~TSMethodSignature() override = default;

    NO_COPY_SEMANTIC(TSMethodSignature);
    NO_MOVE_SEMANTIC(TSMethodSignature);

    explicit TSMethodSignature(Expression *key, ir::FunctionSignature &&signature, bool computed, bool optional)
        : AstNode(AstNodeType::TS_METHOD_SIGNATURE),
          key_(key),
          signature_(std::move(signature)),
          computed_(computed),
          optional_(optional)
    {
    }

    // NOTE (csabahurton): friend relationship can be removed once there are getters for private fields
    friend class checker::TSAnalyzer;

    bool IsScopeBearer() const override
    {
        return true;
    }

    varbinder::Scope *Scope() const override
    {
        return scope_;
    }

    void SetScope(varbinder::Scope *scope)
    {
        scope_ = scope;
    }

    [[nodiscard]] const Expression *Key() const noexcept
    {
        return key_;
    }

    [[nodiscard]] Expression *Key() noexcept
    {
        return key_;
    }

    [[nodiscard]] const TSTypeParameterDeclaration *TypeParams() const noexcept
    {
        return signature_.TypeParams();
    }

    [[nodiscard]] TSTypeParameterDeclaration *TypeParams()
    {
        return signature_.TypeParams();
    }

    [[nodiscard]] const ArenaVector<Expression *> &Params() const noexcept
    {
        return signature_.Params();
    }

    [[nodiscard]] const TypeNode *ReturnTypeAnnotation() const noexcept
    {
        return signature_.ReturnType();
    }

    TypeNode *ReturnTypeAnnotation()
    {
        return signature_.ReturnType();
    }

    [[nodiscard]] bool Computed() const noexcept
    {
        return computed_;
    }

    [[nodiscard]] bool Optional() const noexcept
    {
        return optional_;
    }

    void TransformChildren(const NodeTransformer &cb) override;
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
    varbinder::Scope *scope_ {nullptr};
    Expression *key_;
    ir::FunctionSignature signature_;
    bool computed_;
    bool optional_;
};
}  // namespace panda::es2panda::ir

#endif
