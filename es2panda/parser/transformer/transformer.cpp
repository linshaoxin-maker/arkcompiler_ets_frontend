/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "transformer.h"

#include <util/ustring.h>

#include "binder/scope.h"
#include "ir/base/catchClause.h"
#include "ir/base/classDefinition.h"
#include "ir/base/classProperty.h"
#include "ir/base/decorator.h"
#include "ir/base/methodDefinition.h"
#include "ir/base/scriptFunction.h"
#include "ir/base/templateElement.h"
#include "ir/expressions/assignmentExpression.h"
#include "ir/expressions/binaryExpression.h"
#include "ir/expressions/callExpression.h"
#include "ir/expressions/classExpression.h"
#include "ir/expressions/functionExpression.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/literals/bigIntLiteral.h"
#include "ir/expressions/literals/numberLiteral.h"
#include "ir/expressions/literals/stringLiteral.h"
#include "ir/expressions/memberExpression.h"
#include "ir/expressions/objectExpression.h"
#include "ir/expressions/sequenceExpression.h"
#include "ir/expressions/superExpression.h"
#include "ir/expressions/templateLiteral.h"
#include "ir/expressions/thisExpression.h"
#include "ir/module/exportDefaultDeclaration.h"
#include "ir/module/exportNamedDeclaration.h"
#include "ir/module/exportSpecifier.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/classDeclaration.h"
#include "ir/statements/doWhileStatement.h"
#include "ir/statements/emptyStatement.h"
#include "ir/statements/expressionStatement.h"
#include "ir/statements/forInStatement.h"
#include "ir/statements/forOfStatement.h"
#include "ir/statements/forUpdateStatement.h"
#include "ir/statements/functionDeclaration.h"
#include "ir/statements/switchStatement.h"
#include "ir/statements/variableDeclaration.h"
#include "ir/statements/variableDeclarator.h"
#include "ir/statements/whileStatement.h"
#include "ir/ts/tsConstructorType.h"
#include "ir/ts/tsEnumDeclaration.h"
#include "ir/ts/tsEnumMember.h"
#include "ir/ts/tsFunctionType.h"
#include "ir/ts/tsImportEqualsDeclaration.h"
#include "ir/ts/tsInterfaceDeclaration.h"
#include "ir/ts/tsMethodSignature.h"
#include "ir/ts/tsModuleBlock.h"
#include "ir/ts/tsModuleDeclaration.h"
#include "ir/ts/tsParameterProperty.h"
#include "ir/ts/tsPrivateIdentifier.h"
#include "ir/ts/tsQualifiedName.h"
#include "ir/ts/tsSignatureDeclaration.h"
#include "ir/ts/tsTypeParameterDeclaration.h"
#include "util/helpers.h"

namespace panda::es2panda::parser {

void Transformer::Transform(Program *program)
{
    program_ = program;
    if (Extension() == ScriptExtension::TS) {
        TransformFromTS();
    }
}

void Transformer::TransformFromTS()
{
    ASSERT(Extension() == ScriptExtension::TS);
    VisitTSNodes(program_->Ast());
    PushVariablesToNearestStatements(program_->Ast());
}

ir::AstNode *Transformer::VisitTSNodes(ir::AstNode *parent)
{
    if (!parent) {
        return nullptr;
    }
    parent->UpdateSelf([this](auto *childNode) { return VisitTSNode(childNode); }, Binder());
    return parent;
}

void Transformer::AddVariableToNearestStatements(util::StringView name)
{
    /*
     *  Add variable declare like 'var ##var_1;' to nearest statements in namespace function or top level scope
     *  Record the variable name and scope in tempVarDeclStatements_ and will push the VariableDeclaration nodes
     *  to statements in PushVariablesToNearestStatements
     */
    auto currentScope = Scope();
    while (currentScope != nullptr) {
        if (currentScope->IsTSModuleScope()) {
            auto node = currentScope->Node();
            ASSERT(node->IsTSModuleDeclaration());
            if (node->AsTSModuleDeclaration()->Body()->IsTSModuleBlock()) {
                break;
            }
        }
        if (currentScope->IsFunctionScope()) {
            auto node = currentScope->Node();
            ASSERT(node->IsScriptFunction());
            if (!node->AsScriptFunction()->FunctionBodyIsExpression()) {
                break;
            }
        }
        currentScope = currentScope->Parent();
    }
    tempVarDeclStatements_.insert({name, currentScope});
}

void Transformer::PushVariablesToNearestStatements(ir::BlockStatement *ast)
{
    /*
     *  Push the VariableDeclaration nodes to nearest statements
     *  For example, transform:
     *  namespace ns {
     *    ...
     *  }
     *
     *  To:
     *  namespace ns {
     *    var ##var_1;
     *    ...
     *  }
     */
    if (tempVarDeclStatements_.empty()) {
        return;
    }
    for (auto it : tempVarDeclStatements_) {
        auto *scope = it.second;
        if (scope == nullptr) {
            auto scopeCtx = binder::LexicalScope<binder::Scope>::Enter(Binder(), ast->Scope());
            ast->AddStatementAtPos(0, CreateVariableDeclarationWithIdentify(it.first, VariableParsingFlags::VAR,
                nullptr, false));
        } else if (scope->IsFunctionScope() || scope->IsTSModuleScope()) {
            auto *body = scope->Node()->AsScriptFunction()->Body();
            ASSERT(body->IsBlockStatement());
            auto scopeCtx = binder::LexicalScope<binder::Scope>::Enter(Binder(), scope);
            body->AsBlockStatement()->AddStatementAtPos(0, CreateVariableDeclarationWithIdentify(it.first,
                VariableParsingFlags::VAR, nullptr, false));
        }
    }
}

binder::Scope *Transformer::FindExportVariableInTsModuleScope(util::StringView name) const
{
    bool isExport = false;
    auto currentScope = Scope();
    while (currentScope != nullptr) {
        binder::Variable *v = currentScope->FindLocal(name, binder::ResolveBindingOptions::ALL);
        bool isTSModuleScope = currentScope->IsTSModuleScope();
        if (v != nullptr) {
            if (!v->Declaration()->IsVarDecl() && !v->Declaration()->IsLetDecl() && !v->Declaration()->IsConstDecl()) {
                break;
            }
            if (isTSModuleScope && currentScope->AsTSModuleScope()->FindExportVariable(name)) {
                isExport = true;
            }
            break;
        }
        if (currentScope->InLocalTSBindings(name) &&
            !currentScope->FindLocalTSVariable<binder::TSBindingType::IMPORT_EQUALS>(name)) {
            break;
        }
        if (isTSModuleScope && currentScope->AsTSModuleScope()->InExportBindings(name)) {
            isExport = true;
            break;
        }
        currentScope = currentScope->Parent();
    }
    if (!isExport) {
        return nullptr;
    }
    return currentScope;
}

ir::UpdateNodes Transformer::VisitTSNode(ir::AstNode *childNode)
{
    ASSERT(childNode != nullptr);
    switch (childNode->Type()) {
        case ir::AstNodeType::IDENTIFIER: {
            auto *ident = childNode->AsIdentifier();
            if (!ident->IsReference() || (!IsTsModule() && !IsTsEnum())) {
                return VisitTSNodes(childNode);
            }

            auto name = ident->Name();
            if (IsTsEnum()) {
                auto scope = FindEnumMemberScope(name);
                if (scope) {
                    return CreateMemberExpressionFromIdentifier(scope, ident);
                }
            }
            if (IsTsModule()) {
                auto scope = FindExportVariableInTsModuleScope(name);
                if (scope) {
                    return CreateMemberExpressionFromIdentifier(scope, ident);
                }
            }

            return VisitTSNodes(childNode);
        }
        case ir::AstNodeType::TS_MODULE_DECLARATION: {
            auto *node = childNode->AsTSModuleDeclaration();
            if (node->Declare() || !node->IsInstantiated()) {
                return childNode;
            }
            auto res = VisitTsModuleDeclaration(node);
            SetOriginalNode(res, childNode);
            return res;
        }
        case ir::AstNodeType::TS_ENUM_DECLARATION: {
            auto *node = childNode->AsTSEnumDeclaration();
            if (node->IsDeclare()) {
                return childNode;
            }
            auto res = VisitTsEnumDeclaration(node);
            SetOriginalNode(res, childNode);
            return res;
        }
        case ir::AstNodeType::EXPORT_NAMED_DECLARATION: {
            auto *node = childNode->AsExportNamedDeclaration();
            auto *decl = node->Decl();
            if (!decl) {
                return VisitTSNodes(childNode);
            }

            if (decl->IsTSModuleDeclaration()) {
                auto *tsModuleDeclaration = decl->AsTSModuleDeclaration();
                if (tsModuleDeclaration->Declare() || !tsModuleDeclaration->IsInstantiated()) {
                    return childNode;
                }
                auto res = VisitTsModuleDeclaration(tsModuleDeclaration, true);
                SetOriginalNode(res, childNode);
                return res;
            }

            if (decl->IsTSEnumDeclaration()) {
                if (decl->AsTSEnumDeclaration()->IsDeclare()) {
                    return childNode;
                }
                auto res = VisitTsEnumDeclaration(decl->AsTSEnumDeclaration(), true);
                SetOriginalNode(res, childNode);
                return res;
            }

            if (IsTsModule()) {
                auto res = VisitExportNamedVariable(decl);
                SetOriginalNode(res, node);
                return res;
            }

            if (decl->IsClassDeclaration()) {
                return VisitExportClassDeclaration<ir::ExportNamedDeclaration>(node);
            }

            return VisitTSNodes(node);
        }
        case ir::AstNodeType::EXPORT_DEFAULT_DECLARATION: {
            auto *node = childNode->AsExportDefaultDeclaration();
            auto *decl = node->Decl();
            if (!decl || !decl->IsClassDeclaration()) {
                return VisitTSNodes(childNode);
            }
            return VisitExportClassDeclaration<ir::ExportDefaultDeclaration>(node);
        }
        case ir::AstNodeType::TS_IMPORT_EQUALS_DECLARATION: {
            auto *node = childNode->AsTSImportEqualsDeclaration();
            auto *express = node->ModuleReference();
            if (express->IsTSExternalModuleReference()) {
                return VisitTSNodes(childNode);
            }
            auto *res = VisitTsImportEqualsDeclaration(node);
            SetOriginalNode(res, childNode);
            return res;
        }
        case ir::AstNodeType::CLASS_DECLARATION: {
            auto *node = childNode->AsClassDeclaration();
            if (node->Definition()->Declare()) {
                return node;
            }
            DuringClass duringClass(&classList_, node->Definition()->GetName());
            node = VisitTSNodes(node)->AsClassDeclaration();
            auto res = VisitClassDeclaration(node);
            SetOriginalNode(res, childNode);
            return res;
        }
        case ir::AstNodeType::CLASS_EXPRESSION: {
            auto *node = childNode->AsClassExpression();
            DuringClass duringClass(&classList_, node->Definition()->GetName());
            node = VisitTSNodes(node)->AsClassExpression();
            auto res = VisitClassExpression(node);
            SetOriginalNode(res, childNode);
            return res;
        }
        case ir::AstNodeType::CLASS_DEFINITION: {
            auto *node = childNode->AsClassDefinition();
            VisitPrivateProperty(node);
            VisitComputedProperty(node);
            auto res = VisitTSNodes(childNode);
            SetOriginalNode(res, childNode);
            return res;
        }
        case ir::AstNodeType::TS_PRIVATE_IDENTIFIER: {
            auto id = childNode->AsTSPrivateIdentifier()->Key()->AsIdentifier();
            auto name = FindPrivatePropertyBindName(id->Name());
            auto res = CreateReferenceIdentifier(name);
            SetOriginalNode(res, childNode);
            return res;
        }
        default: {
            return VisitTSNodes(childNode);
        }
    }
}

template <typename T>
ir::UpdateNodes Transformer::VisitExportClassDeclaration(T *node)
{
    auto *decl = node->Decl();
    auto newDecl = VisitTSNode(decl);
    if (std::holds_alternative<ir::AstNode *>(newDecl)) {
        auto statement = std::get<ir::AstNode *>(newDecl);
        ASSERT(statement->IsClassDeclaration());
        node->SetDecl(statement->AsClassDeclaration());
        return node;
    } else {
        auto statements = std::get<std::vector<ir::AstNode *>>(newDecl);
        std::vector<ir::AstNode *> res;
        auto firstStatement = statements.front();
        statements.erase(statements.begin());
        if (firstStatement->IsVariableDeclaration()) {
            node->SetDecl(firstStatement->AsVariableDeclaration());
        } else {
            ASSERT(firstStatement->IsClassDeclaration());
            node->SetDecl(firstStatement->AsClassDeclaration());
        }
        res.push_back(node);
        res.insert(res.end(), statements.begin(), statements.end());
        return res;
    }
}

util::StringView Transformer::CreateNewVariable(bool needAddToStatements)
{
    util::StringView name = CreateNewVariableName();
    if (needAddToStatements) {
        AddVariableToNearestStatements(name);
    }
    return name;
}

util::StringView Transformer::CreateUniqueName(const std::string &head, size_t *index) const
{
    util::StringView name;
    size_t idx = 0;
    if (index != nullptr) {
        idx = *index;
    }
    do {
        idx++;
        std::stringstream ss;
        ss << head << std::to_string(idx);
        auto s = ss.str();
        if (!Binder()->HasVariableName(util::StringView(s))) {
            name = util::UString(s, Allocator()).View();
            break;
        }
    } while (true);
    if (index != nullptr) {
        *index = idx;
    }
    Binder()->AddDeclarationName(name);
    return name;
}

util::StringView Transformer::CreateNewVariableName() const
{
    auto name = CreateUniqueName(std::string(NEW_VAR_PREFIX) + std::string(NEW_VAR_HEAD));
    return name;
}

ir::UpdateNodes Transformer::VisitClassExpression(ir::ClassExpression *node)
{
    /*
     *  Transform:
     *  var c = class C {
     *    static a = 1
     *  }
     *
     *  To:
     *  var ##var_1;
     *  var c = (##var_1 = class C {},
     *           ##var_1.a = 1,
     *           ##var_1)
     */
    auto instanceComputedProperty = VisitInstanceProperty(node->Definition());

    VisitTSParameterProperty(node->Definition());

    auto varName = CreateNewVariable(false);
    auto staticProperty = VisitStaticProperty(node->Definition(), varName);
    if (instanceComputedProperty.empty() && staticProperty.empty()) {
        return node;
    }
    AddVariableToNearestStatements(varName);

    auto assignment = AllocNode<ir::AssignmentExpression>(CreateReferenceIdentifier(varName),
        node->AsExpression(), lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
    ArenaVector<ir::Expression *> sequence(Allocator()->Adapter());
    sequence.push_back(assignment);

    for (auto *it : instanceComputedProperty) {
        sequence.push_back(it->GetExpression());
    }
    for (auto *it : staticProperty) {
        sequence.push_back(it->GetExpression());
    }

    sequence.push_back(CreateReferenceIdentifier(varName));
    return AllocNode<ir::SequenceExpression>(std::move(sequence));
}

void Transformer::VisitComputedProperty(ir::ClassDefinition *node)
{
    /*
     *  Only create variable for the computed members with decorators or static class property
     *  The new value will be used in the decorators or static property initialize
     *  Transform:
     *  class C {
     *    @f
     *    [a](){}
     *    static [b] = 1
     *    [c] = 1
     *  }
     *
     *  To:
     *  var ##var_1;
     *  var ##var_2;
     *  var ##var_3;
     *  class C {
     *    @f
     *    [##var_1 = a](){}
     *    static [##var_2 = b] = 1
     *    [##var_3 = c] = 1
     *  }
     */
    for (auto *it : node->Body()) {
        if (it->IsClassProperty()) {
            auto *classProperty = it->AsClassProperty();
            if (!classProperty->IsComputed()) {
                continue;
            }
            auto *key = classProperty->Key();
            auto name = CreateNewVariable();
            auto *newKey = AllocNode<ir::AssignmentExpression>(CreateReferenceIdentifier(name),
                key, lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
            classProperty->SetKey(newKey);
            AddComputedPropertyBinding(it, name);
        } else if (it->IsMethodDefinition()) {
            auto *methodDefinition = it->AsMethodDefinition();
            if (!methodDefinition->Computed() ||
                (!methodDefinition->HasDecorators() && !methodDefinition->HasParamDecorators())) {
                continue;
            }
            auto *key = methodDefinition->Key();
            auto name = CreateNewVariable();
            auto *newKey = AllocNode<ir::AssignmentExpression>(CreateReferenceIdentifier(name),
                key, lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
            methodDefinition->SetKey(newKey);
            AddComputedPropertyBinding(it, name);
        }
    }
}

void Transformer::VisitPrivateProperty(ir::ClassDefinition *node)
{
    /*
     *  Create an unique variable name for private property member in class
     *  Transform:
     *  class C {
     *    #a = 1
     *  }
     *
     *  To:
     *  class C {
     *    ##${RecordName}#C#a#1 = 1
     *  }
     */
    for (auto *it : node->Body()) {
        if (!it->IsClassProperty()) {
            continue;
        }
        auto *key = it->AsClassProperty()->Key();
        if (!key->IsTSPrivateIdentifier()) {
            continue;
        }
        auto name = key->AsTSPrivateIdentifier()->Key()->AsIdentifier()->Name();
        auto bindName = CreatePrivatePropertyBindName(name);
        AddPrivatePropertyBinding(name, bindName);
    }
}

util::StringView Transformer::FindPrivatePropertyBindName(util::StringView name)
{
    for (size_t i = classList_.size() - 1; i >= 0; i--) {
        auto res = classList_[i].bindNameMap->find(name);
        if (res != classList_[i].bindNameMap->end()) {
            return res->second;
        }
    }
    UNREACHABLE();
}

util::StringView Transformer::CreatePrivatePropertyBindName(util::StringView name)
{
    std::stringstream head;
    head << NEW_VAR_PREFIX << std::string(RecordName());
    for (auto it : classList_) {
        head << PRIVATE_PROPERTY_SIGN << std::string(it.name);
    }
    head << PRIVATE_PROPERTY_SIGN << std::string(name) << PRIVATE_PROPERTY_SIGN;
    size_t index = GetCurrentClassInfoPropertyIndex();
    auto uniqueName = CreateUniqueName(head.str(), &index);
    SetCurrentClassInfoPropertyIndex(index);
    return uniqueName;
}

std::vector<ir::ExpressionStatement *> Transformer::VisitInstanceProperty(ir::ClassDefinition *node)
{
    /*
     *  Move class InstanceProperty to constructor.
     *  Transform:
     *  class C {
     *    "prop" = 1;
     *  }
     *
     *  To:
     *  class C {
     *    constructor () {
     *      this["prop"] = 1;
     *    }
     *  }
     */
    std::vector<ir::ClassProperty *> addToCtor;
    // Non-null computed properties need to be added outside the class. It is a subset of addToCtor.
    std::vector<ir::ExpressionStatement *> computedProps;
    for (auto *it : node->Body()) {
        if (it->IsClassProperty() && !it->AsClassProperty()->IsStatic() && it->AsClassProperty()->Value() != nullptr) {
            addToCtor.push_back(it->AsClassProperty());
        }
    }
    if (addToCtor.empty()) {
        return {};
    }

    auto ctorScopeCtx = binder::LexicalScope<binder::FunctionScope>::Enter(Binder(), node->Ctor()->Function()->Scope());

    ir::BlockStatement *blockStat = node->Ctor()->Function()->Body()->AsBlockStatement();
    size_t insertPos = (node->Super() == nullptr) ? 0 : 1;
    for (auto *it : addToCtor) {
        if (it->IsComputed()) {
            computedProps.push_back(AllocNode<ir::ExpressionStatement>(it->Key()));
        }
        auto *member = GetClassMemberName(it->Key(), it->IsComputed(), it);
        auto *left = AllocNode<ir::MemberExpression>(AllocNode<ir::ThisExpression>(), member,
                                                     ir::MemberExpression::MemberExpressionKind::ELEMENT_ACCESS,
                                                     true, false);
        auto assignment = AllocNode<ir::AssignmentExpression>(left, it->Value(),
                                                              lexer::TokenType::PUNCTUATOR_SUBSTITUTION);

        ResetParentScopeForAstNode(assignment);
        blockStat->AddStatementAtPos(insertPos, AllocNode<ir::ExpressionStatement>(assignment));
        insertPos++;
    }
    return computedProps;
}

void Transformer::VisitTSParameterProperty(ir::ClassDefinition *node)
{
    /*
     *  Add class property for the parameter property declaration in constructor
     *  Transform:
     *  class C {
     *    constructor(public a = 1) {}
     *  }
     *
     *  To:
     *  class C {
     *    constructor(public a = 1) {
     *      this.a = a;
     *    }
     *  }
     */
    auto *func = node->Ctor()->Function();
    auto *body = func->Body();
    if (body == nullptr) {
        return;
    }
    auto blockStatement = body->AsBlockStatement();
    size_t insertPos = (node->Super() == nullptr) ? 0 : 1;
    for (auto *it : func->Params()) {
        if (!it->IsTSParameterProperty()) {
            continue;
        }
        auto *parameter = it->AsTSParameterProperty()->Parameter();
        util::StringView name;
        // TSParameterPropert only can be identifier or assignment pattern
        if (parameter->IsIdentifier()) {
            name = parameter->AsIdentifier()->Name();
        } else {
            ASSERT(parameter->IsAssignmentPattern());
            auto *left = parameter->AsAssignmentPattern()->Left();
            ASSERT(left->IsIdentifier());
            name = left->AsIdentifier()->Name();
        }
        auto left = AllocNode<ir::MemberExpression>(AllocNode<ir::ThisExpression>(),
            AllocNode<ir::Identifier>(name),
            ir::MemberExpression::MemberExpressionKind::PROPERTY_ACCESS, false, false);
        auto right = CreateReferenceIdentifier(name);
        auto assignment = AllocNode<ir::AssignmentExpression>(left, right,
            lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
        blockStatement->AddStatementAtPos(insertPos, AllocNode<ir::ExpressionStatement>(assignment));
        insertPos++;
    }
}

std::vector<ir::ExpressionStatement *> Transformer::VisitStaticProperty(ir::ClassDefinition *node,
                                                                        util::StringView name)
{
    /*
     *  Create statement for static class property
     *  If it's a conputed property, we should initialize it's new variable first.
     *  Transform:
     *  var ##var_1;
     *  class C {
     *    static a = 1
     *    static [##var_1 = s] = 1
     *  }
     *
     *  To:
     *  var ##var_1;
     *  class C {
     *  }
     *  C.a = 1;
     *  ##var_1 = s;
     *  C[##var_1] = 1;
     *
     *  TODO(xucheng): should support static private property
     */
    std::vector<ir::ExpressionStatement *> res;
    auto classDefinitionBody = node->Body();
    for (auto *it : classDefinitionBody) {
        if (!it->IsClassProperty()) {
            continue;
        }
        auto *classProperty = it->AsClassProperty();
        if (!classProperty->IsStatic()) {
            continue;
        }
        if (classProperty->IsComputed()) {
            res.push_back(AllocNode<ir::ExpressionStatement>(classProperty->Key()));
        }
        auto right = classProperty->Value();
        if (right == nullptr) {
            continue;
        }
        auto *member = GetClassMemberName(classProperty->Key(), classProperty->IsComputed(), classProperty);
        auto left = AllocNode<ir::MemberExpression>(CreateReferenceIdentifier(name), member,
            ir::MemberExpression::MemberExpressionKind::ELEMENT_ACCESS, true, false);
        auto assignment = AllocNode<ir::AssignmentExpression>(left, right, lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
        res.push_back(AllocNode<ir::ExpressionStatement>(assignment));
    }
    return res;
}

ir::UpdateNodes Transformer::VisitClassDeclaration(ir::ClassDeclaration *node)
{
    // TODO(xucheng): maybe will support metadata later
    auto name = node->Definition()->GetName();
    std::vector<ir::AstNode *> res;
    bool hasClassDecorators = node->HasDecorators();
    if (hasClassDecorators) {
        auto definiton = node->Definition();
        auto *clsExpression = AllocNode<ir::ClassExpression>(definiton);
        res.push_back(CreateVariableDeclarationWithIdentify(name, VariableParsingFlags::LET, node, false,
            clsExpression, false));
    } else {
        res.push_back(node);
    }

    auto instanceComputedProperty = VisitInstanceProperty(node->Definition());
    // instanceComputedProperty has been transformed by VisitComputedPerporty before, here is an assignmentExpression.
    if (!instanceComputedProperty.empty()) {
        res.insert(res.end(), instanceComputedProperty.begin(), instanceComputedProperty.end());
    }

    VisitTSParameterProperty(node->Definition());

    auto staticProperty = VisitStaticProperty(node->Definition(), name);
    if (!staticProperty.empty()) {
        res.insert(res.end(), staticProperty.begin(), staticProperty.end());
    }

    auto classDefinitionBody = node->Definition()->Body();
    // decorators of static members, should be called after instance members
    std::vector<ir::AstNode *> staticMemberDecorators;
    for (auto *it : classDefinitionBody) {
        if (it->IsMethodDefinition()) {
            auto *definition = it->AsMethodDefinition();
            bool isStatic = definition->IsStatic();
            auto paramDecorators = CreateParamDecorators(name, definition, false, isStatic);
            if (isStatic) {
                staticMemberDecorators.insert(staticMemberDecorators.end(),
                    paramDecorators.begin(), paramDecorators.end());
            } else {
                res.insert(res.end(), paramDecorators.begin(), paramDecorators.end());
            }
            if (!definition->HasDecorators()) {
                continue;
            }
            auto methodDecorators = CreateMethodDecorators(name, definition, isStatic);
            if (isStatic) {
                staticMemberDecorators.insert(staticMemberDecorators.end(),
                    methodDecorators.begin(), methodDecorators.end());
            } else {
                res.insert(res.end(), methodDecorators.begin(), methodDecorators.end());
            }
        } else if (it->IsClassProperty()) {
            auto *classProperty = it->AsClassProperty();
            bool isStatic = classProperty->IsStatic();
            if (!classProperty->HasDecorators()) {
                continue;
            }
            auto propertyDecorators = CreatePropertyDecorators(name, classProperty, isStatic);
            if (isStatic) {
                staticMemberDecorators.insert(staticMemberDecorators.end(),
                    propertyDecorators.begin(), propertyDecorators.end());
            } else {
                res.insert(res.end(), propertyDecorators.begin(), propertyDecorators.end());
            }
        }
    }

    if (!staticMemberDecorators.empty()) {
        res.insert(res.end(), staticMemberDecorators.begin(), staticMemberDecorators.end());
    }

    // constructor decorators
    auto *ctor = node->Definition()->Ctor();
    auto ctorParamDecorators = CreateParamDecorators(name, ctor, true, false);
    res.insert(res.end(), ctorParamDecorators.begin(), ctorParamDecorators.end());

    // class decorators
    if (hasClassDecorators) {
        auto classDecorators = CreateClassDecorators(node);
        res.insert(res.end(), classDecorators.begin(), classDecorators.end());
    }
    if (res.size() == 1) {
        return res.front();
    }
    return res;
}

std::vector<ir::AstNode *> Transformer::CreateParamDecorators(util::StringView className,
                                                              ir::MethodDefinition *node,
                                                              bool isConstructor,
                                                              bool isStatic)
{
    /*
     *  Param decorators
     *  Transform:
     *  class C {
     *    f(@g a){}
     *  }
     *
     *  To:
     *  class C {
     *    f(a){}
     *  }
     *  g(C.prototype, "f", 0)
     *
     *  Static method or constructor will use constructor function of the class instead of prototype of class
     */
    std::vector<ir::AstNode *> res;
    auto paramsDecorators = node->GetParamDecorators();
    for (int i = paramsDecorators.size() - 1; i >= 0; i--) {
        auto paramIndex = paramsDecorators[i].paramIndex;
        auto decorators = paramsDecorators[i].decorators;
        for (int j = decorators.size() - 1; j >= 0; j--) {
            ArenaVector<ir::Expression *> arguments(Allocator()->Adapter());
            arguments.push_back(CreateDecoratorTarget(className, isConstructor || isStatic));
            arguments.push_back(isConstructor ?
                CreateReferenceIdentifier(CONSTRUCTOR_NAME) :
                GetClassMemberName(node->Key(), node->Computed(), node));
            arguments.push_back(AllocNode<ir::NumberLiteral>(paramIndex));
            auto *callExpr = AllocNode<ir::CallExpression>(decorators[j]->Expr(),
                std::move(arguments), nullptr, false);
            res.push_back(AllocNode<ir::ExpressionStatement>(callExpr));
        }
    }
    return res;
}

std::vector<ir::AstNode *> Transformer::CreatePropertyDecorators(util::StringView className,
                                                                 ir::ClassProperty *node,
                                                                 bool isStatic)
{
    /*
     *  Property decorators
     *  Transform:
     *  class C {
     *    @f a = 1
     *  }
     *
     *  To:
     *  class C {
     *    a = 1
     *  }
     *  f(C.prototype, "a")
     *
     *  Static property will use constructor function of the class instead of prototype of class
     */
    std::vector<ir::AstNode *> res;
    auto decorators = node->Decorators();
    for (int i = decorators.size() - 1; i >= 0; i--) {
        ArenaVector<ir::Expression *> arguments(Allocator()->Adapter());
        arguments.push_back(CreateDecoratorTarget(className, isStatic));
        arguments.push_back(GetClassMemberName(node->Key(), node->IsComputed(), node));
        auto *callExpr = AllocNode<ir::CallExpression>(decorators[i]->Expr(), std::move(arguments), nullptr, false);

        res.push_back(AllocNode<ir::ExpressionStatement>(callExpr));
    }
    return res;
}

std::vector<ir::AstNode *> Transformer::CreateMethodDecorators(util::StringView className,
                                                               ir::MethodDefinition *node,
                                                               bool isStatic)
{
    /*
     *  Method decorators and accessor decorators
     *  Transform:
     *  class C {
     *    @g
     *    f(){}
     *  }
     *
     *  To:
     *  class C {
     *    f(){}
     *  }
     *  Object.defineProperty(C.prototype, "f",
     *    g(C.prototype, "f", Object.getOwnPropertyDescriptor(C.prototype, "f")) ||
     *    Object.getOwnPropertyDescriptor(C.prototype, "f"));
     *
     *  static method will use constructor function of the class instead of prototype of class
     *  If the decorator has a return value, it will be set as the new property of the method
     */
    std::vector<ir::AstNode *> res;
    auto decorators = node->Decorators();
    for (int i = decorators.size() - 1; i >= 0; i--) {
        ArenaVector<ir::Expression *> arguments(Allocator()->Adapter());
        arguments.push_back(CreateDecoratorTarget(className, isStatic));
        arguments.push_back(GetClassMemberName(node->Key(), node->Computed(), node));
        arguments.push_back(CreateGetOwnPropertyDescriptorCall(CreateDecoratorTarget(className, isStatic),
            GetClassMemberName(node->Key(), node->Computed(), node)));
        auto *callExpr = AllocNode<ir::CallExpression>(decorators[i]->Expr(), std::move(arguments), nullptr, false);

        auto *getProperty = CreateGetOwnPropertyDescriptorCall(CreateDecoratorTarget(className, isStatic),
            GetClassMemberName(node->Key(), node->Computed(), node));
        auto newValue = AllocNode<ir::BinaryExpression>(callExpr, getProperty,
            lexer::TokenType::PUNCTUATOR_LOGICAL_OR);

        auto *defineProperty = CreateDefinePropertyCall(CreateDecoratorTarget(className, isStatic),
            GetClassMemberName(node->Key(), node->Computed(), node), newValue);

        res.push_back(AllocNode<ir::ExpressionStatement>(defineProperty));
    }
    return res;
}

ir::Expression *Transformer::CreateDecoratorTarget(util::StringView className, bool isStatic)
{
    if (isStatic) {
        return CreateReferenceIdentifier(className);
    }
    return CreateClassPrototype(className);
}

ir::MemberExpression *Transformer::CreateClassPrototype(util::StringView className)
{
    auto *cls = CreateReferenceIdentifier(className);
    return AllocNode<ir::MemberExpression>(cls, AllocNode<ir::Identifier>(CLASS_PROTOTYPE),
        ir::MemberExpression::MemberExpressionKind::PROPERTY_ACCESS, false, false);
}

ir::CallExpression *Transformer::CreateDefinePropertyCall(ir::Expression *target,
                                                          ir::Expression *key,
                                                          ir::Expression *value)
{
    auto *id = CreateReferenceIdentifier(OBJECT_VAR_NAME);
    auto *caller = AllocNode<ir::MemberExpression>(id, AllocNode<ir::Identifier>(FUNC_NAME_OF_DEFINE_PROPERTY),
        ir::MemberExpression::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    ArenaVector<ir::Expression *> arguments(Allocator()->Adapter());
    arguments.push_back(target);
    arguments.push_back(key);
    arguments.push_back(value);
    return AllocNode<ir::CallExpression>(caller, std::move(arguments), nullptr, false);
}

ir::CallExpression *Transformer::CreateGetOwnPropertyDescriptorCall(ir::Expression *target, ir::Expression *key)
{
    auto *id = CreateReferenceIdentifier(OBJECT_VAR_NAME);
    auto *caller = AllocNode<ir::MemberExpression>(id,
        AllocNode<ir::Identifier>(FUNC_NAME_OF_GET_OWN_PROPERTY_DESCRIPTOR),
        ir::MemberExpression::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    ArenaVector<ir::Expression *> arguments(Allocator()->Adapter());
    arguments.push_back(target);
    arguments.push_back(key);
    return AllocNode<ir::CallExpression>(caller, std::move(arguments), nullptr, false);
}

ir::Expression *Transformer::GetClassMemberName(ir::Expression *key, bool isComputed, ir::Statement *node)
{
    if (isComputed) {
        auto name = GetComputedPropertyBinding(node);
        return AllocNode<ir::Identifier>(name);
    }
    if (key->IsIdentifier()) {
        return AllocNode<ir::StringLiteral>(key->AsIdentifier()->Name());
    } else if (key->IsStringLiteral()) {
        return AllocNode<ir::StringLiteral>(key->AsStringLiteral()->Str());
    } else if (key->IsNumberLiteral()) {
        return AllocNode<ir::NumberLiteral>(key->AsNumberLiteral()->Number(), key->AsNumberLiteral()->Str());
    } else if (key->IsBigIntLiteral()) {
        return AllocNode<ir::BigIntLiteral>(key->AsBigIntLiteral()->Str());
    }
    UNREACHABLE();
    return nullptr;
}

std::vector<ir::AstNode *> Transformer::CreateClassDecorators(ir::ClassDeclaration *node)
{
    /*
     *  Class decorators
     *  Transform:
     *  @f
     *  class C {
     *  }
     *
     *  To:
     *  class C {
     *  }
     *  C = f(C) || C;
     *
     *  If the decorator has a return value, it will be used as the new declaration of the class
     */
    auto name = node->Definition()->GetName();
    auto decorators = node->Decorators();
    auto size = decorators.size();
    std::vector<ir::AstNode *> res;
    for (int i = size - 1; i >= 0; i--) {
        ArenaVector<ir::Expression *> arguments(Allocator()->Adapter());
        arguments.push_back(CreateReferenceIdentifier(name));
        auto *callExpr = AllocNode<ir::CallExpression>(decorators[i]->Expr(), std::move(arguments), nullptr, false);

        auto left = CreateReferenceIdentifier(name);
        auto id = CreateReferenceIdentifier(name);
        auto right = AllocNode<ir::BinaryExpression>(callExpr, id, lexer::TokenType::PUNCTUATOR_LOGICAL_OR);
        auto *assignExpr = AllocNode<ir::AssignmentExpression>(left, right,
            lexer::TokenType::PUNCTUATOR_SUBSTITUTION);

        res.push_back(AllocNode<ir::ExpressionStatement>(assignExpr));
    }
    return res;
}

ir::AstNode *Transformer::VisitTsImportEqualsDeclaration(ir::TSImportEqualsDeclaration *node)
{
    auto *express = node->ModuleReference();
    if (!IsInstantiatedTSModule(express, Scope())) {
        return node;
    }
    auto name = node->Id()->Name();
    if (IsTsModule() && node->IsExport()) {
        auto moduleName = GetCurrentTSModuleName();
        auto *id = CreateReferenceIdentifier(moduleName);
        auto *left = AllocNode<ir::MemberExpression>(id, AllocNode<ir::Identifier>(name),
            ir::MemberExpression::MemberExpressionKind::PROPERTY_ACCESS, false, false);
        ir::Expression *right = CreateMemberExpressionFromQualified(express);
        auto *assignExpr = AllocNode<ir::AssignmentExpression>(left, right,
            lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
        auto *res = AllocNode<ir::ExpressionStatement>(assignExpr);
        return res;
    }

    ir::Expression *init = CreateMemberExpressionFromQualified(express);
    ir::Statement *res = CreateVariableDeclarationWithIdentify(name, VariableParsingFlags::VAR, node,
        node->IsExport(), init);
    if (node->IsExport()) {
        ArenaVector<ir::ExportSpecifier *> specifiers(Allocator()->Adapter());
        res = AllocNode<ir::ExportNamedDeclaration>(res, std::move(specifiers));
        AddExportLocalEntryItem(name, node->Id());
    }
    return res;
}

bool Transformer::IsInstantiatedTSModule(const ir::Expression *node, binder::Scope *scope) const
{
    auto *var = FindTSModuleVariable(node, scope);
    if (var == nullptr) {
        return true;
    }
    auto *decl = var->Declaration();
    ASSERT(decl->IsNamespaceDecl());
    auto tsModules = decl->AsNamespaceDecl()->Decls();
    for (auto *it : tsModules) {
        if (it->IsInstantiated()) {
            return true;
        }
    }
    return false;
}

binder::Variable *Transformer::FindTSModuleVariable(const ir::Expression *node, binder::Scope *scope) const
{
    if (node == nullptr) {
        return nullptr;
    }
    if (node->IsTSQualifiedName()) {
        auto *tsQualifiedName = node->AsTSQualifiedName();
        auto *var = FindTSModuleVariable(tsQualifiedName->Left(), scope);
        if (var == nullptr) {
            return nullptr;
        }
        auto *exportTSBindings = var->AsNamespaceVariable()->GetExportBindings();
        auto name = tsQualifiedName->Right()->Name();
        auto *res = exportTSBindings->FindExportTSVariable<binder::TSBindingType::NAMESPACE>(name);
        if (res != nullptr) {
            return res;
        }
        res = exportTSBindings->FindExportTSVariable<binder::TSBindingType::IMPORT_EQUALS>(name);
        if (res != nullptr) {
            auto *node = res->Declaration()->Node();
            return FindTSModuleVariable(node->Parent()->AsTSImportEqualsDeclaration()->ModuleReference(),
                res->AsImportEqualsVariable()->GetScope());
        }
        return nullptr;
    }
    ASSERT(node->IsIdentifier());
    auto name = node->AsIdentifier()->Name();
    auto *currentScope = scope;
    while (currentScope != nullptr) {
        auto *res = currentScope->FindLocalTSVariable<binder::TSBindingType::NAMESPACE>(name);
        if (res == nullptr && currentScope->IsTSModuleScope()) {
            res = currentScope->AsTSModuleScope()->FindExportTSVariable<binder::TSBindingType::NAMESPACE>(name);
        }
        if (res != nullptr) {
            return res;
        }
        res = currentScope->FindLocalTSVariable<binder::TSBindingType::IMPORT_EQUALS>(name);
        if (res == nullptr && currentScope->IsTSModuleScope()) {
            res = currentScope->AsTSModuleScope()->FindExportTSVariable<binder::TSBindingType::IMPORT_EQUALS>(name);
        }
        if (res != nullptr) {
            auto *node = res->Declaration()->Node();
            return FindTSModuleVariable(node->Parent()->AsTSImportEqualsDeclaration()->ModuleReference(),
                res->AsImportEqualsVariable()->GetScope());
        }
        currentScope = currentScope->Parent();
    }
    return nullptr;
}

std::vector<ir::AstNode *> Transformer::VisitExportNamedVariable(ir::Statement *decl)
{
    std::vector<ir::AstNode *> res;
    if (decl->IsVariableDeclaration()) {
        auto declarators = decl->AsVariableDeclaration()->Declarators();
        for (auto *it : declarators) {
            if (it->Init()) {
                auto *left = std::get<ir::AstNode *>(VisitTSNode(it->Id()))->AsExpression();
                auto *right = std::get<ir::AstNode *>(VisitTSNode(it->Init()))->AsExpression();
                auto *assignExpr = AllocNode<ir::AssignmentExpression>(left, right,
                    lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
                res.push_back(AllocNode<ir::ExpressionStatement>(assignExpr));
            }
        }
    } else if (decl->IsFunctionDeclaration() || decl->IsClassDeclaration()) {
        auto newDecl = VisitTSNode(decl);
        if (std::holds_alternative<ir::AstNode *>(newDecl)) {
            res.push_back(std::get<ir::AstNode *>(newDecl));
        } else {
            auto statements = std::get<std::vector<ir::AstNode *>>(newDecl);
            res.insert(res.end(), statements.begin(), statements.end());
        }

        auto name = decl->IsFunctionDeclaration() ?
            decl->AsFunctionDeclaration()->Function()->Id() :
            decl->AsClassDeclaration()->Definition()->Ident();
        ASSERT(name != nullptr);
        res.push_back(CreateTsModuleAssignment(name->Name()));
    }
    return res;
}

ir::Expression *Transformer::CreateMemberExpressionFromQualified(ir::Expression *node)
{
    if (node->IsTSQualifiedName()) {
        auto *tsQualifiedName = node->AsTSQualifiedName();
        auto *left = CreateMemberExpressionFromQualified(tsQualifiedName->Left());
        auto *right = AllocNode<ir::Identifier>(tsQualifiedName->Right()->Name());
        return AllocNode<ir::MemberExpression>(left, right,
            ir::MemberExpression::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    }
    ASSERT(node->IsIdentifier());
    auto *id = CreateReferenceIdentifier(node->AsIdentifier()->Name());
    return id;
}

void Transformer::SetOriginalNode(ir::UpdateNodes res, ir::AstNode *originalNode) const
{
    if (std::holds_alternative<ir::AstNode *>(res)) {
        auto *node = std::get<ir::AstNode *>(res);
        if (node == nullptr || node == originalNode) {
            return;
        }
        node->SetOriginal(originalNode);
        node->SetRange(originalNode->Range());
    } else {
        auto nodes = std::get<std::vector<ir::AstNode *>>(res);
        for (auto *it : nodes) {
            it->SetOriginal(originalNode);
            it->SetRange(originalNode->Range());
        }
    }
}

ir::ExpressionStatement *Transformer::CreateTsModuleAssignment(util::StringView name)
{
    auto moduleName = GetCurrentTSModuleName();
    auto *id = CreateReferenceIdentifier(moduleName);
    auto *left = AllocNode<ir::MemberExpression>(id, AllocNode<ir::Identifier>(name),
        ir::MemberExpression::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    auto *right = CreateReferenceIdentifier(name);
    auto *assignExpr = AllocNode<ir::AssignmentExpression>(left, right, lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
    return AllocNode<ir::ExpressionStatement>(assignExpr);
}

util::StringView Transformer::GetNameFromModuleDeclaration(ir::TSModuleDeclaration *node) const
{
    return node->Name()->AsIdentifier()->Name();
}

ir::VariableDeclaration *Transformer::CreateVariableDeclarationWithIdentify(util::StringView name,
                                                                            VariableParsingFlags flags,
                                                                            ir::AstNode *node,
                                                                            bool isExport,
                                                                            ir::Expression *init,
                                                                            bool needBinding)
{
    auto *ident = CreateReferenceIdentifier(name);
    auto *declarator = AllocNode<ir::VariableDeclarator>(ident, init);
    ArenaVector<ir::VariableDeclarator *> declarators(Allocator()->Adapter());
    declarators.push_back(declarator);

    auto varKind = ir::VariableDeclaration::VariableDeclarationKind::VAR;
    if (flags & VariableParsingFlags::VAR) {
    } else if (flags & VariableParsingFlags::LET) {
        varKind = ir::VariableDeclaration::VariableDeclarationKind::LET;
    } else {
        varKind = ir::VariableDeclaration::VariableDeclarationKind::CONST;
    }
    auto *declaration = AllocNode<ir::VariableDeclaration>(varKind, std::move(declarators), false);

    lexer::SourcePosition startPos(0, 0);
    if (node != nullptr) {
        startPos = node->Start();
    }
    if (needBinding) {
        binder::Decl *decl = nullptr;
        binder::DeclarationFlags declflag = isExport ?
            binder::DeclarationFlags::EXPORT :
            binder::DeclarationFlags::NONE;
        if (flags & VariableParsingFlags::VAR) {
            decl = Binder()->AddDecl<binder::VarDecl>(startPos, declflag, false, name);
        } else if (flags & VariableParsingFlags::LET) {
            decl = Binder()->AddDecl<binder::LetDecl>(startPos, declflag, false, name);
        } else {
            decl = Binder()->AddDecl<binder::ConstDecl>(startPos, declflag, false, name);
        }
        decl->BindNode(declaration);
    }

    return declaration;
}

util::StringView Transformer::GetParamName(ir::AstNode *node, util::StringView name) const
{
    if (node->IsTSModuleDeclaration()) {
        auto scope = node->AsTSModuleDeclaration()->Scope();
        if (scope && !scope->HasVariableName(name)) {
            return name;
        }
    }
    if (node->IsTSEnumDeclaration()) {
        auto scope = node->AsTSEnumDeclaration()->Scope();
        if (scope && !scope->HasDeclarationName(name)) {
            return name;
        }
    }

    auto uniqueName = CreateUniqueName(std::string(name) + std::string(INDEX_DIVISION));
    return uniqueName;
}

ir::CallExpression *Transformer::CreateCallExpressionForTsModule(ir::TSModuleDeclaration *node,
                                                                 util::StringView name,
                                                                 bool isExport)
{
    ir::ScriptFunction *funcNode = nullptr;

    binder::FunctionScope *funcScope = node->Scope();
    binder::FunctionParamScope *funcParamScope = funcScope->ParamScope();
    auto paramName = GetParamName(node, name);
    {
        auto paramScopeCtx = binder::LexicalScope<binder::FunctionParamScope>::Enter(Binder(), funcParamScope);

        ArenaVector<ir::Expression *> params(Allocator()->Adapter());
        auto *parameter = CreateReferenceIdentifier(paramName);
        Binder()->AddParamDecl(parameter);
        params.push_back(parameter);

        ir::BlockStatement *blockNode = nullptr;
        {
            auto scopeCtx = binder::LexicalScope<binder::FunctionScope>::Enter(Binder(), funcScope);
            tsModuleList_.push_back({paramName, funcScope});
            if (node->Body()->IsTSModuleDeclaration()) {
                auto *tsModule = node->Body()->AsTSModuleDeclaration();
                auto body = std::get<std::vector<ir::AstNode *>>(VisitTsModuleDeclaration(tsModule, true));
                ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
                for (auto *it : body) {
                    statements.push_back(static_cast<ir::Statement *>(it));
                }
                blockNode = AllocNode<ir::BlockStatement>(funcScope, std::move(statements));
            } else {
                auto body = VisitTSNodes(node->Body());
                blockNode = AllocNode<ir::BlockStatement>(funcScope,
                    std::move(body->AsTSModuleBlock()->Statements()));
            }
            tsModuleList_.pop_back();
            funcScope->AddBindsFromParam();
        }

        funcNode = AllocNode<ir::ScriptFunction>(funcScope, std::move(params), nullptr, blockNode, nullptr,
            ir::ScriptFunctionFlags::NONE, false, Extension() == ScriptExtension::TS);

        funcScope->BindNode(funcNode);
        funcParamScope->BindNode(funcNode);
    }

    auto *funcExpr = AllocNode<ir::FunctionExpression>(funcNode);

    ArenaVector<ir::Expression *> arguments = CreateCallExpressionArguments(name, isExport);
    auto *callExpr = AllocNode<ir::CallExpression>(funcExpr, std::move(arguments), nullptr, false);

    return callExpr;
}

ir::Expression *Transformer::CreateTsModuleParam(util::StringView paramName, bool isExport)
{
    if (isExport) {
        auto moduleName = GetCurrentTSModuleName();
        auto *id = CreateReferenceIdentifier(moduleName);
        return AllocNode<ir::MemberExpression>(id, AllocNode<ir::Identifier>(paramName),
            ir::MemberExpression::MemberExpressionKind::PROPERTY_ACCESS, false, false);
    }

    auto *id = CreateReferenceIdentifier(paramName);
    return id;
}

void Transformer::AddExportLocalEntryItem(util::StringView name, const ir::Identifier *identifier)
{
    auto moduleRecord = GetSourceTextModuleRecord();
    auto *entry = moduleRecord->NewEntry<SourceTextModuleRecord::ExportEntry>(name, name, identifier, identifier);
    [[maybe_unused]] bool res = moduleRecord->AddLocalExportEntry(entry);
    ASSERT(res);
}

ir::UpdateNodes Transformer::VisitTsModuleDeclaration(ir::TSModuleDeclaration *node, bool isExport)
{
    std::vector<ir::AstNode *> res;

    util::StringView name = GetNameFromModuleDeclaration(node);

    auto findRes = Scope()->FindLocal(name, binder::ResolveBindingOptions::ALL);
    if (findRes == nullptr) {
        res.push_back(CreateVariableDeclarationForTSEnumOrTSModule(name, node, isExport));
    }

    auto *callExpr = CreateCallExpressionForTsModule(node, name, isExport && IsTsModule());
    auto *exprStatementNode = AllocNode<ir::ExpressionStatement>(callExpr);
    res.push_back(exprStatementNode);

    return res;
}

ir::Identifier *Transformer::CreateReferenceIdentifier(util::StringView name)
{
    auto *node = AllocNode<ir::Identifier>(name);
    node->AsIdentifier()->SetReference();
    return node;
}

ir::UpdateNodes Transformer::VisitTsEnumDeclaration(ir::TSEnumDeclaration *node, bool isExport)
{
    std::vector<ir::AstNode *> res;

    util::StringView name = GetNameFromTsEnumDeclaration(node);

    auto findRes = Scope()->FindLocal(name);  // Find if the variable with the same name is already defined
    if (findRes == nullptr) {
        res.push_back(CreateVariableDeclarationForTSEnumOrTSModule(name, node, isExport));
    }

    auto *callExpr = CreateCallExpressionForTsEnum(node, name, isExport && IsTsModule());
    auto *exprStatementNode = AllocNode<ir::ExpressionStatement>(callExpr);
    res.push_back(exprStatementNode);

    return res;
}

ir::AstNode *Transformer::CreateVariableDeclarationForTSEnumOrTSModule(util::StringView name,
                                                                       ir::AstNode *node, bool isExport)
{
    auto flag = Scope()->Parent() == nullptr ? VariableParsingFlags::VAR : VariableParsingFlags::LET;
    auto *variableDeclaration = CreateVariableDeclarationWithIdentify(name, flag, node, isExport);
    bool doExport = isExport && !IsTsModule();
    if (doExport) {  // export var
        ArenaVector<ir::ExportSpecifier *> specifiers(Allocator()->Adapter());
        auto *exportDeclaration = AllocNode<ir::ExportNamedDeclaration>(variableDeclaration, std::move(specifiers));
        auto *ident = node->IsTSEnumDeclaration() ?
            node->AsTSEnumDeclaration()->Key()->AsIdentifier() : node->AsTSModuleDeclaration()->Name()->AsIdentifier();
        AddExportLocalEntryItem(name, ident);
        return exportDeclaration;
    }
    return variableDeclaration;
}

util::StringView Transformer::GetNameFromTsEnumDeclaration(const ir::TSEnumDeclaration *node) const
{
    auto *name = node->AsTSEnumDeclaration()->Key();
    return name->AsIdentifier()->Name();
}

ir::CallExpression *Transformer::CreateCallExpressionForTsEnum(ir::TSEnumDeclaration *node, util::StringView name,
                                                               bool isExport)
{
    ir::ScriptFunction *funcNode = nullptr;

    binder::FunctionScope *funcScope = node->Scope();
    binder::FunctionParamScope *funcParamScope = funcScope->ParamScope();
    util::StringView paramName = GetParamName(node, name);  // modify the name of the function param
    {
        auto paramScopeCtx = binder::LexicalScope<binder::FunctionParamScope>::Enter(Binder(), funcParamScope);
        // create function param
        ArenaVector<ir::Expression *> params(Allocator()->Adapter());
        auto *parameter = CreateReferenceIdentifier(paramName);
        Binder()->AddParamDecl(parameter);
        params.push_back(parameter);
        // create function body
        ir::BlockStatement *blockNode = nullptr;
        {
            auto scopeCtx = binder::LexicalScope<binder::FunctionScope>::Enter(Binder(), funcScope);
            tsEnumList_.push_back({paramName, funcScope});

            ArenaVector<ir::TSEnumMember *> members = node->Members();
            ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
            ir::TSEnumMember *preTsEnumMember = nullptr;
            for (auto member : members) {
                auto *currTsEnumMember = member->AsTSEnumMember();
                auto statement = CreateTsEnumMember(currTsEnumMember, preTsEnumMember, paramName);
                preTsEnumMember = currTsEnumMember;
                statements.push_back(statement);
            }

            blockNode = AllocNode<ir::BlockStatement>(funcScope, std::move(statements));
            tsEnumList_.pop_back();
            funcScope->AddBindsFromParam();
        }
        funcNode = AllocNode<ir::ScriptFunction>(funcScope, std::move(params), nullptr, blockNode, nullptr,
            ir::ScriptFunctionFlags::NONE, false, Extension() == ScriptExtension::TS);

        funcScope->BindNode(funcNode);
        funcParamScope->BindNode(funcNode);
    }
    auto *funcExpr = AllocNode<ir::FunctionExpression>(funcNode);

    ArenaVector<ir::Expression *> arguments = CreateCallExpressionArguments(name, isExport);
    auto *callExpr = AllocNode<ir::CallExpression>(funcExpr, std::move(arguments), nullptr, false);

    return callExpr;
}

ArenaVector<ir::Expression *> Transformer::CreateCallExpressionArguments(util::StringView name, bool isExport)
{
    ArenaVector<ir::Expression *> arguments(Allocator()->Adapter());
    ArenaVector<ir::Expression *> properties(Allocator()->Adapter());
    auto *objectExpression = AllocNode<ir::ObjectExpression>(ir::AstNodeType::OBJECT_EXPRESSION,
                                                             std::move(properties),
                                                             false);
    auto assignExpr = AllocNode<ir::AssignmentExpression>(CreateTsModuleParam(name, isExport),
                                                          objectExpression,
                                                          lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
    auto argument = AllocNode<ir::BinaryExpression>(CreateTsModuleParam(name, isExport),
                                                    assignExpr,
                                                    lexer::TokenType::PUNCTUATOR_LOGICAL_OR);
    if (isExport) {
        auto *id = CreateReferenceIdentifier(name);
        arguments.push_back(AllocNode<ir::AssignmentExpression>(id, argument,
            lexer::TokenType::PUNCTUATOR_SUBSTITUTION));
    } else {
        arguments.push_back(argument);
    }

    return arguments;
}

ir::ExpressionStatement *Transformer::CreateTsEnumMember(ir::TSEnumMember *node, ir::TSEnumMember *preNode,
                                                         util::StringView enumLiteralName)
{
    util::StringView enumMemberName = GetNameFromEnumMember(node);
    binder::Variable *enumVar = Scope()->AsTSEnumScope()->FindEnumMemberVariable(enumMemberName);
    ASSERT(enumVar);
    if (node->Init() != nullptr) {
        bool isStringInit = enumVar->AsEnumVariable()->StringInit();
        if (!enumVar->AsEnumVariable()->IsVisited()) {
            isStringInit = IsStringInitForEnumMember(node->Init(), Scope());
            if (isStringInit) {
                enumVar->AsEnumVariable()->SetStringInit();
            }
            enumVar->AsEnumVariable()->SetVisited();
        }
        return isStringInit ? CreateTsEnumMemberWithStringInit(node, enumLiteralName, enumMemberName) :
                              CreateTsEnumMemberWithNumberInit(node, enumLiteralName, enumMemberName);
    }

    enumVar->AsEnumVariable()->SetVisited();
    return CreateTsEnumMemberWithoutInit(node, preNode, enumLiteralName, enumMemberName);
}

ir::ExpressionStatement *Transformer::CreateTsEnumMemberWithStringInit(ir::TSEnumMember *node,
                                                                       util::StringView enumLiteralName,
                                                                       util::StringView enumMemberName)
{
    // transform to the shape like E["a"] = "str";
    auto *object = CreateReferenceIdentifier(enumLiteralName);
    auto *property = AllocNode<ir::StringLiteral>(enumMemberName);
    auto *left = AllocNode<ir::MemberExpression>(object, property,
                                                 ir::MemberExpression::MemberExpressionKind::ELEMENT_ACCESS,
                                                 true, false);
    auto *right = std::get<ir::AstNode *>(VisitTSNode(node->Init()))->AsExpression();

    auto *assignExpr = AllocNode<ir::AssignmentExpression>(left, right, lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
    auto *exprStatementNode = AllocNode<ir::ExpressionStatement>(assignExpr);

    return exprStatementNode;
}

ir::ExpressionStatement *Transformer::CreateTsEnumMemberWithNumberInit(ir::TSEnumMember *node,
                                                                       util::StringView enumLiteralName,
                                                                       util::StringView enumMemberName)
{
    // transform to the shape like E[E["a"] = init] = "a";
    auto *innerObject = CreateReferenceIdentifier(enumLiteralName);
    auto *innerProperty = AllocNode<ir::StringLiteral>(enumMemberName);
    auto *innerLeft = AllocNode<ir::MemberExpression>(innerObject, innerProperty,
                                                      ir::MemberExpression::MemberExpressionKind::ELEMENT_ACCESS,
                                                      true, false);
    auto *innerRight = std::get<ir::AstNode *>(VisitTSNode(node->Init()))->AsExpression();

    auto *object = CreateReferenceIdentifier(enumLiteralName);
    auto *property = AllocNode<ir::AssignmentExpression>(innerLeft, innerRight,
                                                         lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
    auto *left = AllocNode<ir::MemberExpression>(object, property,
                                                 ir::MemberExpression::MemberExpressionKind::ELEMENT_ACCESS,
                                                 true, false);

    auto *right = AllocNode<ir::StringLiteral>(enumMemberName);

    auto *assignExpr = AllocNode<ir::AssignmentExpression>(left, right, lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
    auto *exprStatementNode = AllocNode<ir::ExpressionStatement>(assignExpr);

    return exprStatementNode;
}

ir::ExpressionStatement *Transformer::CreateTsEnumMemberWithoutInit(ir::TSEnumMember *node,
                                                                    ir::TSEnumMember *preNode,
                                                                    util::StringView enumLiteralName,
                                                                    util::StringView enumMemberName)
{
    // transform to the shape like E[E["a"] = value] = "a";
    auto *innerObject = CreateReferenceIdentifier(enumLiteralName);
    auto *innerProperty = AllocNode<ir::StringLiteral>(enumMemberName);
    auto *innerLeft = AllocNode<ir::MemberExpression>(innerObject, innerProperty,
                                                      ir::MemberExpression::MemberExpressionKind::ELEMENT_ACCESS,
                                                      true, false);

    ir::AssignmentExpression *property = nullptr;
    if (preNode == nullptr) {  // first enumMember, value = 0
        auto *innerRight = AllocNode<ir::NumberLiteral>(0);
        property = AllocNode<ir::AssignmentExpression>(innerLeft, innerRight,
                                                       lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
    } else {  // not first enumMember, value = E.prenode + 1
        auto *innerRightObject = CreateReferenceIdentifier(enumLiteralName);
        auto *innerPropertyForMemberExpr = AllocNode<ir::Identifier>(GetNameFromEnumMember(preNode));
        auto *innerMemberExpr = AllocNode<ir::MemberExpression>(innerRightObject, innerPropertyForMemberExpr,
            ir::MemberExpression::MemberExpressionKind::PROPERTY_ACCESS, false, false);
        auto *innerRight = AllocNode<ir::BinaryExpression>(innerMemberExpr, AllocNode<ir::NumberLiteral>(1),
                                                           lexer::TokenType::PUNCTUATOR_PLUS);
        property = AllocNode<ir::AssignmentExpression>(innerLeft, innerRight,
                                                       lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
    }
    auto *object = CreateReferenceIdentifier(enumLiteralName);
    auto *left = AllocNode<ir::MemberExpression>(object, property,
                                                 ir::MemberExpression::MemberExpressionKind::ELEMENT_ACCESS,
                                                 true, false);

    auto *right = AllocNode<ir::StringLiteral>(enumMemberName);

    auto *assignExpr = AllocNode<ir::AssignmentExpression>(left, right, lexer::TokenType::PUNCTUATOR_SUBSTITUTION);
    auto *exprStatementNode = AllocNode<ir::ExpressionStatement>(assignExpr);

    return exprStatementNode;
}

bool Transformer::IsStringInitForEnumMember(const ir::Expression *expr, binder::Scope *scope) const
{
    if (expr == nullptr) {
        return false;
    }

    // The string enumMember is either initialized with a string literal, or with another string enumMember.
    switch (expr->Type()) {
        case ir::AstNodeType::STRING_LITERAL:
        case ir::AstNodeType::TEMPLATE_LITERAL: {
            // TemplateLiteral in Enum must be a string literal.
            return true;
        }
        case ir::AstNodeType::IDENTIFIER: {
            // Return true if this identifier is a string enumMember of the current Enum.
            util::StringView identName = expr->AsIdentifier()->Name();
            ASSERT(scope && scope->IsTSEnumScope());
            binder::Variable *v = scope->AsTSEnumScope()->FindEnumMemberVariable(identName);
            if (v == nullptr) {
                return false;
            }
            if (!v->AsEnumVariable()->IsVisited()) {  // visit the quoted item
                auto *initExpr = v->AsEnumVariable()->Declaration()->Node()->AsTSEnumMember()->Init();
                if (IsStringInitForEnumMember(initExpr, scope)) {
                    v->AsEnumVariable()->SetStringInit();
                }
                v->AsEnumVariable()->SetVisited();
            }
            if (v->AsEnumVariable()->IsVisited() && v->AsEnumVariable()->StringInit()) {
                return true;
            }

            return false;
        }
        case ir::AstNodeType::MEMBER_EXPRESSION: {
            return IsStringForMemberExpression(expr->AsMemberExpression(), scope);
        }
        case ir::AstNodeType::BINARY_EXPRESSION: {
            auto *left = expr->AsBinaryExpression()->Left();
            auto *right = expr->AsBinaryExpression()->Right();
            if (expr->AsBinaryExpression()->OperatorType() == lexer::TokenType::PUNCTUATOR_PLUS &&
                IsStringInitForEnumMember(right, scope) && IsStringInitForEnumMember(left, scope)) {
                return true;
            }
            return false;
        }
        default:
            return false;
    }

    return false;
}

bool Transformer::IsStringForMemberExpression(const ir::MemberExpression *memberExpr, binder::Scope *scope) const
{
    // Return true only if memberExpression is a string enumMember.
    const ir::Expression *expr = memberExpr;
    ArenaDeque<const ir::Expression *> members(Allocator()->Adapter());
    while (expr->IsMemberExpression()) {
        if (expr->AsMemberExpression()->Property()->IsIdentifier() ||
            expr->AsMemberExpression()->Property()->IsStringLiteral() ||
            expr->AsMemberExpression()->Property()->IsTemplateLiteral()) {
            members.push_front(expr->AsMemberExpression()->Property());
            expr = expr->AsMemberExpression()->Object();
        } else {
            return false;
        }
    }
    if (!expr->IsIdentifier()) {
        return false;
    }
    members.push_front(expr->AsIdentifier());

    // Find front Ident TSVariables
    ArenaVector<binder::Variable *> findRes = FindFrontIdentifierTSVariables(members.front()->AsIdentifier(), scope);
    members.pop_front();

    for (auto currVar : findRes) {
        if (VerifyMemberExpressionDeque(currVar, members)) {
            return true;
        }
    }
    return false;
}

ArenaVector<binder::Variable *> Transformer::FindFrontIdentifierTSVariables(const ir::Identifier *ident,
                                                                            binder::Scope *scope) const
{
    util::StringView name = ident->Name();
    binder::Variable *v = nullptr;
    ArenaVector<binder::Variable *> findRes(Allocator()->Adapter());
    while (scope != nullptr) {
        // find enumMemberBindings_
        if (scope->IsTSEnumScope()) {
            v = scope->AsTSEnumScope()->FindEnumMemberVariable(name);
            if (v != nullptr) {
                break;
            }
        }

        const std::vector<binder::TSBindingType> types = {binder::TSBindingType::NAMESPACE,
                                                          binder::TSBindingType::ENUMLITERAL,
                                                          binder::TSBindingType::IMPORT_EQUALS};
        // find tsBindings_
        FindLocalTSVariables(scope, name, types, findRes);
        // find exportTSBindings_
        if (scope->IsTSModuleScope()) {
            FindExportTSVariables(scope, name, types, findRes);
        }

        if (!findRes.empty()) {
            break;
        }

        // find js variable
        v = scope->FindLocal(name);
        if (v != nullptr) {
            if (scope->IsTSModuleScope() || scope->IsTSEnumScope()) {  // v may be converted from ts variable
                v = scope->Parent()->FindLocal(name);
                if (v == nullptr) {
                    break;
                }
            } else {
                break;
            }
        }
        if (scope->IsTSModuleScope()) {
            v = scope->AsTSModuleScope()->FindExportVariable(name);
            if (v != nullptr) {
                break;
            }
        }

        if (scope->IsTSModuleScope() || scope->IsTSEnumScope()) {
            scope = scope->Parent();
        }
        scope = scope->Parent();
    }

    return findRes;
}

bool Transformer::IsInstantiatedNamespaceVariable(binder::Variable *var) const
{
    ASSERT(var->IsNamespaceVariable());
    auto *decl = var->AsNamespaceVariable()->Declaration();
    ASSERT(decl->IsNamespaceDecl());
    ArenaVector<ir::TSModuleDeclaration *> nodes = decl->AsNamespaceDecl()->Decls();
    for (ir::TSModuleDeclaration *node : nodes) {
        if (node->IsInstantiated()) {
            return true;
        }
    }
    return false;
}

void Transformer::FindLocalTSVariables(binder::Scope *scope, const util::StringView name,
                                       const std::vector<binder::TSBindingType> &types,
                                       ArenaVector<binder::Variable *> &findRes) const
{
    for (binder::TSBindingType type : types) {
        binder::Variable *v = nullptr;
        switch (type) {
            case binder::TSBindingType::NAMESPACE: {
                v = scope->FindLocalTSVariable<binder::TSBindingType::NAMESPACE>(name);
                if (v != nullptr && !IsInstantiatedNamespaceVariable(v)) {
                    v = nullptr;
                }
                break;
            }
            case binder::TSBindingType::ENUMLITERAL: {
                v = scope->FindLocalTSVariable<binder::TSBindingType::ENUMLITERAL>(name);
                break;
            }
            case binder::TSBindingType::IMPORT_EQUALS: {
                v = scope->FindLocalTSVariable<binder::TSBindingType::IMPORT_EQUALS>(name);
                if (v != nullptr &&
                    !IsInstantiatedTSModule(v->AsImportEqualsVariable()->Declaration()->Node()->
                    Parent()->AsTSImportEqualsDeclaration()->ModuleReference(), scope)) {
                    v = nullptr;
                }
                break;
            }
            default:
                continue;
        }
        if (v != nullptr) {
            findRes.push_back(v);
        }
    }
}

void Transformer::FindExportTSVariables(binder::Scope *scope, const util::StringView name,
                                        const std::vector<binder::TSBindingType> &types,
                                        ArenaVector<binder::Variable *> &findRes) const
{
    for (binder::TSBindingType type : types) {
        binder::Variable *v = nullptr;
        switch (type) {
            case binder::TSBindingType::NAMESPACE: {
                v = scope->AsTSModuleScope()->FindExportTSVariable<binder::TSBindingType::NAMESPACE>(name);
                if (v != nullptr && !IsInstantiatedNamespaceVariable(v)) {
                    v = nullptr;
                }
                break;
            }
            case binder::TSBindingType::ENUMLITERAL: {
                v = scope->AsTSModuleScope()->FindExportTSVariable<binder::TSBindingType::ENUMLITERAL>(name);
                break;
            }
            case binder::TSBindingType::IMPORT_EQUALS: {
                v = scope->AsTSModuleScope()->FindExportTSVariable<binder::TSBindingType::IMPORT_EQUALS>(name);
                if (v != nullptr &&
                    !IsInstantiatedTSModule(v->AsImportEqualsVariable()->Declaration()->Node()->
                    Parent()->AsTSImportEqualsDeclaration()->ModuleReference(), scope)) {
                    v = nullptr;
                }
                break;
            }
            default:
                continue;
        }
        if (v != nullptr) {
            findRes.push_back(v);
        }
    }
}

bool Transformer::VerifyMemberExpressionDeque(binder::Variable *currVar,
                                              ArenaDeque<const ir::Expression *> members) const
{
    ASSERT(!members.empty());
    switch (currVar->Flags()) {
        case binder::VariableFlags::ENUM_LITERAL: {
            // the recursion ends.
            util::StringView enumMemberName = GetNameForMemberExpressionItem(members.front());
            members.pop_front();
            if (!members.empty()) {
                return false;
            }
            binder::Variable *enumMemberVar = currVar->AsEnumLiteralVariable()->FindEnumMemberVariable(enumMemberName);
            if (enumMemberVar == nullptr) {
                return false;
            }
            if (!enumMemberVar->AsEnumVariable()->IsVisited()) {  // visit the quoted item
                auto *scope = enumMemberVar->AsEnumVariable()->Declaration()->
                              Node()->Parent()->AsTSEnumDeclaration()->Scope();
                auto *initExpr = enumMemberVar->AsEnumVariable()->Declaration()->Node()->AsTSEnumMember()->Init();
                if (IsStringInitForEnumMember(initExpr, scope)) {
                    enumMemberVar->AsEnumVariable()->SetStringInit();
                }
                enumMemberVar->AsEnumVariable()->SetVisited();
            }
            if (enumMemberVar->AsEnumVariable()->IsVisited() && enumMemberVar->AsEnumVariable()->StringInit()) {
                return true;
            }

            return false;
        }
        case binder::VariableFlags::NAMESPACE: {
            auto *exportTSBindings = currVar->AsNamespaceVariable()->GetExportBindings();
            if (exportTSBindings != nullptr) {
                ArenaVector<binder::Variable *> findRes(Allocator()->Adapter());
                util::StringView name = GetNameForMemberExpressionItem(members.front());
                binder::Variable *v = exportTSBindings->FindExportTSVariable<binder::TSBindingType::NAMESPACE>(name);
                if (v != nullptr && IsInstantiatedNamespaceVariable(v)) {
                    findRes.push_back(v);
                }
                v = exportTSBindings->FindExportTSVariable<binder::TSBindingType::ENUMLITERAL>(name);
                if (v != nullptr) {
                    findRes.push_back(v);
                }
                v = exportTSBindings->FindExportTSVariable<binder::TSBindingType::IMPORT_EQUALS>(name);
                if (v != nullptr) {
                    findRes.push_back(v);
                }
                members.pop_front();

                for (auto itemVar : findRes) {
                    if (VerifyMemberExpressionDeque(itemVar, members)) {
                        return true;
                    }
                }
                return false;
            }
            return false;
        }
        case binder::VariableFlags::IMPORT_EQUALS: {
            // Replace import_equal
            auto *node = currVar->Declaration()->Node()->Parent()->AsTSImportEqualsDeclaration()->ModuleReference();
            while (node->IsTSQualifiedName()) {
                members.push_front(node->AsTSQualifiedName()->Right()->AsIdentifier());
                node = node->AsTSQualifiedName()->Left();
            }
            members.push_front(node->AsIdentifier());

            ArenaVector<binder::Variable *> findRes = FindFrontIdentifierTSVariables(
                members.front()->AsIdentifier(), currVar->AsImportEqualsVariable()->GetScope());
            members.pop_front();

            for (auto itemVar : findRes) {
                if (VerifyMemberExpressionDeque(itemVar, members)) {
                    return true;
                }
            }
            return false;
        }
        default:
            return false;
    }

    return false;
}

util::StringView Transformer::GetNameForMemberExpressionItem(const ir::Expression *node) const
{
    util::StringView name {};
    if (node->IsIdentifier()) {
        name = node->AsIdentifier()->Name();
    } else if (node->IsStringLiteral()) {
        name = node->AsStringLiteral()->Str();
    } else if (node->IsTemplateLiteral()) {
        name = node->AsTemplateLiteral()->Quasis().front()->Raw();
    }
    return name;
}

util::StringView Transformer::GetNameFromEnumMember(const ir::TSEnumMember *node) const
{
    util::StringView name {};
    if (node->Key()->IsIdentifier()) {
        name = node->Key()->AsIdentifier()->Name();
    } else if (node->Key()->IsStringLiteral()) {
        name = node->Key()->AsStringLiteral()->Str();
    }
    return name;
}

binder::Scope *Transformer::FindEnumMemberScope(const util::StringView name) const
{
    // Transform is required only if ident is an enumMember.
    auto scope = Scope();
    while (scope != nullptr) {
        if (scope->InLocalTSBindings(name)) {
            return nullptr;
        }
        if (scope->IsTSModuleScope() && scope->AsTSModuleScope()->InExportBindings(name)) {
            return nullptr;
        }
        if (scope->IsTSEnumScope() && scope->AsTSEnumScope()->FindEnumMemberVariable(name)) {
            return scope;
        }
        if (scope->FindLocal(name)) {
            return nullptr;
        }

        if (scope->IsTSModuleScope() || scope->IsTSEnumScope()) {
            scope = scope->Parent();
        }
        scope = scope->Parent();
    }

    return nullptr;
}

ir::MemberExpression *Transformer::CreateMemberExpressionFromIdentifier(binder::Scope *scope, ir::Identifier *node)
{
    auto identName = node->Name();
    auto moduleName = scope->IsTSEnumScope() ? FindTSEnumNameByScope(scope) : FindTSModuleNameByScope(scope);
    auto *id = CreateReferenceIdentifier(moduleName);
    auto *res = AllocNode<ir::MemberExpression>(id, AllocNode<ir::Identifier>(identName),
                                                ir::MemberExpression::MemberExpressionKind::PROPERTY_ACCESS,
                                                false, false);
    SetOriginalNode(res, node);
    return res;
}

void Transformer::CheckTransformedAstStructure(const Program *program) const
{
    bool passed = true;
    CheckTransformedAstNodes(program->Ast(), &passed);
    if (passed) {
        std::cout << "Transformed AST structure check passed." << std::endl;
    }
}

void Transformer::CheckTransformedAstNodes(const ir::AstNode *parent, bool *passed) const
{
    parent->Iterate([this, parent, passed](auto *childNode) { CheckTransformedAstNode(parent, childNode, passed); });
}

void Transformer::CheckTransformedAstNode(const ir::AstNode *parent, ir::AstNode *childNode, bool *passed) const
{
    if (!(*passed)) {
        return;
    }
    if (childNode->IsClassProperty() && childNode->AsClassProperty()->IsStatic()) {
        return;
    }
    if (childNode->IsDecorator()) {
        return;
    }
    if (childNode->Parent() != parent) {
        std::cout << "Illegal ast structure after transforme." << std::endl;
        *passed = false;
        return;
    }
    CheckTransformedAstNodes(childNode, passed);
}

void Transformer::ResetParentScopeForAstNodes(const ir::AstNode *parent) const
{
    parent->Iterate([this](auto *childNode) { ResetParentScopeForAstNode(childNode); });
}

void Transformer::ResetParentScopeForAstNode(ir::AstNode *childNode) const
{
    switch (childNode->Type()) {
        case ir::AstNodeType::SCRIPT_FUNCTION: {
            auto scope = childNode->AsScriptFunction()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::CATCH_CLAUSE: {
            auto scope = childNode->AsCatchClause()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::CLASS_DEFINITION: {
            auto scope = childNode->AsClassDefinition()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::BLOCK_STATEMENT: {
            auto scope = childNode->AsBlockStatement()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::DO_WHILE_STATEMENT: {
            auto scope = childNode->AsDoWhileStatement()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::WHILE_STATEMENT: {
            auto scope = childNode->AsWhileStatement()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::FOR_IN_STATEMENT: {
            auto scope = childNode->AsForInStatement()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::FOR_OF_STATEMENT: {
            auto scope = childNode->AsForOfStatement()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::FOR_UPDATE_STATEMENT: {
            auto scope = childNode->AsForUpdateStatement()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::SWITCH_STATEMENT: {
            auto scope = childNode->AsSwitchStatement()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::TS_ENUM_DECLARATION: {
            auto scope = childNode->AsTSEnumDeclaration()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::TS_INTERFACE_DECLARATION: {
            auto scope = childNode->AsTSInterfaceDeclaration()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::TS_METHOD_SIGNATURE: {
            auto scope = childNode->AsTSMethodSignature()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::TS_MODULE_DECLARATION: {
            auto scope = childNode->AsTSModuleDeclaration()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::TS_SIGNATURE_DECLARATION: {
            auto scope = childNode->AsTSSignatureDeclaration()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::TS_TYPE_PARAMETER_DECLARATION: {
            auto scope = childNode->AsTSTypeParameterDeclaration()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::TS_CONSTRUCTOR_TYPE: {
            auto scope = childNode->AsTSConstructorType()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        case ir::AstNodeType::TS_FUNCTION_TYPE: {
            auto scope = childNode->AsTSFunctionType()->Scope();
            ASSERT(scope != nullptr);
            scope->SetParent(Scope());
            break;
        }
        default: {
            ResetParentScopeForAstNodes(childNode);
            break;
        }
    }
}

}  // namespace panda::es2panda::parser
