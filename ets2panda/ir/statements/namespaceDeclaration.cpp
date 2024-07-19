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

#include "namespaceDeclaration.h"

#include "checker/TSchecker.h"
#include "compiler/core/ETSGen.h"
#include "compiler/core/pandagen.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"

namespace ark::es2panda::ir {

ir::MethodDefinition *CreateMethodDefinition(ArenaAllocator *allocator, ir::FunctionDeclaration *funcDecl)
{
    auto *funcExpr = util::NodeAllocator::ForceSetParent<ir::FunctionExpression>(allocator, funcDecl->Function());
    funcDecl->Function()->SetStart(funcDecl->Function()->Id()->End());
    funcExpr->SetRange(funcDecl->Function()->Range());

    ir::MethodDefinitionKind methodKind = ir::MethodDefinitionKind::EXTENSION_METHOD;
    /*if (funcDecl->Function()->IsExtensionMethod()) {
        methodKind = ir::MethodDefinitionKind::EXTENSION_METHOD;
    }*/

    auto *method = util::NodeAllocator::ForceSetParent<ir::MethodDefinition>(
        allocator, methodKind, funcDecl->Function()->Id()->Clone(allocator, nullptr), funcExpr,
        funcDecl->Function()->Modifiers(), allocator, false);
    method->SetRange(funcDecl->Range());

    return method;
}

NamespaceDeclaration::NamespaceDeclaration(ArenaAllocator *allocator, Identifier *ident, ArenaVector<Statement *> statements)
    : Statement(AstNodeType::NAMESPACE_DECLARATION), ident_(ident), statements_(std::move(statements))
{
    ASSERT(ident_ != nullptr);
    ident_->SetParent(this);

    for (auto& st : statements_) {
        if (st->IsFunctionDeclaration()) {
            st = CreateMethodDefinition(allocator, st->AsFunctionDeclaration());
        }

        st->SetParent(this);
    }
}

void NamespaceDeclaration::TransformChildren(const NodeTransformer &cb, std::string_view const transformationName)
{
    if (auto transformedNode = cb(ident_); transformedNode != ident_) {
        ident_->SetTransformedNode(transformationName, transformedNode);
        ident_ = transformedNode->AsIdentifier();
    }

    for (auto *&it : statements_) {
        if (auto *transformedNode = cb(it); it != transformedNode) {
            it->SetTransformedNode(transformationName, transformedNode);
            it = transformedNode->AsStatement();
        }
    }
}

void NamespaceDeclaration::Iterate(const NodeTraverser &cb) const
{
    cb(ident_);

    for (auto *it : statements_) {
        cb(it);
    }
}

void NamespaceDeclaration::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "NamespaceDeclaration"}, {"id", ident_}, {"statements", statements_}});
}

void NamespaceDeclaration::Dump(ir::SrcDumper *dumper) const
{
    dumper->Add("namespace ");
    ident_->Dump(dumper);
    dumper->Add(" {");
    dumper->Endl();

    for (const auto it : statements_) {
        it->Dump(dumper);
        dumper->Endl();
    }
    dumper->Add("}");
    dumper->Endl();
}

void NamespaceDeclaration::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void NamespaceDeclaration::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *NamespaceDeclaration::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *NamespaceDeclaration::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

}  // namespace ark::es2panda::ir
