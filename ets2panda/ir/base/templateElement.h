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

#ifndef ES2PANDA_PARSER_INCLUDE_AST_TEMPLATE_ELEMENT_H
#define ES2PANDA_PARSER_INCLUDE_AST_TEMPLATE_ELEMENT_H

#include "ir/expression.h"
#include "util/ustring.h"

namespace panda::es2panda::ir {
class TemplateElement : public Expression {
public:
    explicit TemplateElement() : Expression(AstNodeType::TEMPLATE_ELEMENT) {}

    explicit TemplateElement(util::StringView raw, util::StringView cooked)
        : Expression(AstNodeType::TEMPLATE_ELEMENT), raw_(raw), cooked_(cooked)
    {
    }

    const util::StringView &Raw() const
    {
        return raw_;
    }

    const util::StringView &Cooked() const
    {
        return cooked_;
    }

    void TransformChildren(const NodeTransformer &cb) override;
    void Iterate(const NodeTraverser &cb) const override;
    void Dump(ir::AstDumper *dumper) const override;
    void Compile([[maybe_unused]] compiler::PandaGen *pg) const override;
    void Compile([[maybe_unused]] compiler::ETSGen *etsg) const override;
    checker::Type *Check([[maybe_unused]] checker::TSChecker *checker) override;
    checker::Type *Check([[maybe_unused]] checker::ETSChecker *checker) override;

private:
    util::StringView raw_ {};
    util::StringView cooked_ {};
};
}  // namespace panda::es2panda::ir

#endif
