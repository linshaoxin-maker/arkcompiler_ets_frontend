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

#include <util/helpers.h>
#include <binder/tsBinding.h>
#include <ir/astNode.h>
#include <ir/base/catchClause.h>
#include <ir/base/classDefinition.h>
#include <ir/base/decorator.h>
#include <ir/base/scriptFunction.h>
#include <ir/expression.h>
#include <ir/expressions/arrayExpression.h>
#include <ir/expressions/assignmentExpression.h>
#include <ir/expressions/binaryExpression.h>
#include <ir/expressions/conditionalExpression.h>
#include <ir/expressions/literals/stringLiteral.h>
#include <ir/expressions/objectExpression.h>
#include <ir/expressions/sequenceExpression.h>
#include <ir/module/exportAllDeclaration.h>
#include <ir/module/exportDefaultDeclaration.h>
#include <ir/module/exportNamedDeclaration.h>
#include <ir/module/exportSpecifier.h>
#include <ir/module/importDeclaration.h>
#include <ir/module/importDefaultSpecifier.h>
#include <ir/module/importNamespaceSpecifier.h>
#include <ir/module/importSpecifier.h>
#include <ir/statements/blockStatement.h>
#include <ir/statements/breakStatement.h>
#include <ir/statements/classDeclaration.h>
#include <ir/statements/continueStatement.h>
#include <ir/statements/debuggerStatement.h>
#include <ir/statements/doWhileStatement.h>
#include <ir/statements/emptyStatement.h>
#include <ir/statements/expressionStatement.h>
#include <ir/statements/forInStatement.h>
#include <ir/statements/forOfStatement.h>
#include <ir/statements/forUpdateStatement.h>
#include <ir/statements/functionDeclaration.h>
#include <ir/statements/ifStatement.h>
#include <ir/statements/labelledStatement.h>
#include <ir/statements/returnStatement.h>
#include <ir/statements/switchCaseStatement.h>
#include <ir/statements/switchStatement.h>
#include <ir/statements/throwStatement.h>
#include <ir/statements/tryStatement.h>
#include <ir/statements/variableDeclaration.h>
#include <ir/statements/variableDeclarator.h>
#include <ir/statements/whileStatement.h>
#include <ir/ts/tsEnumDeclaration.h>
#include <ir/ts/tsExternalModuleReference.h>
#include <ir/ts/tsImportEqualsDeclaration.h>
#include <ir/ts/tsInterfaceBody.h>
#include <ir/ts/tsInterfaceDeclaration.h>
#include <ir/ts/tsInterfaceHeritage.h>
#include <ir/ts/tsModuleBlock.h>
#include <ir/ts/tsModuleDeclaration.h>
#include <ir/ts/tsNamespaceExportDeclaration.h>
#include <ir/ts/tsTypeAliasDeclaration.h>
#include <ir/ts/tsTypeParameter.h>
#include <ir/ts/tsTypeParameterDeclaration.h>
#include <ir/ts/tsTypeParameterInstantiation.h>
#include <ir/ts/tsTypeReference.h>
#include <lexer/lexer.h>
#include <lexer/token/letters.h>
#include <lexer/token/sourceLocation.h>
#include <util/ustring.h>

#include <tuple>

#include "parserImpl.h"

namespace panda::es2panda::parser {

bool ParserImpl::CheckDeclare()
{
    ASSERT(lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_DECLARE);

    const auto startPos = lexer_->Save();
    lexer_->NextToken();  // eat 'declare'
    if (lexer_->GetToken().NewLine()) {
        lexer_->Rewind(startPos);
        return false;
    }
    switch (lexer_->GetToken().Type()) {
        case lexer::TokenType::KEYW_VAR:
        case lexer::TokenType::KEYW_LET:
        case lexer::TokenType::KEYW_CONST:
        case lexer::TokenType::KEYW_FUNCTION:
        case lexer::TokenType::KEYW_CLASS: {
            break;
        }
        case lexer::TokenType::LITERAL_IDENT: {
            if (lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_TYPE ||
                lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_MODULE ||
                lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_GLOBAL ||
                lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_NAMESPACE ||
                lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_ENUM ||
                lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_ABSTRACT ||
                lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_INTERFACE) {
                break;
            }

            [[fallthrough]];
        }
        default: {
            lexer_->Rewind(startPos);
            return false;
        }
    }

    if (context_.Status() & ParserStatus::IN_AMBIENT_CONTEXT) {
        lexer_->Rewind(startPos);
        ThrowSyntaxError("A 'declare' modifier cannot be used in an already ambient context.");
    }

    return true;
}

bool ParserImpl::IsLabelFollowedByIterationStatement()
{
    lexer_->NextToken();

    switch (lexer_->GetToken().Type()) {
        case lexer::TokenType::KEYW_DO:
        case lexer::TokenType::KEYW_FOR:
        case lexer::TokenType::KEYW_WHILE: {
            return true;
        }
        case lexer::TokenType::LITERAL_IDENT: {
            if (lexer_->Lookahead() == LEX_CHAR_COLON) {
                lexer_->NextToken();
                return IsLabelFollowedByIterationStatement();
            }

            [[fallthrough]];
        }
        default:
            return false;
    }
    return false;
}

bool ParserImpl::IsTsDeclarationStatement() const
{
    const auto startPos = lexer_->Save();
    bool isTsDeclarationStatement = false;

    auto keywordType = lexer_->GetToken().KeywordType();
    lexer_->NextToken();
    if (lexer_->GetToken().Type() == lexer::TokenType::KEYW_AWAIT) {
        lexer_->GetToken().SetTokenType(lexer::TokenType::LITERAL_IDENT);
    }
    switch (keywordType) {
        case lexer::TokenType::KEYW_MODULE:
        case lexer::TokenType::KEYW_NAMESPACE: {
            isTsDeclarationStatement = !lexer_->GetToken().NewLine() &&
                (lexer_->GetToken().Type() == lexer::TokenType::LITERAL_IDENT ||
                lexer_->GetToken().Type() == lexer::TokenType::LITERAL_STRING);
            break;
        }
        case lexer::TokenType::KEYW_GLOBAL: {
            isTsDeclarationStatement = lexer_->GetToken().Type() == lexer::TokenType::LITERAL_IDENT ||
                lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE ||
                lexer_->GetToken().Type() == lexer::TokenType::KEYW_EXPORT;
            break;
        }
        case lexer::TokenType::KEYW_INTERFACE:
        case lexer::TokenType::KEYW_TYPE: {
            isTsDeclarationStatement = !lexer_->GetToken().NewLine() &&
                lexer_->GetToken().Type() == lexer::TokenType::LITERAL_IDENT;
            break;
        }
        case lexer::TokenType::KEYW_ENUM: {
            isTsDeclarationStatement = true;
            break;
        }
        default: {
            break;
        }
    }

    lexer_->Rewind(startPos);
    return isTsDeclarationStatement;
}

ir::Statement *ParserImpl::ParseStatement(StatementParsingFlags flags)
{
    bool isDeclare = false;
    auto decorators = ParseDecorators();

    if (Extension() == ScriptExtension::TS) {
        if (lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_DECLARE) {
            isDeclare = CheckDeclare();
        }

        if (lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_ABSTRACT) {
            const auto startPos = lexer_->Save();
            lexer_->NextToken();  // eat abstract keyword

            if (lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_DECLARE) {
                if (isDeclare) {
                    ThrowSyntaxError("'declare' modifier already seen.");
                }
                lexer_->NextToken();
                isDeclare = true;
            }

            if (lexer_->GetToken().Type() != lexer::TokenType::KEYW_CLASS) {
                lexer_->Rewind(startPos);
            } else {
                return ParseClassStatement(flags, isDeclare, std::move(decorators), true);
            }
        }

        if ((lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_GLOBAL ||
             lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_MODULE ||
             lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_NAMESPACE) &&
             IsTsDeclarationStatement()) {
            auto savedStatus = context_.Status();
            if (isDeclare) {
                context_.Status() |= ParserStatus::IN_AMBIENT_CONTEXT;
            }
            ir::TSModuleDeclaration *gdecl = ParseTsModuleDeclaration(isDeclare);
            context_.Status() = savedStatus;
            return gdecl;
        }
    }
    ParseStatementHelper();
}

ir::Statement *ParserImpl::ParseStatementHelper()
{
    switch (lexer_->GetToken().Type()) {
        case lexer::TokenType::PUNCTUATOR_LEFT_BRACE: {
            return ParseBlockStatement();
            }
        case lexer::TokenType::PUNCTUATOR_SEMI_COLON: {
            return ParseEmptyStatement();
        }
        case lexer::TokenType::KEYW_EXPORT: {
            return ParseExportDeclaration(flags, std::move(decorators));
        }
        case lexer::TokenType::KEYW_IMPORT: {
            return ParseImportDeclaration(flags);
        }
        case lexer::TokenType::KEYW_FUNCTION: {
            return ParseFunctionStatement(flags, isDeclare);
        }
        case lexer::TokenType::KEYW_CLASS: {
            return ParseClassStatement(flags, isDeclare, std::move(decorators));
        }
        case lexer::TokenType::KEYW_VAR: {
            return ParseVarStatement(isDeclare);
        }
        case lexer::TokenType::KEYW_LET: {
            return ParseLetStatement(flags, isDeclare);
        }
        case lexer::TokenType::KEYW_CONST: {
            return ParseConstStatement(flags, isDeclare);
        }
        case lexer::TokenType::KEYW_THROW: {
            return ParseThrowStatement();
        }
        case lexer::TokenType::KEYW_RETURN: {
            return ParseReturnStatement();
        }
        case lexer::TokenType::KEYW_SWITCH: {
            return ParseSwitchStatement();
        }
        case lexer::TokenType::KEYW_DEBUGGER: {
            return ParseDebuggerStatement();
        }
        case lexer::TokenType::LITERAL_IDENT: {
            return ParsePotentialExpressionStatement(flags, isDeclare);
        }
        default: {
            ParseStatementControlStructure()
        }
    }
}

ir::Statement *ParserImpl::ParseStatementControlStructure()
{
    switch (lexer_->GetToken().Type()) {
        case lexer::TokenType::KEYW_IF: {
            return ParseIfStatement();
        }
        case lexer::TokenType::KEYW_DO: {
            return ParseDoWhileStatement();
        }
        case lexer::TokenType::KEYW_FOR: {
            return ParseForStatement();
        }
        case lexer::TokenType::KEYW_TRY: {
            return ParseTryStatement();
        }
        case lexer::TokenType::KEYW_WHILE: {
            return ParseWhileStatement();
        }
        case lexer::TokenType::KEYW_BREAK: {
            return ParseBreakStatement();
        }
        case lexer::TokenType::KEYW_CONTINUE: {
            return ParseContinueStatement();
        }
        default: {
            break;
        }
    }

    return ParseExpressionStatement(flags);
}

ir::TSModuleDeclaration *ParserImpl::ParseTsModuleDeclaration(bool isDeclare, bool isExport)
{
    lexer::SourcePosition startLoc = lexer_->GetToken().Start();
    context_.Status() |= ParserStatus::TS_MODULE;

    if (lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_GLOBAL) {
        return ParseTsAmbientExternalModuleDeclaration(startLoc, isDeclare);
    }

    if (lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_NAMESPACE) {
        lexer_->NextToken();
        if (lexer_->GetToken().Type() == lexer::TokenType::KEYW_AWAIT) {
            lexer_->GetToken().SetTokenType(lexer::TokenType::LITERAL_IDENT);
        }
    } else {
        ASSERT(lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_MODULE);
        lexer_->NextToken();
        if (lexer_->GetToken().Type() == lexer::TokenType::KEYW_AWAIT) {
            lexer_->GetToken().SetTokenType(lexer::TokenType::LITERAL_IDENT);
        }
        if (lexer_->GetToken().Type() == lexer::TokenType::LITERAL_STRING) {
            return ParseTsAmbientExternalModuleDeclaration(startLoc, isDeclare);
        }
    }

    return ParseTsModuleOrNamespaceDelaration(startLoc, isDeclare, isExport);
}

ir::TSModuleDeclaration *ParserImpl::ParseTsAmbientExternalModuleDeclaration(const lexer::SourcePosition &startLoc,
                                                                             bool isDeclare)
{
    bool isGlobal = false;
    ir::Expression *name = nullptr;

    if (lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_GLOBAL) {
        isGlobal = true;
        name = AllocNode<ir::Identifier>(lexer_->GetToken().Ident());
    } else {
        ASSERT(lexer_->GetToken().Type() == lexer::TokenType::LITERAL_STRING);

        if (!isDeclare && !(context_.Status() & ParserStatus::IN_AMBIENT_CONTEXT)) {
            ThrowSyntaxError("Only ambient modules can use quoted names");
        }

        name = AllocNode<ir::StringLiteral>(lexer_->GetToken().String());
    }

    name->SetRange(lexer_->GetToken().Loc());

    lexer_->NextToken();

    binder::ExportBindings *exportBindings = Allocator()->New<binder::ExportBindings>(Allocator());
    auto localCtx = binder::LexicalScope<binder::TSModuleScope>(Binder(), exportBindings);

    ir::Statement *body = nullptr;
    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        body = ParseTsModuleBlock();
    } else if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
        lexer_->NextToken();
    } else if (!lexer_->GetToken().NewLine()) {
        ThrowSyntaxError("';' expected");
    }

    auto *moduleDecl = AllocNode<ir::TSModuleDeclaration>(localCtx.GetScope(), name, body, isDeclare, isGlobal);
    moduleDecl->SetRange({startLoc, lexer_->GetToken().End()});
    localCtx.GetScope()->BindNode(moduleDecl);

    return moduleDecl;
}

ir::TSModuleDeclaration *ParserImpl::ParseTsModuleOrNamespaceDelaration(const lexer::SourcePosition &startLoc,
                                                                        bool isDeclare, bool isExport)
{
    if (lexer_->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
        ThrowSyntaxError("Identifier expected");
    }

    auto name = lexer_->GetToken().Ident();
    auto *parentScope = Binder()->GetScope();
    binder::Variable *res = parentScope->FindLocalTSVariable<binder::TSBindingType::NAMESPACE>(name);
    if (!res && isExport && parentScope->IsTSModuleScope()) {
        res = parentScope->AsTSModuleScope()->FindExportTSVariable<binder::TSBindingType::NAMESPACE>(name);
        if (res != nullptr) {
            parentScope->AddLocalTSVariable<binder::TSBindingType::NAMESPACE>(name, res);
        }
    }
    if (res == nullptr) {
        Binder()->AddTsDecl<binder::NamespaceDecl>(lexer_->GetToken().Start(), isDeclare, Allocator(), name);
        res = parentScope->FindLocalTSVariable<binder::TSBindingType::NAMESPACE>(name);
        if (isExport && parentScope->IsTSModuleScope()) {
            parentScope->AsTSModuleScope()->AddExportTSVariable<binder::TSBindingType::NAMESPACE>(name, res);
        }
        res->AsNamespaceVariable()->SetExportBindings(Allocator()->New<binder::ExportBindings>(Allocator()));
    }
    binder::ExportBindings *exportBindings = res->AsNamespaceVariable()->GetExportBindings();

    auto *identNode = AllocNode<ir::Identifier>(name);
    identNode->SetRange(lexer_->GetToken().Loc());

    lexer_->NextToken();

    ir::Statement *body = nullptr;

    auto savedStatus = context_.Status();
    if (isDeclare) {
        context_.Status() |= ParserStatus::IN_AMBIENT_CONTEXT;
    }

    auto localCtx = binder::LexicalScope<binder::TSModuleScope>(Binder(), exportBindings);

    bool isInstantiated = false;
    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_PERIOD) {
        lexer_->NextToken();
        lexer::SourcePosition moduleStart = lexer_->GetToken().Start();
        body = ParseTsModuleOrNamespaceDelaration(moduleStart, false, true);
        isInstantiated = body->AsTSModuleDeclaration()->IsInstantiated();
    } else {
        body = ParseTsModuleBlock();
        auto statements = body->AsTSModuleBlock()->Statements();
        for (auto *it : statements) {
            auto statement = it;
            if (statement->IsExportNamedDeclaration()) {
                statement = statement->AsExportNamedDeclaration()->Decl();
            }
            if (statement != nullptr &&
                !statement->IsTSInterfaceDeclaration() && !statement->IsTSTypeAliasDeclaration() &&
                (!statement->IsTSModuleDeclaration() || statement->AsTSModuleDeclaration()->IsInstantiated())) {
                isInstantiated = true;
                break;
            }
        }
    }
    if (isDeclare) {
        isInstantiated = false;
    }

    context_.Status() = savedStatus;

    auto *moduleDecl = AllocNode<ir::TSModuleDeclaration>(localCtx.GetScope(), identNode, body,
                                                          isDeclare, false, isInstantiated);
    moduleDecl->SetRange({startLoc, lexer_->GetToken().End()});
    localCtx.GetScope()->BindNode(moduleDecl);
    res->Declaration()->AsNamespaceDecl()->Add(moduleDecl);

    return moduleDecl;
}

ir::TSImportEqualsDeclaration *ParserImpl::ParseTsImportEqualsDeclaration(const lexer::SourcePosition &startLoc,
                                                                          bool isExport)
{
    ASSERT(lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_IMPORT);
    lexer_->NextToken();
    if (lexer_->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
        ThrowSyntaxError("Unexpected token");
    }

    auto *id = AllocNode<ir::Identifier>(lexer_->GetToken().Ident());
    id->SetRange(lexer_->GetToken().Loc());
    lexer_->NextToken();  // eat id name

    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        ThrowSyntaxError("'=' expected");
    }
    lexer_->NextToken();  // eat substitution

    if (lexer_->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
        ThrowSyntaxError("identifier expected");
    }

    if (lexer_->GetToken().KeywordType() != lexer::TokenType::KEYW_REQUIRE ||
        lexer_->Lookahead() != LEX_CHAR_LEFT_PAREN) {
        binder::DeclarationFlags declflag = binder::DeclarationFlags::NONE;
        auto *decl = Binder()->AddDecl<binder::ImportEqualsDecl>(id->Start(), declflag, false, id->Name());
        decl->BindNode(id);
        auto *scope = Binder()->GetScope();
        auto name = id->Name();
        auto *var = scope->FindLocalTSVariable<binder::TSBindingType::IMPORT_EQUALS>(name);
        ASSERT(var != nullptr);
        var->AsImportEqualsVariable()->SetScope(scope);
        if (isExport && scope->IsTSModuleScope()) {
            scope->AsTSModuleScope()->AddExportTSVariable<binder::TSBindingType::IMPORT_EQUALS>(name, var);
        }
    }

    auto *importEqualsDecl = AllocNode<ir::TSImportEqualsDeclaration>(id, ParseModuleReference(), isExport);
    importEqualsDecl->SetRange({startLoc, lexer_->GetToken().End()});

    ConsumeSemicolon(importEqualsDecl);

    return importEqualsDecl;
}

ir::TSNamespaceExportDeclaration *ParserImpl::ParseTsNamespaceExportDeclaration(const lexer::SourcePosition &startLoc)
{
    if (!IsDtsFile()) {
        ThrowSyntaxError("namespace export declaration is only supported in TypeScript '.d.ts'");
    }
    ASSERT(lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_AS);
    lexer_->NextToken();  // eat as keyword
    if (lexer_->GetToken().KeywordType() != lexer::TokenType::KEYW_NAMESPACE) {
        ThrowSyntaxError("'namespace' expected");
    }
    lexer_->NextToken();  // eat namespace keyword
    if (lexer_->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
        ThrowSyntaxError("identifier expected");
    }

    auto *id = AllocNode<ir::Identifier>(lexer_->GetToken().Ident());
    id->SetRange(lexer_->GetToken().Loc());
    lexer_->NextToken();  // eat identifier

    auto *namespaceExportDecl = AllocNode<ir::TSNamespaceExportDeclaration>(id);
    namespaceExportDecl->SetRange({startLoc, lexer_->GetToken().End()});

    ConsumeSemicolon(namespaceExportDecl);

    return namespaceExportDecl;
}

ir::TSModuleBlock *ParserImpl::ParseTsModuleBlock()
{
    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        ThrowSyntaxError("'{' expected.");
    }

    lexer::SourcePosition startLoc = lexer_->GetToken().Start();
    lexer_->NextToken();
    auto statements = ParseStatementList();

    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        ThrowSyntaxError("Expected a '}'");
    }

    auto *blockNode = AllocNode<ir::TSModuleBlock>(std::move(statements));
    blockNode->SetRange({startLoc, lexer_->GetToken().End()});

    lexer_->NextToken();
    return blockNode;
}

ir::Statement *ParserImpl::ParseVarStatement(bool isDeclare)
{
    auto *variableDecl = ParseVariableDeclaration(VariableParsingFlags::VAR, isDeclare);
    ConsumeSemicolon(variableDecl);
    return variableDecl;
}

ir::Statement *ParserImpl::ParseLetStatement(StatementParsingFlags flags, bool isDeclare)
{
    if (!(flags & StatementParsingFlags::ALLOW_LEXICAL)) {
        ThrowSyntaxError("The 'let' declarations can only be declared at the top level or inside a block.");
    }

    auto *variableDecl = ParseVariableDeclaration(VariableParsingFlags::LET, isDeclare);
    ConsumeSemicolon(variableDecl);
    return variableDecl;
}

ir::Statement *ParserImpl::ParseConstStatement(StatementParsingFlags flags, bool isDeclare)
{
    lexer::SourcePosition constVarStar = lexer_->GetToken().Start();
    lexer_->NextToken();

    if (lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_ENUM) {
        if (Extension() == ScriptExtension::TS) {
            return ParseEnumDeclaration(false, isDeclare, true);
        }
        ThrowSyntaxError("Unexpected token");
    }

    if (!(flags & StatementParsingFlags::ALLOW_LEXICAL)) {
        ThrowSyntaxError("The 'const' declarations can only be declared at the top level or inside a block.");
    }

    auto *variableDecl =
        ParseVariableDeclaration(VariableParsingFlags::CONST | VariableParsingFlags::NO_SKIP_VAR_KIND, isDeclare);
    variableDecl->SetStart(constVarStar);
    ConsumeSemicolon(variableDecl);

    return variableDecl;
}

ir::EmptyStatement *ParserImpl::ParseEmptyStatement()
{
    auto *empty = AllocNode<ir::EmptyStatement>();
    empty->SetRange(lexer_->GetToken().Loc());
    lexer_->NextToken();
    return empty;
}

ir::DebuggerStatement *ParserImpl::ParseDebuggerStatement()
{
    auto *debuggerNode = AllocNode<ir::DebuggerStatement>();
    debuggerNode->SetRange(lexer_->GetToken().Loc());
    lexer_->NextToken();
    ConsumeSemicolon(debuggerNode);
    return debuggerNode;
}

ir::Statement *ParserImpl::ParseFunctionStatement(StatementParsingFlags flags, bool isDeclare)
{
    CheckFunctionDeclaration(flags);

    if (!(flags & StatementParsingFlags::STMT_LEXICAL_SCOPE_NEEDED)) {
        return ParseFunctionDeclaration(false, ParserStatus::NO_OPTS, isDeclare);
    }

    auto localCtx = binder::LexicalScope<binder::LocalScope>(Binder());
    ArenaVector<ir::Statement *> stmts(Allocator()->Adapter());
    auto *funcDecl = ParseFunctionDeclaration(false, ParserStatus::NO_OPTS, isDeclare);
    stmts.push_back(funcDecl);

    auto *localBlockStmt = AllocNode<ir::BlockStatement>(localCtx.GetScope(), std::move(stmts));
    localBlockStmt->SetRange(funcDecl->Range());
    localCtx.GetScope()->BindNode(localBlockStmt);

    return funcDecl;
}

ir::Statement *ParserImpl::ParsePotentialExpressionStatement(StatementParsingFlags flags, bool isDeclare)
{
    if (lexer_->Lookahead() == LEX_CHAR_COLON) {
        const auto pos = lexer_->Save();
        lexer_->NextToken();
        return ParseLabelledStatement(pos);
    }

    if (Extension() == ScriptExtension::TS && IsTsDeclarationStatement()) {
        switch (lexer_->GetToken().KeywordType()) {
            case lexer::TokenType::KEYW_ENUM: {
                return ParseEnumDeclaration(false, isDeclare, false);
            }
            case lexer::TokenType::KEYW_TYPE: {
                return ParseTsTypeAliasDeclaration(isDeclare);
            }
            case lexer::TokenType::KEYW_INTERFACE: {
                return ParseTsInterfaceDeclaration(isDeclare);
            }
            default:
                break;
        }
    }

    return ParseExpressionStatement(flags);
}

ir::ClassDeclaration *ParserImpl::ParseClassStatement(StatementParsingFlags flags, bool isDeclare,
                                                      ArenaVector<ir::Decorator *> &&decorators, bool isAbstract)
{
    if (!(flags & StatementParsingFlags::ALLOW_LEXICAL)) {
        ThrowSyntaxError("Lexical 'class' declaration is not allowed in single statement context");
    }

    return ParseClassDeclaration(true, std::move(decorators), isDeclare, isAbstract);
}

ir::ClassDeclaration *ParserImpl::ParseClassDeclaration(bool idRequired, ArenaVector<ir::Decorator *> &&decorators,
                                                        bool isDeclare, bool isAbstract, bool isExported)
{
    lexer::SourcePosition startLoc = lexer_->GetToken().Start();
    ir::ClassDefinition *classDefinition = ParseClassDefinition(true, idRequired, isDeclare, isAbstract);
    if (isExported && !idRequired) {
        classDefinition->SetAsExportDefault();
    }

    if (!decorators.empty()) {
        classDefinition->SetClassDecoratorPresent();
    }

    auto location = classDefinition->Ident() ? classDefinition->Ident()->Start() : startLoc;
    auto className = classDefinition->GetName();
    ASSERT(!className.Empty());

    binder::DeclarationFlags flag = isExported ? binder::DeclarationFlags::EXPORT : binder::DeclarationFlags::NONE;
    auto *decl = Binder()->AddDecl<binder::ClassDecl>(location, flag, classDefinition->Declare(), className);

    decl->BindNode(classDefinition);

    lexer::SourcePosition endLoc = classDefinition->End();
    auto *classDecl = AllocNode<ir::ClassDeclaration>(classDefinition, std::move(decorators));
    classDecl->SetRange({startLoc, endLoc});
    return classDecl;
}

ir::TSTypeAliasDeclaration *ParserImpl::ParseTsTypeAliasDeclaration(bool isDeclare)
{
    ASSERT(lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_TYPE);
    lexer::SourcePosition typeStart = lexer_->GetToken().Start();
    lexer_->NextToken();  // eat type keyword

    if (lexer_->GetToken().Type() != lexer::TokenType::LITERAL_IDENT &&
        !(lexer_->GetToken().Type() == lexer::TokenType::KEYW_AWAIT && isDeclare)) {
        ThrowSyntaxError("Identifier expected");
    }

    if (lexer_->GetToken().IsReservedTypeName()) {
        std::string errMsg("Type alias name cannot be '");
        errMsg.append(TokenToString(lexer_->GetToken().KeywordType()));
        errMsg.append("'");
        ThrowSyntaxError(errMsg.c_str());
    }

    const util::StringView &ident = lexer_->GetToken().Ident();
    binder::TSBinding tsBinding(Allocator(), ident);
    auto *decl = Binder()->AddTsDecl<binder::TypeAliasDecl>(lexer_->GetToken().Start(), isDeclare, tsBinding.View());

    auto *id = AllocNode<ir::Identifier>(ident);
    id->SetRange(lexer_->GetToken().Loc());
    lexer_->NextToken();

    ir::TSTypeParameterDeclaration *typeParamDecl = nullptr;
    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        typeParamDecl = ParseTsTypeParameterDeclaration(true, true);
    }

    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        ThrowSyntaxError("'=' expected");
    }

    lexer_->NextToken();  // eat '='

    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
    ir::Expression *typeAnnotation = ParseTsTypeAnnotation(&options);

    auto *typeAliasDecl = AllocNode<ir::TSTypeAliasDeclaration>(id, typeParamDecl, typeAnnotation, isDeclare);
    typeAliasDecl->SetRange({typeStart, lexer_->GetToken().End()});
    decl->BindNode(typeAliasDecl);
    ConsumeSemicolon(typeAliasDecl);

    return typeAliasDecl;
}

ir::TSInterfaceDeclaration *ParserImpl::ParseTsInterfaceDeclaration(bool isDeclare)
{
    ASSERT(lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_INTERFACE);
    context_.Status() |= ParserStatus::ALLOW_THIS_TYPE;
    lexer::SourcePosition interfaceStart = lexer_->GetToken().Start();
    lexer_->NextToken();  // eat interface keyword

    ValidateTsInterfaceName(isDeclare);

    const util::StringView &ident = lexer_->GetToken().Ident();
    binder::TSBinding tsBinding(Allocator(), ident);

    const auto &bindings = Binder()->GetScope()->Bindings();
    auto res = bindings.find(tsBinding.View());
    binder::InterfaceDecl *decl {};

    if (res == bindings.end()) {
        decl = Binder()->AddTsDecl<binder::InterfaceDecl>(lexer_->GetToken().Start(), isDeclare,
                                                          Allocator(), tsBinding.View());
    } else if (!res->second->Declaration()->IsInterfaceDecl()) {
        Binder()->ThrowRedeclaration(lexer_->GetToken().Start(), ident);
    } else {
        decl = res->second->Declaration()->AsInterfaceDecl();
    }

    auto *id = AllocNode<ir::Identifier>(lexer_->GetToken().Ident());
    id->SetRange(lexer_->GetToken().Loc());
    id->SetReference();
    lexer_->NextToken();

    binder::LexicalScope<binder::LocalScope> localScope(Binder());

    ir::TSTypeParameterDeclaration *typeParamDecl = nullptr;
    if (Extension() == ScriptExtension::TS && lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        typeParamDecl = ParseTsTypeParameterDeclaration(true, true);
    }

    ArenaVector<ir::TSInterfaceHeritage *> extends = ParseTsInterfaceExtends();

    lexer::SourcePosition bodyStart = lexer_->GetToken().Start();
    auto members = ParseTsTypeLiteralOrInterface();

    auto *body = AllocNode<ir::TSInterfaceBody>(std::move(members));
    body->SetRange({bodyStart, lexer_->GetToken().End()});

    auto *interfaceDecl =
        AllocNode<ir::TSInterfaceDeclaration>(localScope.GetScope(), id, typeParamDecl, body, std::move(extends));
    interfaceDecl->SetRange({interfaceStart, lexer_->GetToken().End()});

    ASSERT(decl);

    if (res == bindings.end()) {
        decl->BindNode(interfaceDecl);
    }
    decl->AsInterfaceDecl()->Add(interfaceDecl);

    lexer_->NextToken();
    context_.Status() &= ~ParserStatus::ALLOW_THIS_TYPE;

    return interfaceDecl;
}

void ParserImpl::ValidateTsInterfaceName(bool isDeclare)
{
    if (lexer_->GetToken().Type() != lexer::TokenType::LITERAL_IDENT &&
        !(lexer_->GetToken().Type() == lexer::TokenType::KEYW_AWAIT && isDeclare)) {
        ThrowSyntaxError("Identifier expected");
    }

    if (lexer_->GetToken().IsReservedTypeName()) {
        std::string errMsg("Interface name cannot be '");
        errMsg.append(TokenToString(lexer_->GetToken().KeywordType()));
        errMsg.append("'");
        ThrowSyntaxError(errMsg.c_str());
    }
}

ArenaVector<ir::TSInterfaceHeritage *> ParserImpl::ParseTsInterfaceExtends()
{
    ArenaVector<ir::TSInterfaceHeritage *> extends(Allocator()->Adapter());

    if (lexer_->GetToken().KeywordType() != lexer::TokenType::KEYW_EXTENDS) {
        return extends;
    }

    lexer_->NextToken();  // eat extends keyword
    while (true) {
        if (lexer_->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
            ThrowSyntaxError("Identifier expected");
        }
        const lexer::SourcePosition &heritageStart = lexer_->GetToken().Start();
        lexer::SourcePosition heritageEnd = lexer_->GetToken().End();
        ir::Expression *expr = AllocNode<ir::Identifier>(lexer_->GetToken().Ident());
        expr->AsIdentifier()->SetReference();
        expr->SetRange(lexer_->GetToken().Loc());
        if (lexer_->Lookahead() == LEX_CHAR_LESS_THAN) {
            lexer_->ForwardToken(lexer::TokenType::PUNCTUATOR_LESS_THAN, 1);
        } else {
            lexer_->NextToken();
        }
        if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_PERIOD) {
            expr = ParseTsQualifiedReference(expr);
        }
        ir::TSTypeParameterInstantiation *typeParamInst = nullptr;
        if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
            typeParamInst = ParseTsTypeParameterInstantiation();
            heritageEnd = typeParamInst->End();
        }
        auto *typeReference = AllocNode<ir::TSTypeReference>(expr, typeParamInst);
        typeReference->SetRange({heritageStart, heritageEnd});
        auto *heritage = AllocNode<ir::TSInterfaceHeritage>(typeReference);
        heritage->SetRange(typeReference->Range());
        extends.push_back(heritage);
        if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
            break;
        }
        if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_COMMA) {
            ThrowSyntaxError("',' expected");
        }
        lexer_->NextToken();
    }

    return extends;
}

void ParserImpl::CheckFunctionDeclaration(StatementParsingFlags flags)
{
    if (flags & StatementParsingFlags::ALLOW_LEXICAL) {
        return;
    }

    if (lexer_->Lookahead() == LEX_CHAR_ASTERISK) {
        ThrowSyntaxError("Generators can only be declared at the top level or inside a block.");
    }

    ThrowSyntaxError(
        "In strict mode code, functions can only be "
        "declared at top level, inside a block, "
        "or "
        "as the body of an if statement");
}

void ParserImpl::ConsumeSemicolon(ir::Statement *statement)
{
    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
        statement->SetEnd(lexer_->GetToken().End());
        lexer_->NextToken();
        return;
    }

    if (!lexer_->GetToken().NewLine()) {
        if (lexer_->GetToken().Type() != lexer::TokenType::EOS &&
            lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
            ThrowSyntaxError("Unexpected token");
        }
    }
}

ArenaVector<ir::Statement *> ParserImpl::ParseStatementList(StatementParsingFlags flags)
{
    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
    ParseDirectivePrologue(&statements);

    auto endType =
        (flags & StatementParsingFlags::GLOBAL) ? lexer::TokenType::EOS : lexer::TokenType::PUNCTUATOR_RIGHT_BRACE;

    while (lexer_->GetToken().Type() != endType) {
        statements.push_back(ParseStatement(flags));
    }

    return statements;
}

bool ParserImpl::ParseDirective(ArenaVector<ir::Statement *> *statements)
{
    ASSERT(lexer_->GetToken().Type() == lexer::TokenType::LITERAL_STRING);

    const util::StringView &str = lexer_->GetToken().String();

    const auto status = static_cast<ParserStatus>(
        context_.Status() & (ParserStatus::CONSTRUCTOR_FUNCTION | ParserStatus::HAS_COMPLEX_PARAM));
    if (status == ParserStatus::HAS_COMPLEX_PARAM && str.Is("use strict")) {
        ThrowSyntaxError(
            "Illegal 'use strict' directive in function with "
            "non-simple parameter list");
    }

    ir::Expression *exprNode = ParseExpression(ExpressionParseFlags::ACCEPT_COMMA);
    bool isDirective = exprNode->IsStringLiteral();

    auto *exprStatement = AllocNode<ir::ExpressionStatement>(exprNode);
    exprStatement->SetRange(exprNode->Range());

    ConsumeSemicolon(exprStatement);
    statements->push_back(exprStatement);

    return isDirective;
}

void ParserImpl::ParseDirectivePrologue(ArenaVector<ir::Statement *> *statements)
{
    while (true) {
        if (lexer_->GetToken().Type() != lexer::TokenType::LITERAL_STRING || !ParseDirective(statements)) {
            break;
        }
    }
}

ir::BlockStatement *ParserImpl::ParseBlockStatement(binder::Scope *scope)
{
    ASSERT(lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE);

    lexer::SourcePosition startLoc = lexer_->GetToken().Start();
    lexer_->NextToken();
    auto statements = ParseStatementList();

    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        ThrowSyntaxError("Expected a '}'");
    }

    auto *blockNode = AllocNode<ir::BlockStatement>(scope, std::move(statements));
    blockNode->SetRange({startLoc, lexer_->GetToken().End()});
    scope->BindNode(blockNode);

    return blockNode;
}

ir::BlockStatement *ParserImpl::ParseBlockStatement()
{
    auto localCtx = binder::LexicalScope<binder::LocalScope>(Binder());
    auto *blockNode = ParseBlockStatement(localCtx.GetScope());
    lexer_->NextToken();
    return blockNode;
}

ir::BreakStatement *ParserImpl::ParseBreakStatement()
{
    bool allowBreak = (context_.Status() & (ParserStatus::IN_ITERATION | ParserStatus::IN_SWITCH));

    lexer::SourcePosition startLoc = lexer_->GetToken().Start();
    lexer_->NextToken();

    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON ||
        lexer_->GetToken().Type() == lexer::TokenType::EOS || lexer_->GetToken().NewLine() ||
        lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {

        if (!allowBreak && Extension() == ScriptExtension::JS) {
            ThrowSyntaxError("Illegal break statement");
        }

        if (!allowBreak && Extension() == ScriptExtension::TS) {
            if (context_.Status() & ParserStatus::FUNCTION) {
                ThrowSyntaxError("Jump target cannot cross function boundary");
            } else {
                ThrowSyntaxError(
                    "A 'break' statement can only be used within an "
                    "enclosing iteration or switch statement");
            }
        }

        auto *breakStatement = AllocNode<ir::BreakStatement>();
        breakStatement->SetRange({startLoc, lexer_->GetToken().End()});
        if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
            lexer_->NextToken();
        }

        return breakStatement;
    }

    if (lexer_->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
        ThrowSyntaxError("Unexpected token.");
    }

    const auto &label = lexer_->GetToken().Ident();

    if (!context_.FindLabel(label)) {
        ThrowSyntaxError("Undefined label");
    }

    auto *identNode = AllocNode<ir::Identifier>(label);
    identNode->SetRange(lexer_->GetToken().Loc());

    auto *breakStatement = AllocNode<ir::BreakStatement>(identNode);
    breakStatement->SetRange({startLoc, lexer_->GetToken().End()});

    lexer_->NextToken();
    ConsumeSemicolon(breakStatement);

    return breakStatement;
}

ir::ContinueStatement *ParserImpl::ParseContinueStatement()
{
    if (Extension() == ScriptExtension::TS &&
        (static_cast<ParserStatus>(context_.Status() & (ParserStatus::FUNCTION | ParserStatus::IN_ITERATION |
                                                        ParserStatus::IN_SWITCH)) == ParserStatus::FUNCTION)) {
        ThrowSyntaxError("Jump target cannot cross function boundary");
    }

    if (!(context_.Status() & ParserStatus::IN_ITERATION)) {
        if (Extension() == ScriptExtension::JS) {
            ThrowSyntaxError("Illegal continue statement");
        }
        if (Extension() == ScriptExtension::TS) {
            ThrowSyntaxError(
                "A 'continue' statement can only be used within an "
                "enclosing iteration statement");
        }
    }

    lexer::SourcePosition startLoc = lexer_->GetToken().Start();
    lexer::SourcePosition endLoc = lexer_->GetToken().End();
    lexer_->NextToken();

    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
        auto *continueStatement = AllocNode<ir::ContinueStatement>();
        continueStatement->SetRange({startLoc, lexer_->GetToken().End()});
        lexer_->NextToken();
        return continueStatement;
    }

    if (lexer_->GetToken().NewLine() || lexer_->GetToken().Type() == lexer::TokenType::EOS ||
        lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        auto *continueStatement = AllocNode<ir::ContinueStatement>();
        continueStatement->SetRange({startLoc, endLoc});
        return continueStatement;
    }

    if (lexer_->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
        ThrowSyntaxError("Unexpected token.");
    }

    const auto &label = lexer_->GetToken().Ident();
    const ParserContext *labelCtx = context_.FindLabel(label);

    if (!labelCtx || !(labelCtx->Status() & (ParserStatus::IN_ITERATION | ParserStatus::IN_LABELED)) ||
       (labelCtx->Status() & ParserStatus::DISALLOW_CONTINUE)) {
        ThrowSyntaxError("Undefined label");
    }

    auto *identNode = AllocNode<ir::Identifier>(label);
    identNode->SetRange(lexer_->GetToken().Loc());

    auto *continueStatement = AllocNode<ir::ContinueStatement>(identNode);
    continueStatement->SetRange({startLoc, lexer_->GetToken().End()});

    lexer_->NextToken();
    ConsumeSemicolon(continueStatement);

    return continueStatement;
}

ir::DoWhileStatement *ParserImpl::ParseDoWhileStatement()
{
    auto *savedScope = Binder()->GetScope();
    IterationContext<binder::LoopScope> iterCtx(&context_, Binder());

    lexer::SourcePosition startLoc = lexer_->GetToken().Start();
    lexer_->NextToken();
    ir::Statement *body = ParseStatement();

    if (lexer_->GetToken().Type() != lexer::TokenType::KEYW_WHILE) {
        ThrowSyntaxError("Missing 'while' keyword in a 'DoWhileStatement'");
    }

    lexer_->NextToken();
    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS) {
        ThrowSyntaxError("Missing left parenthesis in a 'DoWhileStatement'");
    }

    lexer_->NextToken();
    ir::Expression *test = nullptr;

    // The while expression should be included in the outer scope
    {
        auto outerScope = binder::LexicalScope<binder::Scope>::Enter(Binder(), savedScope);
        test = ParseExpression(ExpressionParseFlags::ACCEPT_COMMA);
    }

    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
        ThrowSyntaxError("Missing right parenthesis in a 'DoWhileStatement'");
    }

    auto *doWhileStatement = AllocNode<ir::DoWhileStatement>(iterCtx.LexicalScope().GetScope(), body, test);
    doWhileStatement->SetRange({startLoc, lexer_->GetToken().End()});
    iterCtx.LexicalScope().GetScope()->BindNode(doWhileStatement);

    lexer_->NextToken();

    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
        doWhileStatement->SetEnd(lexer_->GetToken().End());
        lexer_->NextToken();
    }

    return doWhileStatement;
}

ir::FunctionDeclaration *ParserImpl::ParseFunctionDeclaration(bool canBeAnonymous, ParserStatus newStatus,
                                                              bool isDeclare)
{
    lexer::SourcePosition startLoc = lexer_->GetToken().Start();

    ASSERT(lexer_->GetToken().Type() == lexer::TokenType::KEYW_FUNCTION);
    ParserStatus savedStatus = context_.Status();

    lexer_->NextToken();

    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MULTIPLY) {
        newStatus |= ParserStatus::GENERATOR_FUNCTION;
        lexer_->NextToken();
    }

    context_.Status() = savedStatus;

    // e.g. export default function () {}
    if (lexer_->GetToken().Type() != lexer::TokenType::LITERAL_IDENT &&
        lexer_->GetToken().Type() != lexer::TokenType::KEYW_AWAIT) {
        if (canBeAnonymous) {
            ir::ScriptFunction *func = ParseFunction(newStatus, isDeclare);
            if (func->Body() != nullptr) {
                lexer_->NextToken();
            }
            func->SetStart(startLoc);
            func->SetAsExportDefault();

            auto *funcDecl = AllocNode<ir::FunctionDeclaration>(func);
            funcDecl->SetRange(func->Range());

            binder::DeclarationFlags declflag = (newStatus & ParserStatus::EXPORT_REACHED) ?
                                                binder::DeclarationFlags::EXPORT : binder::DeclarationFlags::NONE;
            Binder()->AddDecl<binder::FunctionDecl>(startLoc, declflag, isDeclare, Allocator(),
                                                    parser::SourceTextModuleRecord::DEFAULT_LOCAL_NAME, func);

            return funcDecl;
        }

        ThrowSyntaxError("Unexpected token, expected identifier after 'function' keyword");
    }

    if (!isDeclare) {
        CheckStrictReservedWord();
    }

    util::StringView ident = lexer_->GetToken().Ident();

    auto *identNode = AllocNode<ir::Identifier>(ident);
    identNode->SetRange(lexer_->GetToken().Loc());
    lexer_->NextToken();

    newStatus |= ParserStatus::FUNCTION_DECLARATION;
    ir::ScriptFunction *func = ParseFunction(newStatus, isDeclare);
    if (func->Body() != nullptr) {
        lexer_->NextToken();
    }

    func->SetIdent(identNode);
    func->SetStart(startLoc);
    auto *funcDecl = AllocNode<ir::FunctionDeclaration>(func);
    funcDecl->SetRange(func->Range());

    AddFunctionToBinder(func, newStatus);

    if (func->IsOverload() && lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
        lexer_->NextToken();
    }

    CheckOptionalBindingPatternParameter(func);

    return funcDecl;
}

void ParserImpl::AddFunctionToBinder(ir::ScriptFunction *func, ParserStatus newStatus)
{
    binder::DeclarationFlags declflag = (newStatus & ParserStatus::EXPORT_REACHED) ?
                                        binder::DeclarationFlags::EXPORT : binder::DeclarationFlags::NONE;
    const ir::Identifier *identNode = func->Id();
    const lexer::SourcePosition &startLoc = func->Start();
    const util::StringView ident = identNode->Name();
    if (Extension() == ScriptExtension::TS) {
        const auto &bindings = Binder()->GetScope()->Bindings();
        auto res = bindings.find(ident);
        binder::Decl *currentDecl = res == bindings.end() ? nullptr : res->second->Declaration();
        binder::FunctionDecl *decl {};

        if (res == bindings.end() ||
            (currentDecl->IsClassDecl() && currentDecl->AsClassDecl()->IsDeclare())) {
            decl = Binder()->AddDecl<binder::FunctionDecl>(identNode->Start(), declflag, func->Declare(),
                                                           Allocator(), ident, func);
        } else {
            if (!currentDecl->IsFunctionDecl()) {
                Binder()->ThrowRedeclaration(startLoc, currentDecl->Name());
            }

            decl = currentDecl->AsFunctionDecl();

            if (!decl->Node()->AsScriptFunction()->IsOverload()) {
                Binder()->ThrowRedeclaration(startLoc, currentDecl->Name());
            }
            if (!func->IsOverload()) {
                decl->BindNode(func);
            }
        }

        decl->Add(func);
    } else {
        Binder()->AddDecl<binder::FunctionDecl>(identNode->Start(), declflag, func->Declare(),
                                                Allocator(), ident, func);
    }
}

void ParserImpl::CheckOptionalBindingPatternParameter(ir::ScriptFunction *func) const
{
    if (func->Declare() || func->IsOverload()) {
        return;
    }
    for (auto *it : func->Params()) {
        if ((it->IsObjectPattern() && it->AsObjectPattern()->Optional()) ||
            (it->IsArrayPattern() && it->AsArrayPattern()->Optional())) {
            ThrowSyntaxError(
                "A binding pattern parameter cannot be optional in an "
                "implementation signature", it->Start());
        }
    }
}

ir::Statement *ParserImpl::ParseExpressionStatement(StatementParsingFlags flags)
{
    const auto startPos = lexer_->Save();
    ParserStatus savedStatus = context_.Status();

    if (lexer_->GetToken().IsAsyncModifier()) {
        context_.Status() |= ParserStatus::ASYNC_FUNCTION;
        lexer_->NextToken();

        if (lexer_->GetToken().Type() == lexer::TokenType::KEYW_FUNCTION && !lexer_->GetToken().NewLine()) {
            if (!(flags & StatementParsingFlags::ALLOW_LEXICAL)) {
                ThrowSyntaxError("Async functions can only be declared at the top level or inside a block.");
            }

            ir::FunctionDeclaration *functionDecl = ParseFunctionDeclaration(false, ParserStatus::ASYNC_FUNCTION);
            functionDecl->SetStart(startPos.token.Start());

            return functionDecl;
        }

        lexer_->Rewind(startPos);
    }

    ir::Expression *exprNode = ParseExpression(ExpressionParseFlags::ACCEPT_COMMA);
    context_.Status() = savedStatus;
    lexer::SourcePosition endPos = exprNode->End();

    auto *exprStatementNode = AllocNode<ir::ExpressionStatement>(exprNode);
    exprStatementNode->SetRange({startPos.token.Start(), endPos});
    ConsumeSemicolon(exprStatementNode);

    return exprStatementNode;
}

ParserImpl::ParseForInOfHelper(const ir::VariableDeclarator *varDecl, lexer::TokenType tokenType)
{
    if (varDecl->Init()) {
            if (tokenType == lexer::TokenType::KEYW_IN)
                ThrowSyntaxError("for-in loop variable declaration may not have an initializer");
            } else {
                ThrowSyntaxError("for-of loop variable declaration may not have an initializer");
            }
}

std::tuple<ForStatementKind, ir::Expression *, ir::Expression *> ParserImpl::ParseForInOf(
    ir::AstNode *initNode, ExpressionParseFlags exprFlags, bool isAwait)
{
    ForStatementKind forKind = ForStatementKind::UPDATE;
    ir::Expression *updateNode = nullptr;
    ir::Expression *rightNode = nullptr;

    if (lexer_->GetToken().Type() == lexer::TokenType::KEYW_IN ||
        lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_OF) {
        const ir::VariableDeclarator *varDecl = initNode->AsVariableDeclaration()->Declarators().front();

        if (lexer_->GetToken().Type() == lexer::TokenType::KEYW_IN) {
            ParseForInOfHelper(varDecl, lexer_->GetToken().Type());
            forKind = ForStatementKind::IN;
            exprFlags = ExpressionParseFlags::ACCEPT_COMMA;
        } else {
            ParseForInOfHelper(varDecl, lexer_->GetToken().Type());
            forKind = ForStatementKind::OF;
        }

        lexer_->NextToken();
        rightNode = ParseExpression(exprFlags);
    } else {
        if (isAwait) {
            ThrowSyntaxError("Unexpected token");
        }

        if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
            ThrowSyntaxError("Invalid left-hand side in 'For[In/Of]Statement'");
        } else if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
            lexer_->NextToken();
        } else {
            rightNode = ParseExpression(ExpressionParseFlags::ACCEPT_COMMA);
            if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
                ThrowSyntaxError("Unexpected token, expected ';' in 'ForStatement'.");
            }
            lexer_->NextToken();
        }

        if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
            updateNode = ParseExpression(ExpressionParseFlags::ACCEPT_COMMA);
        }
    }

    return {forKind, rightNode, updateNode};
}

std::tuple<ForStatementKind, ir::AstNode *, ir::Expression *, ir::Expression *> ParserImpl::ParseForInOf(
    ir::Expression *leftNode, ExpressionParseFlags exprFlags, bool isAwait)
{
    ForStatementKind forKind = ForStatementKind::UPDATE;
    ir::AstNode *initNode = nullptr;
    ir::Expression *updateNode = nullptr;
    ir::Expression *rightNode = nullptr;

    if (lexer_->GetToken().Type() == lexer::TokenType::KEYW_IN ||
        (lexer_->GetToken().KeywordType() == lexer::TokenType::KEYW_OF)) {
        if (lexer_->GetToken().Type() == lexer::TokenType::KEYW_IN) {
            forKind = ForStatementKind::IN;
            exprFlags = ExpressionParseFlags::ACCEPT_COMMA;
        } else {
            forKind = ForStatementKind::OF;
        }

        bool isValid = true;
        switch (leftNode->Type()) {
            case ir::AstNodeType::IDENTIFIER: {
                constexpr std::string_view ASYNC = "async";
                if (isAwait || !(forKind == ForStatementKind::OF && leftNode->AsIdentifier()->Name().Is(ASYNC) &&
                    leftNode->End().index - leftNode->Start().index == ASYNC.length())) {
                    break;
                }
                ThrowSyntaxError(" The left-hand side of a for-of loop may not be 'async'", leftNode->Start());
            }
            case ir::AstNodeType::MEMBER_EXPRESSION: {
                break;
            }
            case ir::AstNodeType::ARRAY_EXPRESSION: {
                isValid = leftNode->AsArrayExpression()->ConvertibleToArrayPattern();
                break;
            }
            case ir::AstNodeType::OBJECT_EXPRESSION: {
                isValid = leftNode->AsObjectExpression()->ConvertibleToObjectPattern();
                break;
            }
            default: {
                isValid = false;
            }
        }

        if (!isValid) {
            ValidateLvalueAssignmentTarget(leftNode);
        }

        initNode = leftNode;
        lexer_->NextToken();
        rightNode = ParseExpression(exprFlags);

        return {forKind, initNode, rightNode, updateNode};
    }

    if (isAwait) {
        ThrowSyntaxError("Unexpected token");
    }

    exprFlags &= ExpressionParseFlags::POTENTIALLY_IN_PATTERN;
    ir::Expression *expr = ParseAssignmentExpression(leftNode, exprFlags);

    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
        initNode = ParseSequenceExpression(expr);
    } else {
        initNode = expr;
    }

    if (initNode->IsConditionalExpression()) {
        ir::ConditionalExpression *condExpr = initNode->AsConditionalExpression();
        if (condExpr->Alternate()->IsBinaryExpression()) {
            const auto *binaryExpr = condExpr->Alternate()->AsBinaryExpression();
            if (binaryExpr->OperatorType() == lexer::TokenType::KEYW_IN) {
                ThrowSyntaxError("Invalid left-hand side in for-in statement");
            }
        }
    }

    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
        ThrowSyntaxError("Invalid left-hand side in 'For[In/Of]Statement'");
    }

    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
        ThrowSyntaxError("Unexpected token, expected ';' in 'ForStatement'.");
    }

    lexer_->NextToken();

    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
        lexer_->NextToken();
    } else {
        rightNode = ParseExpression(ExpressionParseFlags::ACCEPT_COMMA);

        if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
            ThrowSyntaxError("Unexpected token, expected ';' in 'ForStatement'.");
        }
        lexer_->NextToken();
    }

    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
        updateNode = ParseExpression(ExpressionParseFlags::ACCEPT_COMMA);
    }

    return {forKind, initNode, rightNode, updateNode};
}

std::tuple<ir::Expression *, ir::Expression *> ParserImpl::ParseForUpdate(bool isAwait)
{
    if (isAwait) {
        ThrowSyntaxError("Unexpected token");
    }

    ir::Expression *updateNode = nullptr;
    ir::Expression *rightNode = nullptr;

    if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
        lexer_->NextToken();
    } else {
        rightNode = ParseExpression(ExpressionParseFlags::ACCEPT_COMMA);
        if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
            ThrowSyntaxError("Unexpected token, expected ';' in 'ForStatement'.");
        }
        lexer_->NextToken();
    }

    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
        updateNode = ParseExpression(ExpressionParseFlags::ACCEPT_COMMA);
    }

    return {rightNode, updateNode};
}

ir::Statement *ParserImpl::ParseForStatement()
{
    lexer::SourcePosition startLoc = lexer_->GetToken().Start();
    ForStatementKind forKind = ForStatementKind::UPDATE;
    ir::AstNode *initNode = nullptr;
    ir::Expression *updateNode = nullptr;
    ir::Expression *leftNode = nullptr;
    ir::Expression *rightNode = nullptr;
    bool canBeForInOf = true;
    bool isAwait = false;
    lexer_->NextToken();
    VariableParsingFlags varFlags = VariableParsingFlags::STOP_AT_IN | VariableParsingFlags::IN_FOR;
    ExpressionParseFlags exprFlags = ExpressionParseFlags::NO_OPTS;

    if (lexer_->GetToken().Type() == lexer::TokenType::KEYW_AWAIT) {
        isAwait = true;
        varFlags |= VariableParsingFlags::DISALLOW_INIT;
        lexer_->NextToken();
    }

    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS) {
        ThrowSyntaxError("Missing left parenthesis in a 'ForStatement'");
    }
    lexer_->NextToken();

    IterationContext<binder::LoopScope> iterCtx(&context_, Binder());

    switch (lexer_->GetToken().Type()) {
        case lexer::TokenType::KEYW_VAR: {
            initNode = ParseVariableDeclaration(varFlags | VariableParsingFlags::VAR);
            break;
        }
        case lexer::TokenType::KEYW_LET: {
            initNode = ParseVariableDeclaration(varFlags | VariableParsingFlags::LET);
            break;
        }
        case lexer::TokenType::KEYW_CONST: {
            initNode = ParseVariableDeclaration(varFlags | VariableParsingFlags::CONST |
                                                VariableParsingFlags::ACCEPT_CONST_NO_INIT);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_SEMI_COLON: {
            if (isAwait) {
                ThrowSyntaxError("Unexpected token");
            }

            canBeForInOf = false;
            lexer_->NextToken();
            break;
        }
        default: {
            leftNode = ParseUnaryOrPrefixUpdateExpression(ExpressionParseFlags::POTENTIALLY_IN_PATTERN);

            break;
        }
    }

    if (initNode != nullptr) {
        if (lexer_->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
            lexer_->NextToken();
            canBeForInOf = false;
        } else {
            canBeForInOf = initNode->AsVariableDeclaration()->Declarators().size() == 1;
        }
    }

    // VariableDeclaration->DeclarationSize > 1 or seen semi_colon
    if (!canBeForInOf) {
        std::tie(rightNode, updateNode) = ParseForUpdate(isAwait);
    } else if (leftNode) {
        // initNode was parsed as LHS
        if (leftNode->IsArrayExpression() || leftNode->IsObjectExpression()) {
            exprFlags |= ExpressionParseFlags::POTENTIALLY_IN_PATTERN;
        }
        std::tie(forKind, initNode, rightNode, updateNode) = ParseForInOf(leftNode, exprFlags, isAwait);
    } else if (initNode) {
        // initNode was parsed as VariableDeclaration and declaration size = 1
        std::tie(forKind, rightNode, updateNode) = ParseForInOf(initNode, exprFlags, isAwait);
    }

    if (lexer_->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
        ThrowSyntaxError("Unexpected token, expected ')' in 'ForStatement'.");
    }
    lexer_->NextToken();

    ir::Statement *bodyNode = ParseStatement();
    lexer::SourcePosition endLoc = bodyNode->End();

    ir::Statement *forStatement = nullptr;
    auto *loopScope = iterCtx.LexicalScope().GetScope();

    if (forKind == ForStatementKind::UPDATE) {
        forStatement = AllocNode<ir::ForUpdateStatement>(loopScope, initNode, rightNode, updateNode, bodyNode);
    } else if (forKind == ForStatementKind::IN) {
        forStatement = AllocNode<ir::ForInStatement>(loopScope, initNode, rightNode, bodyNode);
    } else {
        forStatement = AllocNode<ir::ForOfStatement>(loopScope, initNode, rightNode, bodyNode, isAwait);
    }

    forStatement->SetRange({startLoc, endLoc});
    loopScope->BindNode(forStatement);

    return forStatement;
}

}  // namespace panda::es2panda::parser