/*
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

#include "enumLowering.h"
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

// const char* EnumConstantClassName = "EnumConst";
const char *ENUM_INT_BASE_CLASS_NAME = "EnumIntType";
const char *ENUM_STR_BASE_CLASS_NAME = "EnumStrType";
const char *ENUM_CONSTANT_CREATE_METHOD_NAME = "create";
const char *ENUM_VALUE_OF_LIB_FUNCTION_NAME = "enumValueOf";
const char *ENUM_CONSTANT_ARRAY_NAME = "arr";

auto CreateIdentifierRef(util::StringView const name, checker::ETSChecker *checker)
{
    auto *ret = checker->Allocator()->New<ir::Identifier>(name, checker->Allocator());
    ret->SetReference();
    return ret;
}

auto CreateETSTypeReference(util::StringView const name, checker::ETSChecker *checker)
{
    auto *name_ident = CreateIdentifierRef(name, checker);
    auto *reference_part = checker->Allocator()->New<ir::ETSTypeReferencePart>(name_ident);
    return checker->Allocator()->New<ir::ETSTypeReference>(reference_part);
}

auto CreateETSParameterExpression(util::StringView const par_name, util::StringView const type_name,
                                  checker::ETSChecker *checker)
{
    auto *type_annot = CreateETSTypeReference(type_name, checker);
    auto *name = checker->Allocator()->New<ir::Identifier>(par_name, type_annot, checker->Allocator());
    return checker->Allocator()->New<ir::ETSParameterExpression>(name, nullptr);
}

ir::MethodDefinition *CreateMethodValueOf(const util::StringView &enum_name,
                                          ir::TSTypeParameterInstantiation *tp_par_inst, checker::ETSChecker *checker)
{
    auto *key = checker->AllocNode<ir::Identifier>("valueOf", checker->Allocator());

    ArenaVector<ir::Expression *> params(checker->Allocator()->Adapter());
    params.push_back(CreateETSParameterExpression("name", "string", checker));

    auto *ret_type = CreateETSTypeReference(enum_name, checker);

    ArenaVector<ir::Statement *> statements(checker->Allocator()->Adapter());
    auto *callee = CreateIdentifierRef(ENUM_VALUE_OF_LIB_FUNCTION_NAME, checker);
    ArenaVector<ir::Expression *> args(checker->Allocator()->Adapter());
    auto *arg0_obj = CreateIdentifierRef(enum_name, checker);
    auto *arg0_prop = CreateIdentifierRef(ENUM_CONSTANT_ARRAY_NAME, checker);
    args.push_back(checker->AllocNode<ir::MemberExpression>(arg0_obj, arg0_prop,
                                                            ir::MemberExpressionKind::PROPERTY_ACCESS, false, false));
    auto *arg1 = CreateIdentifierRef("name", checker);
    args.push_back(arg1);
    auto *express = checker->AllocNode<ir::CallExpression>(callee, std::move(args), tp_par_inst, false);
    auto *argument = checker->AllocNode<ir::TSAsExpression>(express, ret_type, false);
    statements.push_back(checker->AllocNode<ir::ReturnStatement>(argument));
    auto *body = checker->AllocNode<ir::BlockStatement>(checker->Allocator(), std::move(statements));

    auto *func = checker->AllocNode<ir::ScriptFunction>(
        ir::FunctionSignature(nullptr, std::move(params), ret_type), body,
        ir::ScriptFunctionFlags::HAS_RETURN | ir::ScriptFunctionFlags::THROWS,
        ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC, false, Language(Language::Id::ETS));

    func->SetIdent(key);

    auto *value = checker->AllocNode<ir::FunctionExpression>(func);

    auto flags = ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC;

    auto *method = checker->AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD, key, value, flags,
                                                            checker->Allocator(), false);

    return method;
}

ir::MethodDefinition *CreateMethodCreate(const util::StringView &enum_name, bool is_int_enum,
                                         checker::ETSChecker *checker)
{
    auto *key = checker->AllocNode<ir::Identifier>("create", checker->Allocator());

    ArenaVector<ir::Expression *> params(checker->Allocator()->Adapter());
    params.push_back(CreateETSParameterExpression("typ", "string", checker));
    params.push_back(CreateETSParameterExpression("name", "string", checker));
    params.push_back(CreateETSParameterExpression("val", is_int_enum ? "Int" : "string", checker));
    params.push_back(CreateETSParameterExpression("idx", "Int", checker));

    auto id = checker->AllocNode<ir::Identifier>("ret", checker->Allocator());
    auto init = checker->AllocNode<ir::ETSNewClassInstanceExpression>(
        CreateETSTypeReference(enum_name, checker), ArenaVector<ir::Expression *>(checker->Allocator()->Adapter()),
        nullptr);
    auto declarator = checker->AllocNode<ir::VariableDeclarator>(ir::VariableDeclaratorFlag::LET, id, init);
    ArenaVector<ir::VariableDeclarator *> declarators(checker->Allocator()->Adapter());
    declarators.push_back(declarator);

    auto var_kind = ir::VariableDeclaration::VariableDeclarationKind::LET;
    auto *var_decl =
        checker->AllocNode<ir::VariableDeclaration>(var_kind, checker->Allocator(), std::move(declarators), false);

    ArenaVector<ir::Statement *> statements(checker->Allocator()->Adapter());
    statements.push_back(var_decl);

    auto *callee_obj = CreateIdentifierRef("ret", checker);
    auto *callee_prop = CreateIdentifierRef("init", checker);
    auto *callee = checker->AllocNode<ir::MemberExpression>(callee_obj, callee_prop,
                                                            ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    ArenaVector<ir::Expression *> args(checker->Allocator()->Adapter());
    args.push_back(CreateIdentifierRef("typ", checker));
    args.push_back(CreateIdentifierRef("name", checker));
    args.push_back(CreateIdentifierRef("val", checker));
    args.push_back(CreateIdentifierRef("idx", checker));
    auto *expression = checker->AllocNode<ir::CallExpression>(callee, std::move(args), nullptr, false);
    statements.push_back(checker->AllocNode<ir::ExpressionStatement>(expression));

    statements.push_back(checker->AllocNode<ir::ReturnStatement>(callee_obj));

    auto *body = checker->AllocNode<ir::BlockStatement>(checker->Allocator(), std::move(statements));

    auto *func = checker->AllocNode<ir::ScriptFunction>(
        ir::FunctionSignature(nullptr, std::move(params), CreateETSTypeReference(enum_name, checker)), body,
        ir::ScriptFunctionFlags::HAS_RETURN, ir::ModifierFlags::STATIC | ir::ModifierFlags::PRIVATE, false,
        Language(Language::Id::ETS));

    func->SetIdent(key);
    auto *value = checker->AllocNode<ir::FunctionExpression>(func);

    auto flags = ir::ModifierFlags::STATIC | ir::ModifierFlags::PRIVATE;

    auto *method = checker->AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD, key, value, flags,
                                                            checker->Allocator(), false);

    return method;
}

ir::MethodDefinition *CreateMethodValues(const util::StringView &enum_name, checker::ETSChecker *checker)
{
    auto *key = checker->AllocNode<ir::Identifier>("values", checker->Allocator());

    ArenaVector<ir::Expression *> params(checker->Allocator()->Adapter());

    ArenaVector<ir::Statement *> statements(checker->Allocator()->Adapter());
    auto *arg_obj = CreateIdentifierRef(enum_name, checker);
    auto *arg_prop = CreateIdentifierRef(ENUM_CONSTANT_ARRAY_NAME, checker);
    auto *argument = checker->AllocNode<ir::MemberExpression>(arg_obj, arg_prop,
                                                              ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    statements.push_back(checker->AllocNode<ir::ReturnStatement>(argument));
    auto *body = checker->AllocNode<ir::BlockStatement>(checker->Allocator(), std::move(statements));

    auto *func = checker->AllocNode<ir::ScriptFunction>(
        ir::FunctionSignature(nullptr, std::move(params), nullptr), body, ir::ScriptFunctionFlags::HAS_RETURN,
        ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC, false, Language(Language::Id::ETS));

    func->SetIdent(key);
    auto *value = checker->AllocNode<ir::FunctionExpression>(func);

    auto flags = ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC;

    auto *method = checker->AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD, key, value, flags,
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

    auto *method = checker->AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::CONSTRUCTOR, key, value,
                                                            ir::ModifierFlags::NONE, checker->Allocator(), false);

    return method;
}

ir::MemberExpression *CreateEnumConstantArrayElement(const util::StringView &enum_name, const ir::TSEnumMember *member,
                                                     checker::ETSChecker *checker)
{
    auto *object = CreateIdentifierRef(enum_name, checker);
    auto *prop = CreateIdentifierRef(member->Name(), checker);
    auto *elem =
        checker->AllocNode<ir::MemberExpression>(object, prop, ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);

    return elem;
}

ir::ClassProperty *CreateEnumConstantArray(const util::StringView &enum_name, ArenaVector<ir::Expression *> &&elements,
                                           checker::ETSChecker *checker)
{
    auto *key = checker->AllocNode<ir::Identifier>(ENUM_CONSTANT_ARRAY_NAME, checker->Allocator());
    auto *value = checker->AllocNode<ir::ArrayExpression>(std::move(elements), checker->Allocator());

    auto prop_modif = ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC;

    auto *el_type = CreateETSTypeReference(enum_name, checker);
    auto *type_annotation = checker->AllocNode<ir::TSArrayType>(el_type);

    auto *class_prop =
        checker->AllocNode<ir::ClassProperty>(key, value, type_annotation, prop_modif, checker->Allocator(), false);

    return class_prop;
}

ir::ClassProperty *CreateEnumConstantClassProperty(const util::StringView &enum_name, const ir::TSEnumMember *member,
                                                   int idx, checker::ETSChecker *checker)
{
    auto *key = checker->AllocNode<ir::Identifier>(member->Name(), checker->Allocator());

    auto *callee_obj = CreateIdentifierRef(enum_name, checker);
    auto *callee_prop = CreateIdentifierRef(ENUM_CONSTANT_CREATE_METHOD_NAME, checker);
    auto *callee = checker->AllocNode<ir::MemberExpression>(callee_obj, callee_prop,
                                                            ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);

    ArenaVector<ir::Expression *> args(checker->Allocator()->Adapter());
    util::UString enum_desc(checker::EnumDescription(enum_name), checker->Allocator());
    args.push_back(checker->AllocNode<ir::StringLiteral>(enum_desc.View()));
    util::UString enum_const_name(member->Name(), checker->Allocator());
    args.push_back(checker->AllocNode<ir::StringLiteral>(enum_const_name.View()));
    if (member->Init()->IsNumberLiteral()) {
        args.push_back(checker->AllocNode<ir::NumberLiteral>(member->Init()->AsNumberLiteral()->Number()));
    } else if (member->Init()->IsStringLiteral()) {
        args.push_back(checker->AllocNode<ir::StringLiteral>(member->Init()->AsStringLiteral()->Str()));
    } else {
        UNREACHABLE();
    }
    args.push_back(checker->AllocNode<ir::NumberLiteral>(lexer::Number(idx)));

    auto *value = checker->AllocNode<ir::CallExpression>(callee, std::move(args), nullptr, false);

    auto prop_modif =
        ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC | ir::ModifierFlags::READONLY | ir::ModifierFlags::CONST;
    auto *type_annot = CreateETSTypeReference(enum_name, checker);

    auto *class_prop =
        checker->AllocNode<ir::ClassProperty>(key, value, type_annot, prop_modif, checker->Allocator(), false);

    return class_prop;
}

void CreateCCtor(ArenaVector<ir::AstNode *> &properties, const lexer::SourcePosition &loc, checker::ETSChecker *checker)
{
    ArenaVector<ir::Expression *> params(checker->Allocator()->Adapter());

    auto *id = checker->AllocNode<ir::Identifier>(compiler::Signatures::CCTOR, checker->Allocator());

    ArenaVector<ir::Statement *> statements(checker->Allocator()->Adapter());

    auto *body = checker->AllocNode<ir::BlockStatement>(checker->Allocator(), std::move(statements));
    auto *func =
        checker->AllocNode<ir::ScriptFunction>(ir::FunctionSignature(nullptr, std::move(params), nullptr), body,
                                               ir::ScriptFunctionFlags::STATIC_BLOCK | ir::ScriptFunctionFlags::HIDDEN,
                                               ir::ModifierFlags::STATIC, false, Language(Language::Id::ETS));
    func->SetIdent(id);

    auto *func_expr = checker->AllocNode<ir::FunctionExpression>(func);
    auto *static_block = checker->AllocNode<ir::ClassStaticBlock>(func_expr, checker->Allocator());
    static_block->AddModifier(ir::ModifierFlags::STATIC);
    static_block->SetRange({loc, loc});
    properties.push_back(static_block);
}

ir::TSTypeParameterInstantiation *CreateTypeParameterInstantiation(bool is_int_enum, checker::ETSChecker *checker)
{
    ArenaVector<ir::TypeNode *> params(checker->Allocator()->Adapter());
    auto *par_part_name = CreateIdentifierRef("", checker);
    if (is_int_enum) {
        par_part_name->SetName("Int");
    } else {
        par_part_name->SetName("string");
    }
    auto *par_part = checker->AllocNode<ir::ETSTypeReferencePart>(par_part_name);
    params.push_back(checker->AllocNode<ir::ETSTypeReference>(par_part));

    return checker->AllocNode<ir::TSTypeParameterInstantiation>(std::move(params));
}

ir::AstNode *CreateEnumClassFromEnumDeclaration(ir::TSEnumDeclaration *enum_decl, checker::ETSChecker *checker,
                                                varbinder::ETSBinder * /*varbinder*/, parser::Program * /*program*/)
{
    auto &enum_name = enum_decl->Key()->Name();
    auto *ident = checker->AllocNode<ir::Identifier>(enum_name, checker->Allocator());

    ArenaVector<ir::AstNode *> body(checker->Allocator()->Adapter());
    ArenaVector<ir::Expression *> array_elements(checker->Allocator()->Adapter());
    int idx = 0;
    for (auto &member : enum_decl->Members()) {
        body.push_back(CreateEnumConstantClassProperty(enum_name, member->AsTSEnumMember(), idx++, checker));
        array_elements.push_back(CreateEnumConstantArrayElement(enum_name, member->AsTSEnumMember(), checker));
    }

    bool is_int_enum = false;
    auto member0 = enum_decl->Members()[0]->AsTSEnumMember();
    if (member0->Init()->IsNumberLiteral()) {
        is_int_enum = true;
    } else if (!member0->Init()->IsStringLiteral()) {
        UNREACHABLE();
    }
    auto *tp_par_inst = CreateTypeParameterInstantiation(is_int_enum, checker);

    body.push_back(CreateEnumConstantArray(enum_name, std::move(array_elements), checker));

    body.push_back(CreateMethodCreate(enum_name, is_int_enum, checker));
    body.push_back(CreateMethodValues(enum_name, checker));
    body.push_back(CreateMethodValueOf(enum_name, tp_par_inst, checker));
    body.push_back(CreateConstructor(checker));

    CreateCCtor(body, enum_decl->Start(), checker);

    auto def_modif = ir::ClassDefinitionModifiers::ID_REQUIRED | ir::ClassDefinitionModifiers::HAS_SUPER |
                     ir::ClassDefinitionModifiers::CLASS_DECL | ir::ClassDefinitionModifiers::DECLARATION;
    auto *class_def = checker->AllocNode<ir::ClassDefinition>(checker->Allocator(), ident, std::move(body), def_modif,
                                                              Language(Language::Id::ETS));
    if (is_int_enum) {
        class_def->SetSuper(CreateETSTypeReference(ENUM_INT_BASE_CLASS_NAME, checker));
    } else {
        class_def->SetSuper(CreateETSTypeReference(ENUM_STR_BASE_CLASS_NAME, checker));
    }

    auto *class_decl = checker->AllocNode<ir::ClassDeclaration>(class_def, checker->Allocator());
    class_decl->SetRange({enum_decl->Start(), enum_decl->End()});

    class_decl->SetParent(enum_decl->Parent());

    return class_decl;
}

bool EnumLowering::Perform(public_lib::Context *ctx, parser::Program *program)
{
    if (program->Extension() != ScriptExtension::ETS) {
        return true;
    }

    checker::ETSChecker *checker = ctx->checker->AsETSChecker();

    for (auto &[_, ext_programs] : program->ExternalSources()) {
        (void)_;
        for (auto *ext_prog : ext_programs) {
            Perform(ctx, ext_prog);
        }
    }

    program->Ast()->TransformChildrenRecursively([checker, ctx, program](ir::AstNode *ast) -> ir::AstNode * {
        if (ast->IsTSEnumDeclaration()) {
            return CreateEnumClassFromEnumDeclaration(ast->AsTSEnumDeclaration(), checker,
                                                      ctx->compilerContext->VarBinder()->AsETSBinder(), program);
        }
        return ast;
    });

    return true;
}

}  // namespace ark::es2panda::compiler
