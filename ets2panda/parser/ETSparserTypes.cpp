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
#include "ETSNolintParser.h"
#include <utility>

#include "macros.h"
#include "parser/parserFlags.h"
#include "parser/parserStatusContext.h"
#include "util/helpers.h"
#include "util/language.h"
#include "utils/arena_containers.h"
#include "varbinder/varbinder.h"
#include "varbinder/ETSBinder.h"
#include "lexer/lexer.h"
#include "lexer/ETSLexer.h"
#include "ir/astNode.h"
#include "ir/base/decorator.h"
#include "ir/base/catchClause.h"
#include "ir/base/scriptFunction.h"
#include "ir/base/methodDefinition.h"
#include "ir/base/spreadElement.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/functionExpression.h"
#include "ir/expressions/dummyNode.h"
#include "ir/module/importDeclaration.h"
#include "ir/module/importDefaultSpecifier.h"
#include "ir/module/importSpecifier.h"
#include "ir/module/exportSpecifier.h"
#include "ir/module/exportNamedDeclaration.h"
#include "ir/ets/etsPrimitiveType.h"
#include "ir/ets/etsPackageDeclaration.h"
#include "ir/ets/etsReExportDeclaration.h"
#include "ir/ets/etsWildcardType.h"
#include "ir/ets/etsTuple.h"
#include "ir/ets/etsFunctionType.h"
#include "ir/ets/etsScript.h"
#include "ir/ets/etsTypeReference.h"
#include "ir/ets/etsTypeReferencePart.h"
#include "ir/ets/etsNullishTypes.h"
#include "ir/ets/etsUnionType.h"
#include "ir/ets/etsImportSource.h"
#include "ir/ets/etsImportDeclaration.h"
#include "ir/ets/etsStructDeclaration.h"
#include "ir/module/importNamespaceSpecifier.h"
#include "ir/ts/tsInterfaceDeclaration.h"
#include "ir/ts/tsTypeParameterInstantiation.h"
#include "ir/ts/tsInterfaceBody.h"
#include "ir/ts/tsImportEqualsDeclaration.h"
#include "ir/ts/tsArrayType.h"
#include "ir/ts/tsQualifiedName.h"
#include "ir/ts/tsTypeReference.h"
#include "ir/ts/tsTypeParameter.h"
#include "ir/ts/tsInterfaceHeritage.h"
#include "ir/ts/tsFunctionType.h"
#include "ir/ts/tsTypeAliasDeclaration.h"
#include "ir/ts/tsTypeParameterDeclaration.h"
#include "ir/ts/tsThisType.h"
#include "generated/signatures.h"

namespace ark::es2panda::parser {
class FunctionContext;

using namespace std::literals::string_literals;

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

ir::TypeNode *ETSParser::ParsePrimitiveType(TypeAnnotationParsingOptions *options, ir::PrimitiveType type)
{
    if (((*options) & TypeAnnotationParsingOptions::DISALLOW_PRIMARY_TYPE) != 0) {
        ThrowSyntaxError("Primitive type is not allowed here.");
    }

    auto *typeAnnotation = AllocNode<ir::ETSPrimitiveType>(type);
    typeAnnotation->SetRange(Lexer()->GetToken().Loc());
    Lexer()->NextToken();
    return typeAnnotation;
}

ir::TypeNode *ETSParser::ParseUnionType(ir::TypeNode *const firstType)
{
    ArenaVector<ir::TypeNode *> types(Allocator()->Adapter());
    types.push_back(firstType->AsTypeNode());

    while (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_BITWISE_OR) {
        Lexer()->NextToken();  // eat '|'

        auto options = TypeAnnotationParsingOptions::THROW_ERROR | TypeAnnotationParsingOptions::DISALLOW_UNION;
        types.push_back(ParseTypeAnnotation(&options));
    }

    auto const endLoc = types.back()->End();
    auto *const unionType = AllocNode<ir::ETSUnionType>(std::move(types));
    unionType->SetRange({firstType->Start(), endLoc});
    return unionType;
}

ir::TypeNode *ETSParser::GetTypeAnnotationOfPrimitiveType([[maybe_unused]] lexer::TokenType tokenType,
                                                          TypeAnnotationParsingOptions *options)
{
    ir::TypeNode *typeAnnotation = nullptr;
    switch (tokenType) {
        case lexer::TokenType::KEYW_BOOLEAN:
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::BOOLEAN);
            break;
        case lexer::TokenType::KEYW_DOUBLE:
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::DOUBLE);
            break;
        case lexer::TokenType::KEYW_BYTE:
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::BYTE);
            break;
        case lexer::TokenType::KEYW_FLOAT:
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::FLOAT);
            break;
        case lexer::TokenType::KEYW_SHORT:
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::SHORT);
            break;
        case lexer::TokenType::KEYW_INT:
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::INT);
            break;
        case lexer::TokenType::KEYW_CHAR:
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::CHAR);
            break;
        case lexer::TokenType::KEYW_LONG:
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::LONG);
            break;
        case lexer::TokenType::KEYW_VOID:
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::VOID);
            break;
        default:
            typeAnnotation = ParseTypeReference(options);
            break;
    }
    return typeAnnotation;
}

ir::TypeNode *ETSParser::ParseWildcardType(TypeAnnotationParsingOptions *options)
{
    const auto varianceStartLoc = Lexer()->GetToken().Start();
    const auto varianceEndLoc = Lexer()->GetToken().End();
    const auto varianceModifier = ParseTypeVarianceModifier(options);

    auto *typeReference = [this, &varianceModifier, options]() -> ir::ETSTypeReference * {
        if (varianceModifier == ir::ModifierFlags::OUT &&
            (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_GREATER_THAN ||
             Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA)) {
            // unbounded 'out'
            return nullptr;
        }
        return ParseTypeReference(options)->AsETSTypeReference();
    }();

    auto *wildcardType = AllocNode<ir::ETSWildcardType>(typeReference, varianceModifier);
    wildcardType->SetRange({varianceStartLoc, typeReference == nullptr ? varianceEndLoc : typeReference->End()});

    return wildcardType;
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

ir::TypeNode *ETSParser::ParseETSTupleType(TypeAnnotationParsingOptions *const options)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET);

    const auto startLoc = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat '['

    ArenaVector<ir::TypeNode *> tupleTypeList(Allocator()->Adapter());
    auto *const tupleType = AllocNode<ir::ETSTuple>(Allocator());

    bool spreadTypePresent = false;

    while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET) {
        // Parse named parameter if name presents
        if ((Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT) &&
            (Lexer()->Lookahead() == lexer::LEX_CHAR_COLON)) {
            ExpectIdentifier();
            Lexer()->NextToken();  // eat ':'
        }

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_PERIOD_PERIOD_PERIOD) {
            if (spreadTypePresent) {
                ThrowSyntaxError("Only one spread type declaration allowed, at the last index");
            }

            spreadTypePresent = true;
            Lexer()->NextToken();  // eat '...'
        } else if (spreadTypePresent) {
            // This can't be implemented to any index, with type consistency. If a spread type is in the middle of
            // the tuple, then bounds check can't be made for element access, so the type of elements after the
            // spread can't be determined in compile time.
            ThrowSyntaxError("Spread type must be at the last index in the tuple type");
        }

        auto *const currentTypeAnnotation = ParseTypeAnnotation(options);
        currentTypeAnnotation->SetParent(tupleType);

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_QUESTION_MARK) {
            // NOTE(mmartin): implement optional types for tuples
            ThrowSyntaxError("Optional types in tuples are not yet implemented.");
        }

        if (spreadTypePresent) {
            if (!currentTypeAnnotation->IsTSArrayType()) {
                ThrowSyntaxError("Spread type must be an array type");
            }

            tupleType->SetSpreadType(currentTypeAnnotation);
        } else {
            tupleTypeList.push_back(currentTypeAnnotation);
        }

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
            Lexer()->NextToken();  // eat comma
            continue;
        }

        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET) {
            ThrowSyntaxError("Comma is mandatory between elements in a tuple type declaration");
        }
    }

    Lexer()->NextToken();  // eat ']'

    tupleType->SetTypeAnnotationsList(tupleTypeList);
    const auto endLoc = Lexer()->GetToken().End();
    tupleType->SetRange({startLoc, endLoc});

    return tupleType;
}

// Helper function for  ETSParser::GetTypeAnnotationFromToken(...) method
ir::TypeNode *ETSParser::ParsePotentialFunctionalType(TypeAnnotationParsingOptions *options,
                                                      lexer::SourcePosition startLoc)
{
    if (((*options) & TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE) == 0 &&
        (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS ||
         Lexer()->Lookahead() == lexer::LEX_CHAR_COLON)) {
        auto typeAnnotation = ParseFunctionType();
        typeAnnotation->SetStart(startLoc);

        if (auto position = GetDefaultParamPosition(typeAnnotation->AsETSFunctionType()->Params())) {
            ThrowSyntaxError("Default parameters can not be used in functional type", position.value());
        }

        return typeAnnotation;
    }

    auto savePos = Lexer()->Save();
    ParseTypeAnnotation(options);

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_QUESTION_MARK) {
        ThrowSyntaxError("Default parameters can not be used in functional type");
    }

    Lexer()->Rewind(savePos);
    return nullptr;
}

// Just to reduce the size of ParseTypeAnnotation(...) method
std::pair<ir::TypeNode *, bool> ETSParser::GetTypeAnnotationFromToken(TypeAnnotationParsingOptions *options)
{
    ir::TypeNode *typeAnnotation = nullptr;

    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::LITERAL_IDENT: {
            typeAnnotation = ParseLiteralIdent(options);
            if (((*options) & TypeAnnotationParsingOptions::POTENTIAL_CLASS_LITERAL) != 0 &&
                (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_CLASS || IsStructKeyword())) {
                return std::make_pair(typeAnnotation, false);
            }
            break;
        }
        case lexer::TokenType::LITERAL_NULL: {
            typeAnnotation = AllocNode<ir::ETSNullType>();
            typeAnnotation->SetRange(Lexer()->GetToken().Loc());
            Lexer()->NextToken();
            break;
        }
        case lexer::TokenType::KEYW_UNDEFINED: {
            typeAnnotation = AllocNode<ir::ETSUndefinedType>();
            typeAnnotation->SetRange(Lexer()->GetToken().Loc());
            Lexer()->NextToken();
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS: {
            auto startLoc = Lexer()->GetToken().Start();
            lexer::LexerPosition savedPos = Lexer()->Save();
            Lexer()->NextToken();  // eat '('

            typeAnnotation = ParsePotentialFunctionalType(options, startLoc);
            if (typeAnnotation != nullptr) {
                return std::make_pair(typeAnnotation, false);
            }

            typeAnnotation = ParseTypeAnnotation(options);
            typeAnnotation->SetStart(startLoc);

            if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_BITWISE_OR) {
                typeAnnotation = ParseUnionType(typeAnnotation);
            }

            ParseRightParenthesis(options, typeAnnotation, savedPos);
            break;
        }
        case lexer::TokenType::PUNCTUATOR_FORMAT: {
            typeAnnotation = ParseTypeFormatPlaceholder();
            break;
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET: {
            typeAnnotation = ParseETSTupleType(options);
            break;
        }
        case lexer::TokenType::KEYW_THIS: {
            typeAnnotation = ParseThisType(options);
            break;
        }
        default: {
            break;
        }
    }

    return std::make_pair(typeAnnotation, true);
}

ir::TypeNode *ETSParser::ParseThisType(TypeAnnotationParsingOptions *options)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::KEYW_THIS);

    // A syntax error should be thrown if
    // - the usage of 'this' as a type is not allowed in the current context, or
    // - 'this' is not used as a return type, or
    // - the current context is an arrow function (might be inside a method of a class where 'this' is allowed).
    if (((*options & TypeAnnotationParsingOptions::THROW_ERROR) != 0) &&
        (((GetContext().Status() & ParserStatus::ALLOW_THIS_TYPE) == 0) ||
         ((*options & TypeAnnotationParsingOptions::RETURN_TYPE) == 0) ||
         ((GetContext().Status() & ParserStatus::ARROW_FUNCTION) != 0))) {
        ThrowSyntaxError("A 'this' type is available only as return type in a non-static method of a class or struct.");
    }

    auto *thisType = AllocNode<ir::TSThisType>();
    thisType->SetRange(Lexer()->GetToken().Loc());

    Lexer()->NextToken();  // eat 'this'

    return thisType;
}

ir::TypeNode *ETSParser::ParseTypeAnnotation(TypeAnnotationParsingOptions *options)
{
    bool const throwError = ((*options) & TypeAnnotationParsingOptions::THROW_ERROR) != 0;

    auto [typeAnnotation, needFurtherProcessing] = GetTypeAnnotationFromToken(options);

    if (typeAnnotation == nullptr) {
        if (throwError) {
            ThrowSyntaxError("Invalid Type");
        }
        return nullptr;
    }

    if (!needFurtherProcessing) {
        return typeAnnotation;
    }

    const lexer::SourcePosition &startPos = Lexer()->GetToken().Start();

    while (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET) {
        Lexer()->NextToken();  // eat '['

        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET) {
            if (throwError) {
                ThrowExpectedToken(lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET);
            }
            return nullptr;
        }

        Lexer()->NextToken();  // eat ']'
        typeAnnotation = AllocNode<ir::TSArrayType>(typeAnnotation);
        typeAnnotation->SetRange({startPos, Lexer()->GetToken().End()});
    }

    if (((*options) & TypeAnnotationParsingOptions::DISALLOW_UNION) == 0 &&
        Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_BITWISE_OR) {
        return ParseUnionType(typeAnnotation);
    }

    return typeAnnotation;
}

}  // namespace ark::es2panda::parser