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

#include "ETSparser.h"
#include "varbinder/ETSBinder.h"
#include "lexer/ETSLexer.h"
#include "ir/ets/etsTuple.h"
#include "ir/ets/etsNullishTypes.h"
#include "ir/ets/etsUnionType.h"

namespace ark::es2panda::parser {
using namespace std::literals::string_literals;

static bool IsClassMethodModifier(lexer::TokenType type)
{
    switch (type) {
        case lexer::TokenType::KEYW_STATIC:
        case lexer::TokenType::KEYW_FINAL:
        case lexer::TokenType::KEYW_NATIVE:
        case lexer::TokenType::KEYW_ASYNC:
        case lexer::TokenType::KEYW_OVERRIDE:
        case lexer::TokenType::KEYW_ABSTRACT: {
            return true;
        }
        default: {
            break;
        }
    }

    return false;
}

ir::ETSUnionType *ETSParser::CreateOptionalParameterTypeNode(ir::TypeNode *typeAnnotation,
                                                             ir::ETSUndefinedType *defaultUndef)
{
    ArenaVector<ir::TypeNode *> types(Allocator()->Adapter());
    if (typeAnnotation->IsETSUnionType()) {
        for (auto const &type : typeAnnotation->AsETSUnionType()->Types()) {
            types.push_back(type);
        }
    } else {
        types.push_back(typeAnnotation);
    }
    types.push_back(defaultUndef);

    auto *const unionType = AllocNode<ir::ETSUnionType>(std::move(types));
    unionType->SetRange({typeAnnotation->Start(), typeAnnotation->End()});
    return unionType;
}

ir::Expression *ETSParser::CreateParameterThis(const util::StringView className)
{
    auto *paramIdent = AllocNode<ir::Identifier>(varbinder::TypedBinder::MANDATORY_PARAM_THIS, Allocator());
    paramIdent->SetRange(Lexer()->GetToken().Loc());

    ir::Expression *classTypeName = AllocNode<ir::Identifier>(className, Allocator());
    classTypeName->AsIdentifier()->SetReference();
    classTypeName->SetRange(Lexer()->GetToken().Loc());

    auto typeRefPart = AllocNode<ir::ETSTypeReferencePart>(classTypeName, nullptr, nullptr);
    ir::TypeNode *typeAnnotation = AllocNode<ir::ETSTypeReference>(typeRefPart);

    typeAnnotation->SetParent(paramIdent);
    paramIdent->SetTsTypeAnnotation(typeAnnotation);

    auto *paramExpression = AllocNode<ir::ETSParameterExpression>(paramIdent, nullptr);
    paramExpression->SetRange({paramIdent->Start(), paramIdent->End()});

    return paramExpression;
}

//  Extracted from 'ParseNewExpression()' to reduce function's size
ir::ClassDefinition *ETSParser::CreateClassDefinitionForNewExpression(ArenaVector<ir::Expression *> &arguments,
                                                                      ir::TypeNode *typeReference,
                                                                      ir::TypeNode *baseTypeReference)
{
    lexer::SourcePosition endLoc = typeReference->End();

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS) {
        if (baseTypeReference != nullptr) {
            ThrowSyntaxError("Can not use 'new' on primitive types.", baseTypeReference->Start());
        }

        Lexer()->NextToken();

        while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
            ir::Expression *argument = ParseExpression();
            arguments.push_back(argument);

            if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
                Lexer()->NextToken();
                continue;
            }
        }

        endLoc = Lexer()->GetToken().End();
        Lexer()->NextToken();
    }

    ir::ClassDefinition *classDefinition {};

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        ArenaVector<ir::TSClassImplements *> implements(Allocator()->Adapter());
        auto modifiers = ir::ClassDefinitionModifiers::ANONYMOUS | ir::ClassDefinitionModifiers::HAS_SUPER;
        auto [ctor, properties, bodyRange] = ParseClassBody(modifiers);

        auto newIdent = AllocNode<ir::Identifier>("#0", Allocator());
        classDefinition = AllocNode<ir::ClassDefinition>(
            "#0", newIdent, nullptr, nullptr, std::move(implements), ctor,  // remove name
            typeReference->Clone(Allocator(), nullptr), std::move(properties), modifiers, ir::ModifierFlags::NONE,
            Language(Language::Id::ETS));

        classDefinition->SetRange(bodyRange);
    }

    return classDefinition;
}

void ETSParser::CreateImplicitConstructor([[maybe_unused]] ir::MethodDefinition *&ctor,
                                          ArenaVector<ir::AstNode *> &properties,
                                          [[maybe_unused]] ir::ClassDefinitionModifiers modifiers,
                                          const lexer::SourcePosition &startLoc)
{
    if (std::any_of(properties.cbegin(), properties.cend(), [](ir::AstNode *prop) {
            return prop->IsMethodDefinition() && prop->AsMethodDefinition()->IsConstructor();
        })) {
        return;
    }

    if ((modifiers & ir::ClassDefinitionModifiers::ANONYMOUS) != 0) {
        return;
    }

    auto *methodDef = BuildImplicitConstructor(ir::ClassDefinitionModifiers::SET_CTOR_ID, startLoc);
    properties.push_back(methodDef);
}

ir::Statement *ETSParser::CreateStatement(std::string_view const sourceCode, std::string_view const fileName)
{
    util::UString source {sourceCode, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({fileName, source.View().Utf8()});

    lexer::SourcePosition const startLoc = lexer->GetToken().Start();
    lexer->NextToken();

    auto statements = ParseStatementList(StatementParsingFlags::STMT_GLOBAL_LEXICAL);
    auto const statementNumber = statements.size();
    if (statementNumber == 0U) {
        return nullptr;
    }

    if (statementNumber == 1U) {
        return statements[0U];
    }

    auto *const blockStmt = AllocNode<ir::BlockStatement>(Allocator(), std::move(statements));
    blockStmt->SetRange({startLoc, lexer->GetToken().End()});

    for (auto *statement : blockStmt->Statements()) {
        statement->SetParent(blockStmt);
    }

    return blockStmt;
}

ir::Statement *ETSParser::CreateFormattedStatement(std::string_view const sourceCode,
                                                   std::vector<ir::AstNode *> &insertingNodes,
                                                   std::string_view const fileName)
{
    insertingNodes_.swap(insertingNodes);
    auto const statement = CreateStatement(sourceCode, fileName);
    insertingNodes_.swap(insertingNodes);
    return statement;
}

ArenaVector<ir::Statement *> ETSParser::CreateStatements(std::string_view const sourceCode,
                                                         std::string_view const fileName)
{
    util::UString source {sourceCode, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({fileName, source.View().Utf8()});

    lexer->NextToken();
    return ParseStatementList(StatementParsingFlags::STMT_GLOBAL_LEXICAL);
}

ArenaVector<ir::Statement *> ETSParser::CreateFormattedStatements(std::string_view const sourceCode,
                                                                  std::vector<ir::AstNode *> &insertingNodes,
                                                                  std::string_view const fileName)
{
    insertingNodes_.swap(insertingNodes);
    auto statements = CreateStatements(sourceCode, fileName);
    insertingNodes_.swap(insertingNodes);
    return statements;
}

ir::MethodDefinition *ETSParser::CreateMethodDefinition(ir::ModifierFlags modifiers, std::string_view const sourceCode,
                                                        std::string_view const fileName)
{
    util::UString source {sourceCode, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({fileName, source.View().Utf8()});

    auto const startLoc = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    if (IsClassMethodModifier(Lexer()->GetToken().Type())) {
        modifiers |= ParseClassMethodModifiers(false);
    }

    ir::MethodDefinition *methodDefinition = nullptr;
    auto *methodName = ExpectIdentifier();

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS ||
        Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        methodDefinition = ParseClassMethodDefinition(methodName, modifiers);
        methodDefinition->SetStart(startLoc);
    }

    return methodDefinition;
}

ir::MethodDefinition *ETSParser::CreateConstructorDefinition(ir::ModifierFlags modifiers,
                                                             std::string_view const sourceCode,
                                                             std::string_view const fileName)
{
    util::UString source {sourceCode, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({fileName, source.View().Utf8()});

    auto const startLoc = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    if (IsClassMethodModifier(Lexer()->GetToken().Type())) {
        modifiers |= ParseClassMethodModifiers(false);
    }

    if (Lexer()->GetToken().Type() != lexer::TokenType::KEYW_CONSTRUCTOR) {
        ThrowSyntaxError({"Unexpected token. 'Constructor' keyword is expected."});
    }

    if ((modifiers & ir::ModifierFlags::ASYNC) != 0) {
        ThrowSyntaxError({"Constructor should not be async."});
    }

    auto *memberName = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
    modifiers |= ir::ModifierFlags::CONSTRUCTOR;
    Lexer()->NextToken();

    auto *const methodDefinition = ParseClassMethodDefinition(memberName, modifiers);
    methodDefinition->SetStart(startLoc);

    return methodDefinition;
}

ir::Expression *ETSParser::CreateExpression(std::string_view const sourceCode, ExpressionParseFlags const flags,
                                            std::string_view const fileName)
{
    util::UString source {sourceCode, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({fileName, source.View().Utf8()});

    lexer::SourcePosition const startLoc = lexer->GetToken().Start();
    lexer->NextToken();

    ir::Expression *returnExpression = ParseExpression(flags);
    returnExpression->SetRange({startLoc, lexer->GetToken().End()});

    return returnExpression;
}

ir::Expression *ETSParser::CreateFormattedExpression(std::string_view const sourceCode,
                                                     std::vector<ir::AstNode *> &insertingNodes,
                                                     std::string_view const fileName)
{
    ir::Expression *returnExpression;
    insertingNodes_.swap(insertingNodes);

    if (auto statements = CreateStatements(sourceCode, fileName);
        statements.size() == 1U && statements.back()->IsExpressionStatement()) {
        returnExpression = statements.back()->AsExpressionStatement()->GetExpression();
    } else {
        returnExpression = AllocNode<ir::BlockExpression>(std::move(statements));
    }

    insertingNodes_.swap(insertingNodes);
    return returnExpression;
}

ir::TypeNode *ETSParser::CreateTypeAnnotation(TypeAnnotationParsingOptions *options, std::string_view const sourceCode,
                                              std::string_view const fileName)
{
    util::UString source {sourceCode, Allocator()};
    auto const isp = InnerSourceParser(this);
    auto const lexer = InitLexer({fileName, source.View().Utf8()});

    lexer->NextToken();
    return ParseTypeAnnotation(options);
}

}  // namespace ark::es2panda::parser