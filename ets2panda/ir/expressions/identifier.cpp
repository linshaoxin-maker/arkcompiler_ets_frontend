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

#include "identifier.h"

#include "checker/ETSchecker.h"
#include "checker/TSchecker.h"
#include "compiler/core/pandagen.h"
#include "compiler/core/ETSGen.h"
#include "ir/astDump.h"
#include "ir/srcDump.h"

namespace ark::es2panda::ir {
Identifier::Identifier([[maybe_unused]] Tag const tag, Identifier const &other, ArenaAllocator *const allocator)
    : AnnotatedExpression(static_cast<AnnotatedExpression const &>(other), allocator), decorators_(allocator->Adapter())
{
    name_ = other.name_;
    flags_ = other.flags_;

    for (auto *decorator : other.decorators_) {
        decorators_.emplace_back(decorator->Clone(allocator, this));
    }
}

Identifier *Identifier::Clone(ArenaAllocator *const allocator, AstNode *const parent)
{
    if (auto *const clone = allocator->New<Identifier>(Tag {}, *this, allocator); clone != nullptr) {
        clone->SetTsType(TsType());
        if (parent != nullptr) {
            clone->SetParent(parent);
        }

        clone->SetRange(Range());

        return clone;
    }
    throw Error(ErrorType::GENERIC, "", CLONE_ALLOCATION_ERROR);
}

void Identifier::TransformChildren(const NodeTransformer &cb, std::string_view const transformationName)
{
    if (auto *typeAnnotation = TypeAnnotation(); typeAnnotation != nullptr) {
        if (auto *transformedNode = cb(typeAnnotation); typeAnnotation != transformedNode) {
            typeAnnotation->SetTransformedNode(transformationName, transformedNode);
            SetTsTypeAnnotation(static_cast<TypeNode *>(transformedNode));
        }
    }

    for (auto *&it : decorators_) {
        if (auto *transformedNode = cb(it); it != transformedNode) {
            it->SetTransformedNode(transformationName, transformedNode);
            it = transformedNode->AsDecorator();
        }
    }
}

void Identifier::Iterate(const NodeTraverser &cb) const
{
    if (TypeAnnotation() != nullptr) {
        cb(TypeAnnotation());
    }

    for (auto *it : decorators_) {
        cb(it);
    }
}

ValidationInfo Identifier::ValidateExpression()
{
    if ((flags_ & IdentifierFlags::OPTIONAL) != 0U) {
        return {"Unexpected token '?'.", Start()};
    }

    if (TypeAnnotation() != nullptr) {
        return {"Unexpected token.", TypeAnnotation()->Start()};
    }

    ValidationInfo info;
    return info;
}

void Identifier::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", IsPrivateIdent() ? "PrivateIdentifier" : "Identifier"},
                 {"name", name_},
                 {"typeAnnotation", AstDumper::Optional(TypeAnnotation())},
                 {"optional", AstDumper::Optional(IsOptional())},
                 {"decorators", decorators_}});
}

void Identifier::Dump(ir::SrcDumper *dumper) const
{
    if (IsPrivateIdent()) {
        dumper->Add("private ");
    }
    dumper->Add(std::string(name_));
    if (IsOptional()) {
        dumper->Add("?");
    }
}

void Identifier::Compile(compiler::PandaGen *pg) const
{
    pg->GetAstCompiler()->Compile(this);
}

void Identifier::Compile(compiler::ETSGen *etsg) const
{
    etsg->GetAstCompiler()->Compile(this);
}

checker::Type *Identifier::Check(checker::TSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

checker::Type *Identifier::Check(checker::ETSChecker *checker)
{
    return checker->GetAnalyzer()->Check(this);
}

bool Identifier::CommonDeclarationCheck(const ir::Identifier *id) const
{
    if (id->Parent() != nullptr) {
        auto parent = id->Parent();
        // We need two Parts because check is too long to fit in one function
        if (CheckDeclarationsPart1(parent) || CheckDeclarationsPart2(parent)) {
            return true;
        }

        // Parameters in methods are not references
        // Example:
        // function foo(a: int)
        if (parent->IsETSParameterExpression()) {
            return true;
        }

        // Class Properties are not references
        if (parent->IsClassProperty()) {
            return true;
        }

        // Identifier in catch is not a reference
        // Example:
        //
        // _try_{
        //   _throw_new_Exception();}_catch_(e)_{}
        if (parent->IsCatchClause()) {
            return true;
        }

        // Type Parameter is not a reference
        // Example:
        // interface X <K> {}
        if (parent->IsTSTypeParameter()) {
            return true;
        }

        // Rest Parameter is not a reference
        // Example:
        // class A {
        //    constructor(... items :Object[]){}
        // }
        if (parent->IsRestElement()) {
            return true;
        }

        // Script function identifiers are not references
        // Example:
        // public static foo()
        if (parent->IsScriptFunction()) {
            return true;
        }

        // New methods are not references
        if (parent->IsMethodDefinition()) {
            return true;
        }

        // New classes are not references
        if (parent->IsClassDefinition()) {
            return true;
        }
    }

    // if (CheckNameRelated(id)) {
    //     return true;
    // }

    return false;
}


bool Identifier::CheckDeclarationsPart1(const ir::AstNode *parentNode) const
{
    // All declarations are not references
    if (parentNode->IsVariableDeclaration()) {
        return true;
    }

    if (parentNode->IsClassDeclaration()) {
        return true;
    }

    if (parentNode->IsETSPackageDeclaration()) {
        return true;
    }

    // if (parentNode->IsVariableDeclarator()) {
    //     return true;
    // }

    if (parentNode->IsFunctionDeclaration()) {
        return true;
    }

    if (parentNode->IsImportDeclaration()) {
        return true;
    }

    if (parentNode->IsETSImportDeclaration()) {
        return true;
    }

    if (parentNode->IsETSStructDeclaration()) {
        return true;
    }

    if (parentNode->IsExportAllDeclaration()) {
        return true;
    }

    if (parentNode->IsExportDefaultDeclaration()) {
        return true;
    }

    return false;
}

bool Identifier::CheckDeclarationsPart2(const ir::AstNode *parentNode) const
{
    // All declarations are not references
    if (parentNode->IsExportNamedDeclaration()) {
        return true;
    }

    if (parentNode->IsTSEnumDeclaration()) {
        return true;
    }

    if (parentNode->IsTSInterfaceDeclaration()) {
        return true;
    }

    if (parentNode->IsTSModuleDeclaration()) {
        return true;
    }

    if (parentNode->IsTSSignatureDeclaration()) {
        return true;
    }

    if (parentNode->IsETSReExportDeclaration()) {
        return true;
    }

    if (parentNode->IsTSImportEqualsDeclaration()) {
        return true;
    }

    if (parentNode->IsTSTypeAliasDeclaration()) {
        return true;
    }

    if (parentNode->IsTSTypeParameterDeclaration()) {
        return true;
    }

    if (parentNode->IsDeclare()) {
        return true;
    }

    if (parentNode->Parent() != nullptr) {
        auto parent = parentNode->Parent();
        if (parent->IsTSEnumDeclaration()) {
            return true;
        }
    }

    return false;
}

bool Identifier::CheckNameRelated(const ir::Identifier *id) const
{
    auto name = id->Name();
    // NOTE: skip $dynmodule and JSRuntime identifiers that are not references
    if (name == "$dynmodule" || name == "JSRuntime") {
        return true;
    }

    // NOTE: skip <property> identifiers that are not references
    if (name.Utf8().find("<property>") == 0) {
        return true;
    }

    // NOTE: should be deleted as soon as issue with Identifiers with no name is fixed
    if (name.Empty()) {
        return true;
    }

    // NOTE: some lambda identifiers are not references
    if (name.Utf8().find("lambda$invoke") == 0) {
        return true;
    }
    // NOTE: currently some generated gensym identifiers are not references
    if (name.Utf8().find("gensym") == 0) {
        return true;
    }

    // NOTE: currently some generated field identifiers are not references
    if (name.Utf8().find("field") == 0) {
        return true;
    }

    return false;
}

}  // namespace ark::es2panda::ir
