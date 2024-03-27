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
#include "lexer/ETSLexer.h"
#include "ir/expressions/literals/undefinedLiteral.h"
#include "ir/ets/etsTuple.h"
#include "ir/ets/etsNullishTypes.h"
#include "ir/ets/etsUnionType.h"

namespace ark::es2panda::parser {

// NOLINTBEGIN(modernize-avoid-c-arrays)
static constexpr char const NO_DEFAULT_FOR_REST[] = "Rest parameter cannot have the default value.";
static constexpr char const ONLY_ARRAY_FOR_REST[] = "Rest parameter should be of an array type.";
static constexpr char const EXPLICIT_PARAM_TYPE[] = "Parameter declaration should have an explicit type annotation.";
// NOLINTEND(modernize-avoid-c-arrays)

bool IsPunctuartorSpecialCharacter(lexer::TokenType tokenType)
{
    switch (tokenType) {
        case lexer::TokenType::PUNCTUATOR_COLON:
        case lexer::TokenType::PUNCTUATOR_COMMA:
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT:
        case lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET:
        case lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET:
        case lexer::TokenType::PUNCTUATOR_LESS_THAN:
        case lexer::TokenType::PUNCTUATOR_GREATER_THAN:
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR:
            return true;
        default:
            return false;
    }
}

ir::ScriptFunction *ETSParser::ParseFunction(ParserStatus newStatus, ir::Identifier *className)
{
    FunctionContext functionContext(this, newStatus | ParserStatus::FUNCTION);
    lexer::SourcePosition startLoc = Lexer()->GetToken().Start();
    auto [signature, throwMarker] = ParseFunctionSignature(newStatus, className);

    ir::AstNode *body = nullptr;
    lexer::SourcePosition endLoc = startLoc;
    bool isOverload = false;
    bool isArrow = (newStatus & ParserStatus::ARROW_FUNCTION) != 0;

    if ((newStatus & ParserStatus::ASYNC_FUNCTION) != 0) {
        functionContext.AddFlag(ir::ScriptFunctionFlags::ASYNC);
    }

    if (isArrow) {
        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_ARROW) {
            ThrowSyntaxError("'=>' expected");
        }

        functionContext.AddFlag(ir::ScriptFunctionFlags::ARROW);
        Lexer()->NextToken();
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        std::tie(std::ignore, body, endLoc, isOverload) =
            ParseFunctionBody(signature.Params(), newStatus, GetContext().Status());
    } else if (isArrow) {
        body = ParseExpression();
        endLoc = body->AsExpression()->End();
        functionContext.AddFlag(ir::ScriptFunctionFlags::EXPRESSION);
    }

    if ((GetContext().Status() & ParserStatus::FUNCTION_HAS_RETURN_STATEMENT) != 0) {
        functionContext.AddFlag(ir::ScriptFunctionFlags::HAS_RETURN);
        GetContext().Status() ^= ParserStatus::FUNCTION_HAS_RETURN_STATEMENT;
    }
    functionContext.AddFlag(throwMarker);

    // clang-format off
    auto *funcNode = AllocNode<ir::ScriptFunction>(
        Allocator(), ir::ScriptFunction::ScriptFunctionData {
                        body, std::move(signature), functionContext.Flags(), {}, false, GetContext().GetLanguage()});
    // clang-format on

    funcNode->SetRange({startLoc, endLoc});

    return funcNode;
}

std::tuple<bool, ir::BlockStatement *, lexer::SourcePosition, bool> ETSParser::ParseFunctionBody(
    [[maybe_unused]] const ArenaVector<ir::Expression *> &params, [[maybe_unused]] ParserStatus newStatus,
    [[maybe_unused]] ParserStatus contextStatus)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE);

    ir::BlockStatement *body = ParseBlockStatement();

    return {true, body, body->End(), false};
}

ir::TypeNode *ETSParser::ParseFunctionReturnType([[maybe_unused]] ParserStatus status)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COLON) {
        if ((status & ParserStatus::CONSTRUCTOR_FUNCTION) != 0U) {
            ThrowSyntaxError("Type annotation isn't allowed for constructor.");
        }
        Lexer()->NextToken();  // eat ':'
        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR |
                                               TypeAnnotationParsingOptions::CAN_BE_TS_TYPE_PREDICATE |
                                               TypeAnnotationParsingOptions::RETURN_TYPE;
        return ParseTypeAnnotation(&options);
    }

    return nullptr;
}

ir::ScriptFunctionFlags ETSParser::ParseFunctionThrowMarker(bool isRethrowsAllowed)
{
    ir::ScriptFunctionFlags throwMarker = ir::ScriptFunctionFlags::NONE;

    if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT) {
        if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_THROWS) {
            Lexer()->NextToken();  // eat 'throws'
            throwMarker = ir::ScriptFunctionFlags::THROWS;
        } else if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_RETHROWS) {
            if (isRethrowsAllowed) {
                Lexer()->NextToken();  // eat 'rethrows'
                throwMarker = ir::ScriptFunctionFlags::RETHROWS;
            } else {
                ThrowSyntaxError("Only 'throws' can be used with function types");
            }
        }
    }

    return throwMarker;
}

ir::TypeNode *ETSParser::ParseFunctionType()
{
    auto startLoc = Lexer()->GetToken().Start();
    auto params = ParseFunctionParams();

    auto *const returnTypeAnnotation = [this]() -> ir::TypeNode * {
        ExpectToken(lexer::TokenType::PUNCTUATOR_ARROW);
        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
        return ParseTypeAnnotation(&options);
    }();

    ir::ScriptFunctionFlags throwMarker = ParseFunctionThrowMarker(false);

    auto *funcType = AllocNode<ir::ETSFunctionType>(
        ir::FunctionSignature(nullptr, std::move(params), returnTypeAnnotation), throwMarker);
    const auto endLoc = returnTypeAnnotation->End();
    funcType->SetRange({startLoc, endLoc});

    return funcType;
}

ir::Statement *ETSParser::ParseFunctionStatement([[maybe_unused]] const StatementParsingFlags flags)
{
    ASSERT((flags & StatementParsingFlags::GLOBAL) == 0);
    ThrowSyntaxError("Nested functions are not allowed");
}

ir::Expression *ETSParser::ParseFunctionParameterExpression(ir::AnnotatedExpression *const paramIdent,
                                                            ir::ETSUndefinedType *defaultUndef)
{
    ir::ETSParameterExpression *paramExpression;
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        if (paramIdent->IsRestElement()) {
            ThrowSyntaxError(NO_DEFAULT_FOR_REST);
        }

        auto const lexerPos = Lexer()->Save().Iterator();
        Lexer()->NextToken();  // eat '='

        if (defaultUndef != nullptr) {
            ThrowSyntaxError("Not enable default value with default undefined");
        }
        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS ||
            Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
            ThrowSyntaxError("You didn't set the value.");
        }

        paramExpression = AllocNode<ir::ETSParameterExpression>(paramIdent->AsIdentifier(), ParseExpression());

        std::string value = Lexer()->SourceView(lexerPos.Index(), Lexer()->Save().Iterator().Index()).Mutf8();
        while (value.back() == ' ') {
            value.pop_back();
        }
        if (value.back() == ')' || value.back() == ',') {
            value.pop_back();
        }
        paramExpression->SetLexerSaved(util::UString(value, Allocator()).View());

        paramExpression->SetRange({paramIdent->Start(), paramExpression->Initializer()->End()});
    } else if (paramIdent->IsIdentifier()) {
        auto *typeAnnotation = paramIdent->AsIdentifier()->TypeAnnotation();

        const auto typeAnnotationValue = [this, typeAnnotation,
                                          defaultUndef]() -> std::pair<ir::Expression *, std::string> {
            if (typeAnnotation == nullptr) {
                return std::make_pair(nullptr, "");
            }
            return std::make_pair(defaultUndef != nullptr ? AllocNode<ir::UndefinedLiteral>() : nullptr, "undefined");
        }();

        paramExpression =
            AllocNode<ir::ETSParameterExpression>(paramIdent->AsIdentifier(), std::get<0>(typeAnnotationValue));
        if (defaultUndef != nullptr) {
            paramExpression->SetLexerSaved(util::UString(std::get<1>(typeAnnotationValue), Allocator()).View());
        }
        paramExpression->SetRange({paramIdent->Start(), paramIdent->End()});
    } else {
        paramExpression = AllocNode<ir::ETSParameterExpression>(paramIdent->AsRestElement(), nullptr);
        paramExpression->SetRange({paramIdent->Start(), paramIdent->End()});
    }
    return paramExpression;
}

ir::Expression *ETSParser::ParseFunctionParameter()
{
    auto *const paramIdent = GetAnnotatedExpressionFromParam();

    ir::ETSUndefinedType *defaultUndef = nullptr;

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_QUESTION_MARK) {
        if (paramIdent->IsRestElement()) {
            ThrowSyntaxError(NO_DEFAULT_FOR_REST);
        }
        defaultUndef = AllocNode<ir::ETSUndefinedType>();
        defaultUndef->SetRange({Lexer()->GetToken().Start(), Lexer()->GetToken().End()});
        Lexer()->NextToken();  // eat '?'
    }

    const bool isArrow = (GetContext().Status() & ParserStatus::ARROW_FUNCTION) != 0;

    if (Lexer()->TryEatTokenType(lexer::TokenType::PUNCTUATOR_COLON)) {
        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
        ir::TypeNode *typeAnnotation = ParseTypeAnnotation(&options);

        if (defaultUndef != nullptr) {
            typeAnnotation = CreateOptionalParameterTypeNode(typeAnnotation, defaultUndef);
        }

        if (paramIdent->IsRestElement() && !typeAnnotation->IsTSArrayType()) {
            ThrowSyntaxError(ONLY_ARRAY_FOR_REST);
        }

        typeAnnotation->SetParent(paramIdent);
        paramIdent->SetTsTypeAnnotation(typeAnnotation);
        paramIdent->SetEnd(typeAnnotation->End());
    } else if (!isArrow && defaultUndef == nullptr) {
        ThrowSyntaxError(EXPLICIT_PARAM_TYPE);
    }

    return ParseFunctionParameterExpression(paramIdent, defaultUndef);
}

bool ETSParser::IsArrowFunctionExpressionStart()
{
    const auto savedPos = Lexer()->Save();
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS);
    Lexer()->NextToken();
    auto tokenType = Lexer()->GetToken().Type();

    size_t openBrackets = 1;
    bool expectIdentifier = true;
    while (tokenType != lexer::TokenType::EOS && openBrackets > 0) {
        switch (tokenType) {
            case lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS:
                --openBrackets;
                break;
            case lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS:
                ++openBrackets;
                break;
            case lexer::TokenType::PUNCTUATOR_COMMA:
                expectIdentifier = true;
                break;
            case lexer::TokenType::PUNCTUATOR_SEMI_COLON:
                Lexer()->Rewind(savedPos);
                return false;
            default:
                if (!expectIdentifier) {
                    break;
                }
                if (tokenType != lexer::TokenType::LITERAL_IDENT &&
                    tokenType != lexer::TokenType::PUNCTUATOR_PERIOD_PERIOD_PERIOD) {
                    Lexer()->Rewind(savedPos);
                    return false;
                }
                expectIdentifier = false;
        }
        Lexer()->NextToken();
        tokenType = Lexer()->GetToken().Type();
    }

    while (tokenType != lexer::TokenType::EOS && tokenType != lexer::TokenType::PUNCTUATOR_ARROW) {
        if (lexer::Token::IsPunctuatorToken(tokenType) && !IsPunctuartorSpecialCharacter(tokenType)) {
            break;
        }
        Lexer()->NextToken();
        tokenType = Lexer()->GetToken().Type();
    }
    Lexer()->Rewind(savedPos);
    return tokenType == lexer::TokenType::PUNCTUATOR_ARROW;
}

ir::ArrowFunctionExpression *ETSParser::ParseArrowFunctionExpression()
{
    auto newStatus = ParserStatus::ARROW_FUNCTION;
    auto *func = ParseFunction(newStatus);
    auto *arrowFuncNode = AllocNode<ir::ArrowFunctionExpression>(Allocator(), func);
    arrowFuncNode->SetRange(func->Range());
    return arrowFuncNode;
}

bool ETSParser::ParsePotentialGenericFunctionCall(ir::Expression *primaryExpr, ir::Expression **returnExpression,
                                                  [[maybe_unused]] const lexer::SourcePosition &startLoc,
                                                  bool ignoreCallExpression)
{
    if (Lexer()->Lookahead() == lexer::LEX_CHAR_LESS_THAN ||
        (!primaryExpr->IsIdentifier() && !primaryExpr->IsMemberExpression())) {
        return true;
    }

    const auto savedPos = Lexer()->Save();

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SHIFT) {
        Lexer()->BackwardToken(lexer::TokenType::PUNCTUATOR_LESS_THAN, 1);
    }

    TypeAnnotationParsingOptions options =
        TypeAnnotationParsingOptions::ALLOW_WILDCARD | TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE;
    ir::TSTypeParameterInstantiation *typeParams = ParseTypeParameterInstantiation(&options);

    if (typeParams == nullptr) {
        Lexer()->Rewind(savedPos);
        return true;
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::EOS) {
        ThrowSyntaxError("'(' expected");
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS) {
        if (!ignoreCallExpression) {
            *returnExpression = ParseCallExpression(*returnExpression, false, false);
            (*returnExpression)->AsCallExpression()->SetTypeParams(typeParams);
            return false;
        }

        return true;
    }

    Lexer()->Rewind(savedPos);
    return true;
}

ir::FunctionDeclaration *ETSParser::ParseFunctionDeclaration(bool canBeAnonymous, ir::ModifierFlags modifiers)
{
    lexer::SourcePosition startLoc = Lexer()->GetToken().Start();

    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::KEYW_FUNCTION);
    Lexer()->NextToken();
    auto newStatus = ParserStatus::NEED_RETURN_TYPE | ParserStatus::ALLOW_SUPER;

    if ((modifiers & ir::ModifierFlags::ASYNC) != 0) {
        newStatus |= ParserStatus::ASYNC_FUNCTION;
    }
    if (Lexer()->TryEatTokenType(lexer::TokenType::PUNCTUATOR_MULTIPLY)) {
        newStatus |= ParserStatus::GENERATOR_FUNCTION;
    }

    ir::Identifier *className = nullptr;
    ir::Identifier *identNode = nullptr;
    if (Lexer()->Lookahead() == lexer::LEX_CHAR_DOT) {
        className = ExpectIdentifier();
        if (className != nullptr) {
            newStatus |= ParserStatus::IN_EXTENSION_FUNCTION;
        }
        Lexer()->NextToken();
        identNode = ExpectIdentifier();
    } else if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT) {
        identNode = ExpectIdentifier();
    } else if (!canBeAnonymous) {
        ThrowSyntaxError("Unexpected token, expected identifier after 'function' keyword");
    }
    newStatus |= ParserStatus::FUNCTION_DECLARATION;
    if (identNode != nullptr) {
        CheckRestrictedBinding(identNode->Name(), identNode->Start());
    }
    ir::ScriptFunction *func = ParseFunction(newStatus, className);
    func->SetIdent(identNode);
    auto *funcDecl = AllocNode<ir::FunctionDeclaration>(Allocator(), func);
    if (func->IsOverload() && Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SEMI_COLON) {
        Lexer()->NextToken();
    }
    funcDecl->SetRange(func->Range());
    func->AddModifier(modifiers);
    func->SetStart(startLoc);

    if (className != nullptr) {
        func->AddFlag(ir::ScriptFunctionFlags::INSTANCE_EXTENSION_METHOD);
    }

    return funcDecl;
}

}  // namespace ark::es2panda::parser