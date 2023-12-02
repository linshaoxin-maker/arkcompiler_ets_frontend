/**
 * Copyright (c) 2021 2023 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_IR_EXPRESSION_ARRAY_EXPRESSION_H
#define ES2PANDA_IR_EXPRESSION_ARRAY_EXPRESSION_H

#include "ir/expression.h"
#include "ir/validationInfo.h"

namespace panda::es2panda::ir {
class ArrayExpression : public AnnotatedExpression {
private:
    struct Tag {};

public:
    ArrayExpression() = delete;
    ~ArrayExpression() override = default;

    NO_COPY_SEMANTIC(ArrayExpression);
    NO_MOVE_SEMANTIC(ArrayExpression);

public:
    explicit ArrayExpression(ArenaVector<Expression *> &&elements, ArenaAllocator *const allocator)
        : ArrayExpression(AstNodeType::ARRAY_EXPRESSION, std::move(elements), allocator, false)
    {
    }

    explicit ArrayExpression(AstNodeType node_type, ArenaVector<Expression *> &&elements,
                             ArenaAllocator *const allocator, bool const trailing_comma)
        : AnnotatedExpression(node_type),
          decorators_(allocator->Adapter()),
          elements_(std::move(elements)),
          trailing_comma_(trailing_comma)
    {
    }

    explicit ArrayExpression(Tag tag, ArrayExpression const &other, ArenaAllocator *allocator);

    [[nodiscard]] const ArenaVector<Expression *> &Elements() const noexcept
    {
        return elements_;
    }

    [[nodiscard]] ArenaVector<Expression *> &Elements() noexcept
    {
        return elements_;
    }

    [[nodiscard]] bool IsDeclaration() const noexcept
    {
        return is_declaration_;
    }

    [[nodiscard]] bool IsOptional() const noexcept
    {
        return optional_;
    }

    void SetDeclaration() noexcept
    {
        is_declaration_ = true;
    }

    void SetOptional(bool optional) noexcept
    {
        optional_ = optional;
    }

    void SetPreferredType(checker::Type *preferred_type) noexcept
    {
        preferred_type_ = preferred_type;
    }

    [[nodiscard]] checker::Type *GetPreferredType() noexcept
    {
        return preferred_type_;
    }

    [[nodiscard]] const ArenaVector<Decorator *> &Decorators() const noexcept
    {
        return decorators_;
    }

    const ArenaVector<Decorator *> *DecoratorsPtr() const override
    {
        return &Decorators();
    }

    void AddDecorators([[maybe_unused]] ArenaVector<ir::Decorator *> &&decorators) override
    {
        decorators_ = std::move(decorators);
    }

    bool CanHaveDecorator([[maybe_unused]] bool in_ts) const override
    {
        return true;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    [[nodiscard]] ArrayExpression *Clone(ArenaAllocator *allocator, AstNode *parent = nullptr) override;

    [[nodiscard]] bool ConvertibleToArrayPattern();
    [[nodiscard]] ValidationInfo ValidateExpression();
    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile(compiler::PandaGen *pg) const override;
    void Compile(compiler::ETSGen *etsg) const override;
    checker::Type *Check(checker::TSChecker *checker) override;
    checker::Type *Check(checker::ETSChecker *checker) override;
    checker::Type *CheckPattern(checker::TSChecker *checker);

private:
    ArenaVector<Decorator *> decorators_;
    ArenaVector<Expression *> elements_;
    checker::Type *preferred_type_ {};
    bool is_declaration_ {};
    bool trailing_comma_;
    bool optional_ {};
};
}  // namespace panda::es2panda::ir

#endif
