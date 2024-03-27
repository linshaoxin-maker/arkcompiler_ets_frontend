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
#include "macros.h"
#include "lexer/ETSLexer.h"
#include "checker/types/ets/etsEnumType.h"
#include "ir/ets/etsTuple.h"
#include "ir/ts/tsEnumDeclaration.h"
#include "ir/ts/tsEnumMember.h"

namespace ark::es2panda::parser {
using namespace std::literals::string_literals;

// NOLINTBEGIN(cert-err58-cpp)
static std::string const DUPLICATE_ENUM_VALUE = "Duplicate enum initialization value "s;
static std::string const INVALID_ENUM_TYPE = "Invalid enum initialization type"s;
static std::string const INVALID_ENUM_VALUE = "Invalid enum initialization value"s;
static std::string const MISSING_COMMA_IN_ENUM = "Missing comma between enum constants"s;
static std::string const TRAILING_COMMA_IN_ENUM = "Trailing comma is not allowed in enum constant list"s;
// NOLINTEND(cert-err58-cpp)

ir::TSEnumDeclaration *ETSParser::ParseEnumMembers(ir::Identifier *const key, const lexer::SourcePosition &enumStart,
                                                   const bool isConst, const bool isStatic)
{
    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        ThrowSyntaxError("'{' expected");
    }

    Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);  // eat '{'

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        ThrowSyntaxError("An enum must have at least one enum constant");
    }

    // Lambda to check if enum underlying type is string:
    auto const isStringEnum = [this]() -> bool {
        Lexer()->NextToken();
        auto tokenType = Lexer()->GetToken().Type();
        while (tokenType != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE &&
               tokenType != lexer::TokenType::PUNCTUATOR_COMMA) {
            if (tokenType == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
                Lexer()->NextToken();
                if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_STRING) {
                    return true;
                }
            }
            Lexer()->NextToken();
            tokenType = Lexer()->GetToken().Type();
        }
        return false;
    };

    // Get the underlying type of enum (number or string). It is defined from the first element ONLY!
    auto const pos = Lexer()->Save();
    auto const stringTypeEnum = isStringEnum();
    Lexer()->Rewind(pos);

    ArenaVector<ir::AstNode *> members(Allocator()->Adapter());

    if (stringTypeEnum) {
        ParseStringEnum(members);
    } else {
        ParseNumberEnum(members);
    }

    auto *const enumDeclaration =
        AllocNode<ir::TSEnumDeclaration>(Allocator(), key, std::move(members), isConst, isStatic, InAmbientContext());
    enumDeclaration->SetRange({enumStart, Lexer()->GetToken().End()});

    Lexer()->NextToken();  // eat '}'

    return enumDeclaration;
}

void ETSParser::ParseNumberEnum(ArenaVector<ir::AstNode *> &members)
{
    checker::ETSEnumType::ValueType currentValue {};

    // Lambda to parse enum member (maybe with initializer)
    auto const parseMember = [this, &members, &currentValue]() {
        auto *const ident = ExpectIdentifier(false, true);

        ir::NumberLiteral *ordinal;
        lexer::SourcePosition endLoc;

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
            // Case when user explicitly set the value for enumeration constant

            bool minusSign = false;

            Lexer()->NextToken();
            if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_PLUS) {
                Lexer()->NextToken();
            } else if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MINUS) {
                minusSign = true;
                Lexer()->NextToken();
            }

            if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_NUMBER) {
                ThrowSyntaxError(INVALID_ENUM_TYPE);
            }

            ordinal = ParseNumberLiteral()->AsNumberLiteral();
            if (minusSign) {
                ordinal->Number().Negate();
            }
            if (!ordinal->Number().CanGetValue<checker::ETSEnumType::ValueType>()) {
                ThrowSyntaxError(INVALID_ENUM_VALUE);
            }

            currentValue = ordinal->Number().GetValue<checker::ETSEnumType::ValueType>();

            endLoc = ordinal->End();
        } else {
            // Default enumeration constant value. Equal to 0 for the first item and = previous_value + 1 for all
            // the others.

            ordinal = AllocNode<ir::NumberLiteral>(lexer::Number(currentValue));

            endLoc = ident->End();
        }

        auto *const member = AllocNode<ir::TSEnumMember>(ident, ordinal);
        member->SetRange({ident->Start(), endLoc});
        members.emplace_back(member);

        ++currentValue;
    };

    parseMember();

    while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_COMMA) {
            ThrowSyntaxError(MISSING_COMMA_IN_ENUM);
        }

        Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);  // eat ','

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
            break;
        }

        parseMember();
    }
}

void ETSParser::ParseStringEnum(ArenaVector<ir::AstNode *> &members)
{
    // Lambda to parse enum member (maybe with initializer)
    auto const parseMember = [this, &members]() {
        auto *const ident = ExpectIdentifier();

        ir::StringLiteral *itemValue;

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
            // Case when user explicitly set the value for enumeration constant

            Lexer()->NextToken();
            if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_STRING) {
                ThrowSyntaxError(INVALID_ENUM_TYPE);
            }

            itemValue = ParseStringLiteral();
        } else {
            // Default item value is not allowed for string type enumerations!
            ThrowSyntaxError("All items of string-type enumeration should be explicitly initialized.");
        }

        auto *const member = AllocNode<ir::TSEnumMember>(ident, itemValue);
        member->SetRange({ident->Start(), itemValue->End()});
        members.emplace_back(member);
    };

    parseMember();

    while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_COMMA) {
            ThrowSyntaxError(MISSING_COMMA_IN_ENUM);
        }

        Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);  // eat ','

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
            ThrowSyntaxError(TRAILING_COMMA_IN_ENUM);
        }

        parseMember();
    }
}

ir::ThisExpression *ETSParser::ParseThisExpression()
{
    auto *thisExpression = TypedParser::ParseThisExpression();

    if (Lexer()->GetToken().NewLine()) {
        return thisExpression;
    }

    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::PUNCTUATOR_PERIOD:
        case lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS:
        case lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS:
        case lexer::TokenType::PUNCTUATOR_SEMI_COLON:
        case lexer::TokenType::PUNCTUATOR_COLON:
        case lexer::TokenType::PUNCTUATOR_EQUAL:
        case lexer::TokenType::PUNCTUATOR_NOT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_STRICT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_NOT_STRICT_EQUAL:
        case lexer::TokenType::PUNCTUATOR_COMMA:
        case lexer::TokenType::PUNCTUATOR_QUESTION_MARK:
        case lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET:
        case lexer::TokenType::KEYW_INSTANCEOF:
        case lexer::TokenType::KEYW_AS: {
            break;
        }
        default: {
            ThrowUnexpectedToken(Lexer()->GetToken().Type());
            break;
        }
    }

    return thisExpression;
}

}  // namespace ark::es2panda::parser