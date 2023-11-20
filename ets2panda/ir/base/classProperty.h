/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ES2PANDA_PARSER_INCLUDE_AST_CLASS_PROPERTY_H
#define ES2PANDA_PARSER_INCLUDE_AST_CLASS_PROPERTY_H

#include "ir/base/classElement.h"

namespace panda::es2panda::checker {
class ETSAnalyzer;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::ir {
class Expression;
class TypeNode;

class ClassProperty : public ClassElement {
public:
    ClassProperty() = delete;
    ~ClassProperty() override = default;

    NO_COPY_SEMANTIC(ClassProperty);
    NO_MOVE_SEMANTIC(ClassProperty);

    explicit ClassProperty(Expression *const key, Expression *const value, TypeNode *const type_annotation,
                           ModifierFlags const modifiers, ArenaAllocator *const allocator, bool const is_computed)
        : ClassElement(AstNodeType::CLASS_PROPERTY, key, value, modifiers, allocator, is_computed),
          type_annotation_(type_annotation)
    {
    }

    [[nodiscard]] TypeNode *TypeAnnotation() const noexcept
    {
        return type_annotation_;
    }

    [[nodiscard]] PrivateFieldKind ToPrivateFieldKind(bool const is_static) const override
    {
        return is_static ? PrivateFieldKind::STATIC_FIELD : PrivateFieldKind::FIELD;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    [[nodiscard]] ClassProperty *Clone(ArenaAllocator *allocator, AstNode *parent = nullptr) override;

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;

    void Dump(ir::AstDumper *dumper) const override;

    void Compile(compiler::PandaGen *pg) const override;
    void Compile(compiler::ETSGen *etsg) const override;
    checker::Type *Check(checker::TSChecker *checker) override;
    checker::Type *Check(checker::ETSChecker *checker) override;

private:
    TypeNode *type_annotation_;
};
}  // namespace panda::es2panda::ir

#endif
