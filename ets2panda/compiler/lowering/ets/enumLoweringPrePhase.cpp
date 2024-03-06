/*
 * Copyright (c) 2021 - 2024 Huawei Device Co., Ltd.
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

#include "enumLoweringPrePhase.h"
#include "compiler/lowering/scopesInit/scopesInitPhase.h"

#include "varbinder/variableFlags.h"
#include "varbinder/ETSBinder.h"
#include "checker/ETSchecker.h"
#include "compiler/core/compilerContext.h"
#include "compiler/lowering/util.h"
#include "ir/statements/classDeclaration.h"
#include "ir/base/classDefinition.h"
#include "ir/base/classProperty.h"
#include "ir/astNode.h"
#include "ir/expression.h"
#include "util/ustring.h"

#include "ir/ts/tsEnumDeclaration.h"

namespace ark::es2panda::compiler {

const char *g_enumIntBaseClassName = "EnumIntType";
const char *g_enumStrBaseClassName = "EnumStrType";
const char *g_enumConstantCreateMethodName = "create";
const char *g_enumValueOfLibFunctionName = "enumValueOf";
const char *g_enumConstantArrayName = "arr";

ir::Identifier *CreateIdentifierRef(util::StringView const name, checker::ETSChecker *checker)
{
    auto *ret = checker->AllocNode<ir::Identifier>(name, checker->Allocator());
    ret->SetReference();
    return ret;
}

auto CreateETSTypeReference(util::StringView const name, checker::ETSChecker *checker)
{
    auto *nameIdent = CreateIdentifierRef(name, checker);
    auto *referencePart = checker->AllocNode<ir::ETSTypeReferencePart>(nameIdent);
    return checker->AllocNode<ir::ETSTypeReference>(referencePart);
}

auto CreateETSParameterExpression(util::StringView const parName, util::StringView const typeName,
                                  checker::ETSChecker *checker)
{
    auto *typeAnnot = CreateETSTypeReference(typeName, checker);
    auto *name = checker->AllocNode<ir::Identifier>(parName, typeAnnot, checker->Allocator());
    return checker->AllocNode<ir::ETSParameterExpression>(name, nullptr);
}

ir::MethodDefinition *CreateMethodValueOf(const util::StringView &enumName, ir::TSTypeParameterInstantiation *tpParInst,
                                          checker::ETSChecker *checker)
{
    ArenaVector<ir::Expression *> params(checker->Allocator()->Adapter());
    params.push_back(CreateETSParameterExpression("name", "string", checker));

    auto *retType = CreateETSTypeReference(enumName, checker);

    ArenaVector<ir::Statement *> statements(checker->Allocator()->Adapter());
    ArenaVector<ir::Expression *> args(checker->Allocator()->Adapter());
    auto *arg0Obj = CreateIdentifierRef(enumName, checker);
    auto *arg0Prop = CreateIdentifierRef(g_enumConstantArrayName, checker);
    args.push_back(checker->AllocNode<ir::MemberExpression>(arg0Obj, arg0Prop,
                                                            ir::MemberExpressionKind::PROPERTY_ACCESS, false, false));
    auto *arg1 = CreateIdentifierRef("name", checker);
    args.push_back(arg1);

    auto *callee = CreateIdentifierRef(g_enumValueOfLibFunctionName, checker);
    auto *express = checker->AllocNode<ir::CallExpression>(callee, std::move(args), tpParInst, false);
    auto *argument = checker->AllocNode<ir::TSAsExpression>(express, retType, false);
    statements.push_back(checker->AllocNode<ir::ReturnStatement>(argument));
    auto *body = checker->AllocNode<ir::BlockStatement>(checker->Allocator(), std::move(statements));

    auto *func = checker->AllocNode<ir::ScriptFunction>(
        ir::FunctionSignature(nullptr, std::move(params), retType->Clone(checker->Allocator(), nullptr)), body,
        ir::ScriptFunctionFlags::HAS_RETURN | ir::ScriptFunctionFlags::THROWS, false, Language(Language::Id::ETS));
    func->AddModifier(ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC);
    auto *key = checker->AllocNode<ir::Identifier>("valueOf", checker->Allocator());
    func->SetIdent(key);

    auto *value = checker->AllocNode<ir::FunctionExpression>(func);

    auto flags = ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC;

    auto *method = checker->AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD,
                                                            key->Clone(checker->Allocator(), nullptr), value, flags,
                                                            checker->Allocator(), false);

    return method;
}

ir::MethodDefinition *CreateMethodCreate(const util::StringView &enumName, bool isIntEnum, checker::ETSChecker *checker)
{
    auto id = checker->AllocNode<ir::Identifier>("ret", checker->Allocator());
    auto init = checker->AllocNode<ir::ETSNewClassInstanceExpression>(
        CreateETSTypeReference(enumName, checker), ArenaVector<ir::Expression *>(checker->Allocator()->Adapter()),
        nullptr);
    auto declarator = checker->AllocNode<ir::VariableDeclarator>(ir::VariableDeclaratorFlag::LET, id, init);
    ArenaVector<ir::VariableDeclarator *> declarators(checker->Allocator()->Adapter());
    declarators.push_back(declarator);

    auto varKind = ir::VariableDeclaration::VariableDeclarationKind::LET;
    auto *varDecl =
        checker->AllocNode<ir::VariableDeclaration>(varKind, checker->Allocator(), std::move(declarators), false);

    ArenaVector<ir::Statement *> statements(checker->Allocator()->Adapter());
    statements.push_back(varDecl);

    auto *calleeObj = CreateIdentifierRef("ret", checker);
    auto *calleeProp = CreateIdentifierRef("init", checker);
    auto *callee = checker->AllocNode<ir::MemberExpression>(calleeObj, calleeProp,
                                                            ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    ArenaVector<ir::Expression *> args(checker->Allocator()->Adapter());
    args.push_back(CreateIdentifierRef("typ", checker));
    args.push_back(CreateIdentifierRef("name", checker));
    args.push_back(CreateIdentifierRef("val", checker));
    args.push_back(CreateIdentifierRef("idx", checker));
    auto *expression = checker->AllocNode<ir::CallExpression>(callee, std::move(args), nullptr, false);
    statements.push_back(checker->AllocNode<ir::ExpressionStatement>(expression));

    statements.push_back(checker->AllocNode<ir::ReturnStatement>(CreateIdentifierRef("ret", checker)));

    auto *body = checker->AllocNode<ir::BlockStatement>(checker->Allocator(), std::move(statements));

    ArenaVector<ir::Expression *> params(checker->Allocator()->Adapter());
    params.push_back(CreateETSParameterExpression("typ", "string", checker));
    params.push_back(CreateETSParameterExpression("name", "string", checker));
    params.push_back(CreateETSParameterExpression("val", isIntEnum ? "Int" : "String", checker));
    params.push_back(CreateETSParameterExpression("idx", "Int", checker));

    auto *func = checker->AllocNode<ir::ScriptFunction>(
        ir::FunctionSignature(nullptr, std::move(params), CreateETSTypeReference(enumName, checker)), body,
        ir::ScriptFunctionFlags::HAS_RETURN, false, Language(Language::Id::ETS));
    func->AddModifier(ir::ModifierFlags::STATIC | ir::ModifierFlags::PRIVATE);

    auto *key = CreateIdentifierRef(g_enumConstantCreateMethodName, checker);
    func->SetIdent(key);
    auto *value = checker->AllocNode<ir::FunctionExpression>(func);

    auto flags = ir::ModifierFlags::STATIC | ir::ModifierFlags::PRIVATE;

    auto *method = checker->AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD,
                                                            key->Clone(checker->Allocator(), nullptr)->AsIdentifier(),
                                                            value, flags, checker->Allocator(), false);

    return method;
}

ir::MethodDefinition *CreateMethodValues(const util::StringView &enumName, checker::ETSChecker *checker)
{
    ArenaVector<ir::Expression *> params(checker->Allocator()->Adapter());

    ArenaVector<ir::Statement *> statements(checker->Allocator()->Adapter());
    auto *argObj = CreateIdentifierRef(enumName, checker);
    auto *argProp = CreateIdentifierRef(g_enumConstantArrayName, checker);
    auto *argument = checker->AllocNode<ir::MemberExpression>(argObj, argProp,
                                                              ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    statements.push_back(checker->AllocNode<ir::ReturnStatement>(argument));
    auto *body = checker->AllocNode<ir::BlockStatement>(checker->Allocator(), std::move(statements));

    auto *func =
        checker->AllocNode<ir::ScriptFunction>(ir::FunctionSignature(nullptr, std::move(params), nullptr), body,
                                               ir::ScriptFunctionFlags::HAS_RETURN, false, Language(Language::Id::ETS));

    func->AddModifier(ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC);
    auto *key = checker->AllocNode<ir::Identifier>("values", checker->Allocator());
    func->SetIdent(key);
    auto *value = checker->AllocNode<ir::FunctionExpression>(func);

    auto flags = ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC;

    auto *method = checker->AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD,
                                                            key->Clone(checker->Allocator(), nullptr), value, flags,
                                                            checker->Allocator(), false);
    return method;
}

ir::MethodDefinition *CreateConstructor(checker::ETSChecker *checker)
{
    auto *key = checker->AllocNode<ir::Identifier>("constructor", checker->Allocator());

    ArenaVector<ir::Expression *> params(checker->Allocator()->Adapter());

    ArenaVector<ir::Statement *> statements(checker->Allocator()->Adapter());
    auto *body = checker->AllocNode<ir::BlockStatement>(checker->Allocator(), std::move(statements));

    auto *func = checker->AllocNode<ir::ScriptFunction>(
        ir::FunctionSignature(nullptr, std::move(params), nullptr), body,
        ir::ScriptFunctionFlags::CONSTRUCTOR | ir::ScriptFunctionFlags::IMPLICIT_SUPER_CALL_NEEDED, false,
        Language(Language::Id::ETS));

    func->SetIdent(key);
    auto *value = checker->AllocNode<ir::FunctionExpression>(func);

    auto *method = checker->AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::CONSTRUCTOR,
                                                            key->Clone(checker->Allocator(), nullptr), value,
                                                            ir::ModifierFlags::NONE, checker->Allocator(), false);

    return method;
}

ir::MemberExpression *CreateEnumConstantArrayElement(const util::StringView &enumName, const ir::TSEnumMember *member,
                                                     checker::ETSChecker *checker)
{
    auto *object = CreateIdentifierRef(enumName, checker);
    auto *prop = CreateIdentifierRef(member->Name(), checker);
    auto *elem =
        checker->AllocNode<ir::MemberExpression>(object, prop, ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);

    return elem;
}

ir::ClassProperty *CreateEnumConstantArray(const util::StringView &enumName, ArenaVector<ir::Expression *> &&elements,
                                           checker::ETSChecker *checker)
{
    auto *key = checker->AllocNode<ir::Identifier>(g_enumConstantArrayName, checker->Allocator());
    auto *value = checker->AllocNode<ir::ArrayExpression>(std::move(elements), checker->Allocator());

    auto propModif = ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC;

    auto *elType = CreateETSTypeReference(enumName, checker);
    auto *typeAnnotation = checker->AllocNode<ir::TSArrayType>(elType);

    auto *classProp =
        checker->AllocNode<ir::ClassProperty>(key, value, typeAnnotation, propModif, checker->Allocator(), false);

    return classProp;
}

ir::ClassProperty *CreateEnumConstantClassProperty(const util::StringView &enumName, const ir::TSEnumMember *member,
                                                   int idx, checker::ETSChecker *checker)
{
    ArenaVector<ir::Expression *> args(checker->Allocator()->Adapter());

    util::UString enumDesc(checker::EnumDescription(enumName), checker->Allocator());
    args.push_back(checker->AllocNode<ir::StringLiteral>(enumDesc.View()));

    util::UString enumConstName(member->Name(), checker->Allocator());
    args.push_back(checker->AllocNode<ir::StringLiteral>(enumConstName.View()));

    if (member->Init()->IsNumberLiteral()) {
        args.push_back(checker->AllocNode<ir::NumberLiteral>(member->Init()->AsNumberLiteral()->Number()));
    } else if (member->Init()->IsStringLiteral()) {
        args.push_back(checker->AllocNode<ir::StringLiteral>(member->Init()->AsStringLiteral()->Str()));
    } else {
        UNREACHABLE();
    }
    args.push_back(checker->AllocNode<ir::NumberLiteral>(lexer::Number(idx)));

    auto *calleeObj = CreateIdentifierRef(enumName, checker);
    auto *calleeProp = CreateIdentifierRef(g_enumConstantCreateMethodName, checker);
    auto *callee = checker->AllocNode<ir::MemberExpression>(calleeObj, calleeProp,
                                                            ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    auto *value = checker->AllocNode<ir::CallExpression>(callee, std::move(args), nullptr, false);

    auto propModif =
        ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC | ir::ModifierFlags::READONLY | ir::ModifierFlags::CONST;
    auto *typeAnnot = CreateETSTypeReference(enumName, checker);

    auto *key = checker->AllocNode<ir::Identifier>(member->Name(), checker->Allocator());
    auto *classProp =
        checker->AllocNode<ir::ClassProperty>(key, value, typeAnnot, propModif, checker->Allocator(), false);

    return classProp;
}

void CreateCCtor(ArenaVector<ir::AstNode *> &properties, const lexer::SourcePosition &loc, checker::ETSChecker *checker)
{
    ArenaVector<ir::Expression *> params(checker->Allocator()->Adapter());

    auto *id = checker->AllocNode<ir::Identifier>(compiler::Signatures::CCTOR, checker->Allocator());

    ArenaVector<ir::Statement *> statements(checker->Allocator()->Adapter());

    auto *body = checker->AllocNode<ir::BlockStatement>(checker->Allocator(), std::move(statements));
    auto *func = checker->AllocNode<ir::ScriptFunction>(
        ir::FunctionSignature(nullptr, std::move(params), nullptr), body,
        ir::ScriptFunctionFlags::STATIC_BLOCK | ir::ScriptFunctionFlags::HIDDEN, false, Language(Language::Id::ETS));
    func->AddModifier(ir::ModifierFlags::STATIC);
    func->SetIdent(id);

    auto *funcExpr = checker->AllocNode<ir::FunctionExpression>(func);
    auto *staticBlock = checker->AllocNode<ir::ClassStaticBlock>(funcExpr, checker->Allocator());
    staticBlock->AddModifier(ir::ModifierFlags::STATIC);
    staticBlock->SetRange({loc, loc});
    properties.push_back(staticBlock);
}

ir::TSTypeParameterInstantiation *CreateTypeParameterInstantiation(bool isIntEnum, checker::ETSChecker *checker)
{
    ArenaVector<ir::TypeNode *> params(checker->Allocator()->Adapter());
    auto *parPartName = CreateIdentifierRef("", checker);
    if (isIntEnum) {
        parPartName->SetName("Int");
    } else {
        parPartName->SetName("string");
    }
    auto *parPart = checker->AllocNode<ir::ETSTypeReferencePart>(parPartName);
    params.push_back(checker->AllocNode<ir::ETSTypeReference>(parPart));

    return checker->AllocNode<ir::TSTypeParameterInstantiation>(std::move(params));
}

ir::AstNode *CreateEnumClassFromEnumDeclaration(ir::TSEnumDeclaration *enumDecl, checker::ETSChecker *checker)
{
    auto &enumName = enumDecl->Key()->Name();
    auto *ident = checker->AllocNode<ir::Identifier>(enumName, checker->Allocator());

    ArenaVector<ir::AstNode *> body(checker->Allocator()->Adapter());
    ArenaVector<ir::Expression *> arrayElements(checker->Allocator()->Adapter());
    int idx = 0;
    for (auto &member : enumDecl->Members()) {
        body.push_back(CreateEnumConstantClassProperty(enumName, member->AsTSEnumMember(), idx++, checker));
        arrayElements.push_back(CreateEnumConstantArrayElement(enumName, member->AsTSEnumMember(), checker));
    }

    bool isIntEnum = false;
    auto member0 = enumDecl->Members()[0]->AsTSEnumMember();
    if (member0->Init()->IsNumberLiteral()) {
        isIntEnum = true;
    } else if (!member0->Init()->IsStringLiteral()) {
        UNREACHABLE();
    }
    auto *tpParInst = CreateTypeParameterInstantiation(isIntEnum, checker);

    body.push_back(CreateEnumConstantArray(enumName, std::move(arrayElements), checker));

    body.push_back(CreateMethodCreate(enumName, isIntEnum, checker));
    body.push_back(CreateMethodValues(enumName, checker));
    body.push_back(CreateMethodValueOf(enumName, tpParInst, checker));
    body.push_back(CreateConstructor(checker));

    CreateCCtor(body, enumDecl->Start(), checker);

    auto defModif = ir::ClassDefinitionModifiers::ID_REQUIRED | ir::ClassDefinitionModifiers::HAS_SUPER |
                    ir::ClassDefinitionModifiers::CLASS_DECL | ir::ClassDefinitionModifiers::DECLARATION;
    auto *classDef = checker->AllocNode<ir::ClassDefinition>(checker->Allocator(), ident, std::move(body), defModif,
                                                             Language(Language::Id::ETS));
    if (isIntEnum) {
        classDef->SetSuper(CreateETSTypeReference(g_enumIntBaseClassName, checker));
    } else {
        classDef->SetSuper(CreateETSTypeReference(g_enumStrBaseClassName, checker));
    }

    auto *classDecl = checker->AllocNode<ir::ClassDeclaration>(classDef, checker->Allocator());
    classDecl->SetRange({enumDecl->Start(), enumDecl->End()});

    classDecl->SetParent(enumDecl->Parent());

    return classDecl;
}

bool EnumLoweringPrePhase::Perform(public_lib::Context *ctx, parser::Program *program)
{
    if (program->Extension() != ScriptExtension::ETS) {
        return true;
    }

    checker::ETSChecker *checker = ctx->checker->AsETSChecker();

    for (auto &[_, extPrograms] : program->ExternalSources()) {
        (void)_;
        for (auto *extProg : extPrograms) {
            Perform(ctx, extProg);
        }
    }

    program->Ast()->TransformChildrenRecursively([checker](ir::AstNode *ast) -> ir::AstNode * {
        if (ast->IsTSEnumDeclaration()) {
            return CreateEnumClassFromEnumDeclaration(ast->AsTSEnumDeclaration(), checker);
        }
        return ast;
    });

    return true;
}

}  // namespace ark::es2panda::compiler
