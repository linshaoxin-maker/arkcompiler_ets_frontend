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

#include "classProperty.h"

#include "checker/ETSchecker.h"
#include "checker/TSchecker.h"
#include "checker/types/ets/etsObjectType.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"
#include "ir/base/decorator.h"
#include "ir/typeNode.h"
#include "ir/expression.h"
#include "ir/expressions/identifier.h"

namespace ark::es2panda::ir {
void ClassProperty::TransformChildren(const NodeTransformer &cb)
{
    key_ = cb(key_)->AsExpression();

    if (value_ != nullptr) {
        value_ = cb(value_)->AsExpression();
    }

    if (typeAnnotation_ != nullptr) {
        typeAnnotation_ = static_cast<TypeNode *>(cb(typeAnnotation_));
    }

    for (auto *&it : decorators_) {
        it = cb(it)->AsDecorator();
    }
}

void ClassProperty::Iterate(const NodeTraverser &cb) const
{
    cb(key_);

    if (value_ != nullptr) {
        cb(value_);
    }

    if (typeAnnotation_ != nullptr) {
        cb(typeAnnotation_);
    }

    for (auto *it : decorators_) {
        cb(it);
    }
}

void ClassProperty::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ClassProperty"},
                 {"key", key_},
                 {"value", AstDumper::Optional(value_)},
                 {"accessibility", AstDumper::Optional(AstDumper::ModifierToString(flags_))},
                 {"abstract", AstDumper::Optional(IsAbstract())},
                 {"static", IsStatic()},
                 {"readonly", IsReadonly()},
                 {"declare", IsDeclare()},
                 {"optional", IsOptionalDeclaration()},
                 {"computed", isComputed_},
                 {"typeAnnotation", AstDumper::Optional(typeAnnotation_)},
                 {"definite", IsDefinite()},
                 {"decorators", decorators_}});
}

void ClassProperty::Dump(ir::SrcDumper *dumper) const
{
    if (IsPrivate()) {
        dumper->Add("private ");
    } else if (IsProtected()) {
        dumper->Add("protected ");
    } else if (IsInternal()) {
        dumper->Add("internal ");
    } else {
        dumper->Add("public ");
    }

    if (IsStatic()) {
        dumper->Add("static ");
    }

    if (IsReadonly()) {
        dumper->Add("readonly ");
    }

    if (key_ != nullptr) {
        key_->Dump(dumper);
    }

    if (typeAnnotation_ != nullptr) {
        dumper->Add(": ");
        typeAnnotation_->Dump(dumper);
    }

    if (value_ != nullptr) {
        dumper->Add(" = ");
        value_->Dump(dumper);
    }

    dumper->Add(";");
}

void ClassProperty::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void ClassProperty::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *ClassProperty::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *ClassProperty::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

// NOLINTNEXTLINE(google-default-arguments)
ClassProperty *ClassProperty::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    auto *const key = key_->Clone(allocator)->AsExpression();
    auto *const value = value_->Clone(allocator)->AsExpression();
    auto *const typeAnnotation = typeAnnotation_->Clone(allocator, this);

    if (auto *const clone = allocator->New<ClassProperty>(key, value, typeAnnotation, flags_, allocator, isComputed_);
        clone != nullptr) {
        if (parent != nullptr) {
            clone->SetParent(parent);
        }

        key->SetParent(clone);
        value->SetParent(clone);
        typeAnnotation->SetParent(clone);

        for (auto *const decorator : decorators_) {
            clone->AddDecorator(decorator->Clone(allocator, clone));
        }

        return clone;
    }

    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}
}  // namespace ark::es2panda::ir
