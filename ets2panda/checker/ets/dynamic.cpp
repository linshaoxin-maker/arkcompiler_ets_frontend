/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "checker/ETSchecker.h"

#include "varbinder/scope.h"
#include "varbinder/declaration.h"
#include "varbinder/varbinder.h"
#include "varbinder/ETSBinder.h"
#include "checker/types/ets/etsDynamicFunctionType.h"
#include "ir/base/classProperty.h"
#include "ir/base/classStaticBlock.h"
#include "ir/base/methodDefinition.h"
#include "ir/base/scriptFunction.h"
#include "ir/expressions/assignmentExpression.h"
#include "ir/expressions/callExpression.h"
#include "ir/expressions/functionExpression.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/thisExpression.h"
#include "ir/expressions/memberExpression.h"
#include "ir/ets/etsPrimitiveType.h"
#include "ir/ts/tsAsExpression.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/classDeclaration.h"
#include "ir/statements/expressionStatement.h"
#include "ir/statements/returnStatement.h"
#include "ir/statements/variableDeclaration.h"
#include "ir/statements/variableDeclarator.h"
#include "parser/program/program.h"
#include "util/helpers.h"
#include "util/language.h"
#include "generated/signatures.h"
#include "ir/ets/etsParameterExpression.h"

namespace panda::es2panda::checker {

ir::ETSParameterExpression *ETSChecker::AddParam(varbinder::FunctionParamScope *param_scope, util::StringView name,
                                                 checker::Type *type)
{
    auto param_ctx = varbinder::LexicalScope<varbinder::FunctionParamScope>::Enter(VarBinder(), param_scope, false);
    auto *param_ident = AllocNode<ir::Identifier>(name, Allocator());
    auto *param = AllocNode<ir::ETSParameterExpression>(param_ident, nullptr);
    auto *param_var = std::get<1>(VarBinder()->AddParamDecl(param));
    param_var->SetTsType(type);
    param->Ident()->SetVariable(param_var);
    param->Ident()->SetTsType(type);
    return param;
}

static bool IsByValueCall(varbinder::ETSBinder *varbinder, ir::Expression *callee)
{
    if (callee->IsMemberExpression()) {
        return !callee->AsMemberExpression()->ObjType()->IsETSDynamicType();
    }

    if (callee->IsETSTypeReference()) {
        return false;
    }

    auto *var = callee->AsIdentifier()->Variable();
    auto *data = varbinder->DynamicImportDataForVar(var);
    if (data != nullptr) {
        auto *specifier = data->specifier;
        if (specifier->IsImportSpecifier()) {
            return false;
        }
    }

    return true;
}

template <typename T>
ir::ScriptFunction *ETSChecker::CreateDynamicCallIntrinsic(ir::Expression *callee, const ArenaVector<T *> &arguments,
                                                           Language lang)
{
    auto *name = AllocNode<ir::Identifier>("invoke", Allocator());
    auto *param_scope = Allocator()->New<varbinder::FunctionParamScope>(Allocator(), nullptr);
    auto *scope = Allocator()->New<varbinder::FunctionScope>(Allocator(), param_scope);

    ArenaVector<ir::Expression *> params(Allocator()->Adapter());

    auto *info = CreateSignatureInfo();
    info->min_arg_count = arguments.size() + 2U;

    auto dynamic_type = GlobalBuiltinDynamicType(lang);

    auto *obj_param = AddParam(param_scope, "obj", dynamic_type);
    params.push_back(obj_param);
    info->params.push_back(obj_param->Ident()->Variable()->AsLocalVariable());

    ir::ETSParameterExpression *param2;
    if (!IsByValueCall(VarBinder()->AsETSBinder(), callee)) {
        param2 = AddParam(param_scope, "qname", GlobalETSStringLiteralType());
    } else {
        param2 = AddParam(param_scope, "this", dynamic_type);
    }

    params.push_back(param2);
    info->params.push_back(param2->Ident()->Variable()->AsLocalVariable());

    for (size_t i = 0; i < arguments.size(); i++) {
        util::UString param_name("p" + std::to_string(i), Allocator());
        Type *param_type =
            arguments[i]->TsType()->IsLambdaObject() ? GlobalBuiltinJSValueType() : arguments[i]->TsType();
        ir::ETSParameterExpression *param = AddParam(param_scope, param_name.View(), param_type);
        params.push_back(param);
        info->params.push_back(param->Ident()->Variable()->AsLocalVariable());
    }

    auto *func = AllocNode<ir::ScriptFunction>(scope, std::move(params), nullptr, nullptr, nullptr,
                                               ir::ScriptFunctionFlags::METHOD, ir::ModifierFlags::NONE, false,
                                               Language(Language::Id::ETS));

    scope->BindNode(func);
    param_scope->BindNode(func);
    scope->BindParamScope(param_scope);
    param_scope->BindFunctionScope(scope);

    func->SetIdent(name);

    auto *signature = CreateSignature(info, dynamic_type, func);
    signature->AddSignatureFlag(SignatureFlags::STATIC);

    func->SetSignature(signature);

    return func;
}

static void ToString(ETSChecker *checker, const ArenaVector<ir::Expression *> &arguments, std::stringstream &ss)
{
    for (auto *arg : arguments) {
        auto *type = arg->Check(checker);
        ss << "-";
        type->ToString(ss);
    }
}

static void ToString([[maybe_unused]] ETSChecker *checker, const ArenaVector<varbinder::LocalVariable *> &arguments,
                     std::stringstream &ss)
{
    for (auto *arg : arguments) {
        auto *type = arg->TsType();
        ss << "-";
        type->ToString(ss);
    }
}

template <typename T>
Signature *ETSChecker::ResolveDynamicCallExpression(ir::Expression *callee, const ArenaVector<T *> &arguments,
                                                    Language lang, bool is_construct)
{
    auto &dynamic_intrinsics = *DynamicCallIntrinsics(is_construct);

    auto map_it = dynamic_intrinsics.find(lang);
    if (map_it == dynamic_intrinsics.cend()) {
        std::tie(map_it, std::ignore) = dynamic_intrinsics.emplace(lang, Allocator()->Adapter());
    }

    auto &map = map_it->second;

    std::stringstream ss;
    ss << "dyncall";
    if (IsByValueCall(VarBinder()->AsETSBinder(), callee)) {
        ss << "-byvalue";
    }

    ToString(this, arguments, ss);

    auto key = ss.str();
    auto it = map.find(util::StringView(key));
    if (it == map.end()) {
        auto *func = CreateDynamicCallIntrinsic(callee, arguments, lang);
        map.emplace(util::UString(key, Allocator()).View(), func);
        return func->Signature();
    }

    return it->second->Signature();
}

template Signature *ETSChecker::ResolveDynamicCallExpression<ir::Expression>(
    ir::Expression *callee, const ArenaVector<ir::Expression *> &arguments, Language lang, bool is_construct);

template Signature *ETSChecker::ResolveDynamicCallExpression<varbinder::LocalVariable>(
    ir::Expression *callee, const ArenaVector<varbinder::LocalVariable *> &arguments, Language lang, bool is_construct);

template <bool IS_STATIC>
std::conditional_t<IS_STATIC, ir::ClassStaticBlock *, ir::MethodDefinition *> ETSChecker::CreateClassInitializer(
    varbinder::ClassScope *class_scope, const ClassInitializerBuilder &builder, ETSObjectType *type)
{
    varbinder::LocalScope *method_scope = nullptr;
    if constexpr (IS_STATIC) {
        method_scope = class_scope->StaticMethodScope();
    } else {
        method_scope = class_scope->InstanceMethodScope();
    }
    auto class_ctx = varbinder::LexicalScope<varbinder::LocalScope>::Enter(VarBinder(), method_scope);

    ArenaVector<ir::Expression *> params(Allocator()->Adapter());

    auto *param_scope = Allocator()->New<varbinder::FunctionParamScope>(Allocator(), class_scope);
    auto *scope = Allocator()->New<varbinder::FunctionScope>(Allocator(), param_scope);

    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());

    ir::ScriptFunction *func = nullptr;
    ir::Identifier *id = nullptr;

    if constexpr (IS_STATIC) {
        builder(scope, &statements, nullptr);
        auto *body = AllocNode<ir::BlockStatement>(Allocator(), scope, std::move(statements));
        id = AllocNode<ir::Identifier>(compiler::Signatures::CCTOR, Allocator());
        func =
            AllocNode<ir::ScriptFunction>(scope, std::move(params), nullptr, body, nullptr,
                                          ir::ScriptFunctionFlags::STATIC_BLOCK | ir::ScriptFunctionFlags::EXPRESSION,
                                          ir::ModifierFlags::STATIC, false, Language(Language::Id::ETS));
    } else {
        builder(scope, &statements, &params);
        auto *body = AllocNode<ir::BlockStatement>(Allocator(), scope, std::move(statements));
        id = AllocNode<ir::Identifier>(compiler::Signatures::CTOR, Allocator());
        func = AllocNode<ir::ScriptFunction>(scope, std::move(params), nullptr, body, nullptr,
                                             ir::ScriptFunctionFlags::CONSTRUCTOR | ir::ScriptFunctionFlags::EXPRESSION,
                                             ir::ModifierFlags::PUBLIC, false, Language(Language::Id::ETS));
    }

    scope->BindNode(func);
    func->SetIdent(id);
    param_scope->BindNode(func);
    scope->BindParamScope(param_scope);
    param_scope->BindFunctionScope(scope);

    auto *signature_info = CreateSignatureInfo();
    signature_info->rest_var = nullptr;
    auto *signature = CreateSignature(signature_info, GlobalVoidType(), func);
    func->SetSignature(signature);

    auto *func_expr = AllocNode<ir::FunctionExpression>(func);

    VarBinder()->AsETSBinder()->BuildInternalName(func);
    VarBinder()->AsETSBinder()->BuildFunctionName(func);
    VarBinder()->Functions().push_back(func->Scope());

    if constexpr (IS_STATIC) {
        auto *static_block = AllocNode<ir::ClassStaticBlock>(func_expr, Allocator());
        static_block->AddModifier(ir::ModifierFlags::STATIC);
        return static_block;
    } else {
        type->AddConstructSignature(signature);

        auto *ctor = Allocator()->New<ir::MethodDefinition>(ir::MethodDefinitionKind::CONSTRUCTOR, id, func_expr,
                                                            ir::ModifierFlags::NONE, Allocator(), false);
        auto *func_type = CreateETSFunctionType(signature, id->Name());
        ctor->SetTsType(func_type);
        func_expr->SetParent(class_scope->Node()->AsClassDeclaration()->Definition());
        func->SetParent(ctor);
        return ctor;
    }
}

ir::ClassStaticBlock *ETSChecker::CreateDynamicCallClassInitializer(varbinder::ClassScope *class_scope, Language lang,
                                                                    bool is_construct)
{
    return CreateClassInitializer<true>(
        class_scope,
        [this, lang, is_construct](varbinder::FunctionScope *scope, ArenaVector<ir::Statement *> *statements,
                                   [[maybe_unused]] ArenaVector<ir::Expression *> *params) {
            auto [builtin_class_name, builtin_method_name] =
                util::Helpers::SplitSignature(is_construct ? compiler::Signatures::Dynamic::InitNewClassBuiltin(lang)
                                                           : compiler::Signatures::Dynamic::InitCallClassBuiltin(lang));
            auto *class_id = AllocNode<ir::Identifier>(builtin_class_name, Allocator());
            auto *method_id = AllocNode<ir::Identifier>(builtin_method_name, Allocator());
            auto *callee = AllocNode<ir::MemberExpression>(class_id, method_id,
                                                           ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);

            ArenaVector<ir::Expression *> call_params(Allocator()->Adapter());

            std::stringstream ss;
            auto name = is_construct ? compiler::Signatures::Dynamic::NewClass(lang)
                                     : compiler::Signatures::Dynamic::CallClass(lang);
            auto package = VarBinder()->Program()->GetPackageName();

            ss << compiler::Signatures::CLASS_REF_BEGIN;
            if (!package.Empty()) {
                std::string package_str(package);
                std::replace(package_str.begin(), package_str.end(), *compiler::Signatures::METHOD_SEPARATOR.begin(),
                             *compiler::Signatures::NAMESPACE_SEPARATOR.begin());
                ss << package_str << compiler::Signatures::NAMESPACE_SEPARATOR;
            }
            ss << name << compiler::Signatures::MANGLE_SEPARATOR;

            auto *class_name = AllocNode<ir::StringLiteral>(util::UString(ss.str(), Allocator()).View());
            call_params.push_back(class_name);

            auto *init_call = AllocNode<ir::CallExpression>(callee, std::move(call_params), nullptr, false);

            {
                ScopeContext ctx(this, scope);
                init_call->Check(this);
            }

            statements->push_back(AllocNode<ir::ExpressionStatement>(init_call));
        });
}

void ETSChecker::BuildClass(util::StringView name, const ClassBuilder &builder)
{
    auto *class_id = AllocNode<ir::Identifier>(name, Allocator());
    auto [decl, var] = VarBinder()->NewVarDecl<varbinder::ClassDecl>(class_id->Start(), class_id->Name());
    class_id->SetVariable(var);

    auto class_ctx = varbinder::LexicalScope<varbinder::ClassScope>(VarBinder());

    auto *class_def = AllocNode<ir::ClassDefinition>(Allocator(), class_ctx.GetScope(), class_id,
                                                     ir::ClassDefinitionModifiers::DECLARATION, ir::ModifierFlags::NONE,
                                                     Language(Language::Id::ETS));

    auto *class_def_type = Allocator()->New<checker::ETSObjectType>(
        Allocator(), class_def->Ident()->Name(), class_def->Ident()->Name(), class_def, checker::ETSObjectFlags::CLASS);
    class_def->SetTsType(class_def_type);

    auto *class_decl = AllocNode<ir::ClassDeclaration>(class_def, Allocator());
    class_decl->SetParent(VarBinder()->TopScope()->Node());
    class_def->Scope()->BindNode(class_decl);
    decl->BindNode(class_def);

    VarBinder()->Program()->Ast()->Statements().push_back(class_decl);

    varbinder::BoundContext bound_ctx(VarBinder()->AsETSBinder()->GetGlobalRecordTable(), class_def);

    ArenaVector<ir::AstNode *> class_body(Allocator()->Adapter());

    builder(class_ctx.GetScope(), &class_body);

    class_def->AddProperties(std::move(class_body));
}

void ETSChecker::BuildDynamicCallClass(bool is_construct)
{
    auto &dynamic_intrinsics = *DynamicCallIntrinsics(is_construct);

    if (dynamic_intrinsics.empty()) {
        return;
    }

    for (auto &entry : dynamic_intrinsics) {
        auto lang = entry.first;
        auto &intrinsics = entry.second;
        auto class_name = is_construct ? compiler::Signatures::Dynamic::NewClass(lang)
                                       : compiler::Signatures::Dynamic::CallClass(lang);
        BuildClass(class_name, [this, lang, &intrinsics, is_construct](varbinder::ClassScope *scope,
                                                                       ArenaVector<ir::AstNode *> *class_body) {
            for (auto &[_, func] : intrinsics) {
                (void)_;

                func->Scope()->ParamScope()->SetParent(scope);

                auto *func_expr = AllocNode<ir::FunctionExpression>(func);

                auto *method = AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD, func->Id(), func_expr,
                                                               ir::ModifierFlags::PUBLIC | ir::ModifierFlags::NATIVE |
                                                                   ir::ModifierFlags::STATIC,
                                                               Allocator(), false);

                VarBinder()->AsETSBinder()->BuildInternalName(func);
                VarBinder()->AsETSBinder()->BuildFunctionName(func);

                class_body->push_back(method);
            }

            class_body->push_back(CreateDynamicCallClassInitializer(scope, lang, is_construct));
        });
    }
}

ir::ClassStaticBlock *ETSChecker::CreateDynamicModuleClassInitializer(
    varbinder::ClassScope *class_scope, const std::vector<ir::ETSImportDeclaration *> &imports)
{
    return CreateClassInitializer<true>(
        class_scope, [this, imports](varbinder::FunctionScope *scope, ArenaVector<ir::Statement *> *statements,
                                     [[maybe_unused]] ArenaVector<ir::Expression *> *params) {
            for (auto *import : imports) {
                auto builtin = compiler::Signatures::Dynamic::LoadModuleBuiltin(import->Language());
                auto [builtin_class_name, builtin_method_name] = util::Helpers::SplitSignature(builtin);

                auto *class_id = AllocNode<ir::Identifier>(builtin_class_name, Allocator());
                auto *method_id = AllocNode<ir::Identifier>(builtin_method_name, Allocator());
                auto *callee = AllocNode<ir::MemberExpression>(class_id, method_id,
                                                               ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);

                ArenaVector<ir::Expression *> call_params(Allocator()->Adapter());
                call_params.push_back(import->ResolvedSource());

                auto *load_call = AllocNode<ir::CallExpression>(callee, std::move(call_params), nullptr, false);

                auto *module_class_id =
                    AllocNode<ir::Identifier>(compiler::Signatures::DYNAMIC_MODULE_CLASS, Allocator());
                auto *field_id = AllocNode<ir::Identifier>(import->AssemblerName(), Allocator());
                auto *property = AllocNode<ir::MemberExpression>(
                    module_class_id, field_id, ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);

                auto *initializer =
                    AllocNode<ir::AssignmentExpression>(property, load_call, lexer::TokenType::PUNCTUATOR_SUBSTITUTION);

                {
                    ScopeContext ctx(this, scope);
                    initializer->Check(this);
                }

                statements->push_back(AllocNode<ir::ExpressionStatement>(initializer));
            }
        });
}

template <bool IS_STATIC>
ir::MethodDefinition *ETSChecker::CreateClassMethod(varbinder::ClassScope *class_scope,
                                                    const std::string_view method_name,
                                                    panda::es2panda::ir::ModifierFlags modifier_flags,
                                                    const MethodBuilder &builder)
{
    auto class_ctx =
        varbinder::LexicalScope<varbinder::LocalScope>::Enter(VarBinder(), class_scope->StaticMethodScope());
    ArenaVector<ir::Expression *> params(Allocator()->Adapter());
    auto *param_scope = Allocator()->New<varbinder::FunctionParamScope>(Allocator(), class_scope);
    auto *scope = Allocator()->New<varbinder::FunctionScope>(Allocator(), param_scope);
    auto *id = AllocNode<ir::Identifier>(method_name, Allocator());

    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
    Type *return_type = nullptr;

    builder(scope, &statements, &params, &return_type);

    auto *body = AllocNode<ir::BlockStatement>(Allocator(), scope, std::move(statements));

    auto *func =
        AllocNode<ir::ScriptFunction>(scope, std::move(params), nullptr, body, nullptr, ir::ScriptFunctionFlags::METHOD,
                                      modifier_flags, false, Language(Language::Id::ETS));
    scope->BindNode(func);
    func->SetIdent(id);
    param_scope->BindNode(func);
    scope->BindParamScope(param_scope);
    param_scope->BindFunctionScope(scope);

    auto *signature_info = CreateSignatureInfo();
    signature_info->rest_var = nullptr;
    auto *signature = CreateSignature(signature_info, return_type, func);
    if constexpr (IS_STATIC) {
        signature->AddSignatureFlag(SignatureFlags::STATIC);
    }
    func->SetSignature(signature);

    auto *func_expr = AllocNode<ir::FunctionExpression>(func);
    auto *method = AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD, func->Id(), func_expr,
                                                   modifier_flags, Allocator(), false);

    VarBinder()->AsETSBinder()->BuildInternalName(func);
    VarBinder()->AsETSBinder()->BuildFunctionName(func);
    VarBinder()->Functions().push_back(func->Scope());

    auto *decl = Allocator()->New<varbinder::LetDecl>(id->Name());
    decl->BindNode(method);

    auto *func_type = CreateETSFunctionType(signature, id->Name());
    auto *var = scope->AddDecl(Allocator(), decl, VarBinder()->Extension());
    var->SetTsType(func_type);
    method->SetTsType(func_type);
    var->AddFlag(varbinder::VariableFlags::PROPERTY);
    func->Id()->SetVariable(var);

    auto *class_type = class_scope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType();
    if constexpr (IS_STATIC) {
        class_type->AddProperty<PropertyType::STATIC_METHOD>(var->AsLocalVariable());
    } else {
        class_type->AddProperty<PropertyType::INSTANCE_METHOD>(var->AsLocalVariable());
    }

    return method;
}

ir::MethodDefinition *ETSChecker::CreateDynamicModuleClassInitMethod(varbinder::ClassScope *class_scope)
{
    return CreateClassMethod<true>(class_scope, compiler::Signatures::DYNAMIC_MODULE_CLASS_INIT,
                                   ir::ModifierFlags::PUBLIC | ir::ModifierFlags::STATIC,
                                   [this]([[maybe_unused]] varbinder::FunctionScope *scope,
                                          [[maybe_unused]] ArenaVector<ir::Statement *> *statements,
                                          [[maybe_unused]] ArenaVector<ir::Expression *> *params,
                                          Type **return_type) { *return_type = GlobalBuiltinVoidType(); });
}

ir::MethodDefinition *ETSChecker::CreateLambdaObjectClassInvokeMethod(varbinder::ClassScope *class_scope,
                                                                      Signature *invoke_signature,
                                                                      ir::TypeNode *ret_type_annotation)
{
    return CreateClassMethod<true>(
        class_scope, compiler::Signatures::LAMBDA_OBJECT_INVOKE, ir::ModifierFlags::PUBLIC,
        [this, class_scope, invoke_signature,
         ret_type_annotation](varbinder::FunctionScope *scope, ArenaVector<ir::Statement *> *statements,
                              ArenaVector<ir::Expression *> *params, Type **return_type) {
            util::UString this_param_name(std::string("this"), Allocator());
            ir::ETSParameterExpression *this_param =
                AddParam(scope->Parent()->AsFunctionParamScope(), this_param_name.View(),
                         class_scope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType());
            params->push_back(this_param);

            ArenaVector<ir::Expression *> call_params(Allocator()->Adapter());
            uint32_t idx = 0;
            for (auto *invoke_param : invoke_signature->Params()) {
                ir::ETSParameterExpression *param = AddParam(
                    scope->Parent()->AsFunctionParamScope(),
                    util::UString(std::string("p") + std::to_string(idx), Allocator()).View(), invoke_param->TsType());
                params->push_back(param);
                call_params.push_back(param);
                ++idx;
            }

            auto *propery_id = AllocNode<ir::Identifier>("jsvalue_lambda", Allocator());
            auto *callee = AllocNode<ir::MemberExpression>(this_param, propery_id,
                                                           ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
            auto *call_lambda = AllocNode<ir::CallExpression>(callee, std::move(call_params), nullptr, false);

            {
                ScopeContext ctx(this, scope);
                call_lambda->Check(this);
            }

            auto *cast_to_ret_type_expr = Allocator()->New<ir::TSAsExpression>(call_lambda, ret_type_annotation, false);
            cast_to_ret_type_expr->SetTsType(invoke_signature->ReturnType());
            auto *ret_statement = Allocator()->New<ir::ReturnStatement>(cast_to_ret_type_expr);
            statements->push_back(ret_statement);

            *return_type = invoke_signature->ReturnType();
        });
}

void ETSChecker::EmitDynamicModuleClassInitCall()
{
    auto *global_class = VarBinder()->Program()->GlobalClass();
    auto &body = global_class->Body();
    auto it = std::find_if(body.begin(), body.end(), [](ir::AstNode *node) { return node->IsClassStaticBlock(); });

    ASSERT(it != body.end());

    auto *static_block = (*it)->AsClassStaticBlock();
    auto *cctor_body = static_block->Function()->Body()->AsBlockStatement();

    auto *class_id = AllocNode<ir::Identifier>(compiler::Signatures::DYNAMIC_MODULE_CLASS, Allocator());
    auto *method_id = AllocNode<ir::Identifier>(compiler::Signatures::DYNAMIC_MODULE_CLASS_INIT, Allocator());
    auto *callee =
        AllocNode<ir::MemberExpression>(class_id, method_id, ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);

    ArenaVector<ir::Expression *> call_params(Allocator()->Adapter());
    auto *init_call = AllocNode<ir::CallExpression>(callee, std::move(call_params), nullptr, false);

    {
        ScopeContext ctx(this, cctor_body->Scope());
        init_call->Check(this);
    }

    cctor_body->Statements().push_back(AllocNode<ir::ExpressionStatement>(init_call));
}

void ETSChecker::BuildDynamicImportClass()
{
    auto dynamic_imports = VarBinder()->AsETSBinder()->DynamicImports();
    if (dynamic_imports.empty()) {
        return;
    }

    BuildClass(compiler::Signatures::DYNAMIC_MODULE_CLASS,
               [this, dynamic_imports](varbinder::ClassScope *scope, ArenaVector<ir::AstNode *> *class_body) {
                   std::unordered_set<util::StringView> fields;
                   std::vector<ir::ETSImportDeclaration *> imports;

                   auto *class_type = scope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType();

                   for (auto *import : dynamic_imports) {
                       auto source = import->Source()->Str();
                       if (fields.find(source) != fields.cend()) {
                           continue;
                       }

                       auto assembly_name = std::string(source);
                       std::replace_if(
                           assembly_name.begin(), assembly_name.end(), [](char c) { return std::isalnum(c) == 0; },
                           '_');
                       assembly_name.append(std::to_string(fields.size()));

                       import->AssemblerName() = util::UString(assembly_name, Allocator()).View();
                       fields.insert(import->AssemblerName());
                       imports.push_back(import);

                       auto *field_ident = AllocNode<ir::Identifier>(import->AssemblerName(), Allocator());
                       auto *field = AllocNode<ir::ClassProperty>(field_ident, nullptr, nullptr,
                                                                  ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC,
                                                                  Allocator(), false);
                       field->SetTsType(GlobalBuiltinDynamicType(import->Language()));

                       auto *decl = Allocator()->New<varbinder::LetDecl>(field_ident->Name());
                       decl->BindNode(field);

                       auto *var = scope->AddDecl(Allocator(), decl, VarBinder()->Extension());
                       var->AddFlag(varbinder::VariableFlags::PROPERTY);
                       field_ident->SetVariable(var);
                       var->SetTsType(GlobalBuiltinDynamicType(import->Language()));

                       class_type->AddProperty<PropertyType::STATIC_FIELD>(var->AsLocalVariable());

                       class_body->push_back(field);
                   }

                   class_body->push_back(CreateDynamicModuleClassInitializer(scope, imports));
                   class_body->push_back(CreateDynamicModuleClassInitMethod(scope));
               });

    EmitDynamicModuleClassInitCall();
}

ir::MethodDefinition *ETSChecker::CreateLambdaObjectClassInitializer(varbinder::ClassScope *class_scope,
                                                                     ETSObjectType *functional_interface)
{
    return CreateClassInitializer<false>(
        class_scope,
        [this, class_scope](varbinder::FunctionScope *scope, ArenaVector<ir::Statement *> *statements,
                            ArenaVector<ir::Expression *> *params) {
            util::UString this_param_name(std::string("this"), Allocator());
            ir::ETSParameterExpression *this_param =
                AddParam(scope->Parent()->AsFunctionParamScope(), this_param_name.View(),
                         class_scope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType());
            params->push_back(this_param);

            util::UString jsvalue_param_name(std::string("jsvalue_param"), Allocator());
            ir::ETSParameterExpression *jsvalue_param = AddParam(scope->Parent()->AsFunctionParamScope(),
                                                                 jsvalue_param_name.View(), GlobalBuiltinJSValueType());
            params->push_back(jsvalue_param);

            auto *module_class_id = AllocNode<ir::Identifier>("this", Allocator());
            auto *field_id = AllocNode<ir::Identifier>("jsvalue_lambda", Allocator());
            auto *property = AllocNode<ir::MemberExpression>(module_class_id, field_id,
                                                             ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);
            auto *initializer =
                AllocNode<ir::AssignmentExpression>(property, jsvalue_param, lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
            {
                ScopeContext ctx(this, scope);
                initializer->Check(this);
            }

            statements->push_back(AllocNode<ir::ExpressionStatement>(initializer));
        },
        functional_interface);
}

void ETSChecker::BuildLambdaObjectClass(ETSObjectType *functional_interface, ir::TypeNode *ret_type_annotation)
{
    auto *invoke_method = functional_interface->GetOwnProperty<checker::PropertyType::INSTANCE_METHOD>("invoke");
    auto *invoke_signature = invoke_method->TsType()->AsETSFunctionType()->CallSignatures()[0];

    std::stringstream ss;
    ss << compiler::Signatures::LAMBDA_OBJECT;
    for (auto *arg : invoke_signature->Params()) {
        ss << "-";
        arg->TsType()->ToString(ss);
    }
    static std::string synthetic_lambda_obj_name {ss.str()};

    if (dynamic_lambda_signature_cache_.find(synthetic_lambda_obj_name) != dynamic_lambda_signature_cache_.end()) {
        functional_interface->AddConstructSignature(dynamic_lambda_signature_cache_[synthetic_lambda_obj_name]);
        return;
    }

    BuildClass(util::StringView(synthetic_lambda_obj_name),
               [this, invoke_signature, ret_type_annotation,
                functional_interface](varbinder::ClassScope *scope, ArenaVector<ir::AstNode *> *class_body) {
                   auto *class_type = scope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType();
                   class_type->AddInterface(functional_interface);

                   auto assembly_name = "jsvalue_lambda";
                   auto *field_ident = AllocNode<ir::Identifier>(assembly_name, Allocator());
                   auto *field = AllocNode<ir::ClassProperty>(field_ident, nullptr, nullptr, ir::ModifierFlags::PRIVATE,
                                                              Allocator(), false);
                   field->SetTsType(GlobalBuiltinJSValueType());

                   auto *decl = Allocator()->New<varbinder::LetDecl>(field_ident->Name());
                   decl->BindNode(field);

                   auto *var = scope->AddDecl(Allocator(), decl, VarBinder()->Extension());
                   var->AddFlag(varbinder::VariableFlags::PROPERTY);
                   var->SetTsType(GlobalBuiltinJSValueType());
                   field_ident->SetVariable(var);

                   class_type->AddProperty<PropertyType::INSTANCE_FIELD>(var->AsLocalVariable());

                   class_body->push_back(field);

                   class_body->push_back(CreateLambdaObjectClassInitializer(scope, functional_interface));

                   class_body->push_back(
                       CreateLambdaObjectClassInvokeMethod(scope, invoke_signature, ret_type_annotation));
               });

    dynamic_lambda_signature_cache_[synthetic_lambda_obj_name] = functional_interface->ConstructSignatures()[0];
}

}  // namespace panda::es2panda::checker
