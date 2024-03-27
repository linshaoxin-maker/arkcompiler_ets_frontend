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
#include "ir/expressions/updateExpression.h"
#include "ir/ets/etsTuple.h"
#include "ir/ets/etsNullishTypes.h"
#include "ir/ets/etsUnionType.h"
#include "ir/ets/etsParameterExpression.h"

namespace ark::es2panda::parser {

void ETSParser::AddExternalSource(const std::vector<Program *> &programs)
{
    for (auto *newProg : programs) {
        auto &extSources = globalProgram_->ExternalSources();

        const util::StringView name =
            newProg->Ast()->Statements().empty() ? newProg->FileName() : newProg->GetPackageName();
        if (extSources.count(name) == 0) {
            extSources.emplace(name, Allocator()->Adapter());
        }
        extSources.at(name).emplace_back(newProg);
    }
}

std::string ETSParser::GetNameForTypeNode(const ir::TypeNode *typeAnnotation) const
{
    if (typeAnnotation->IsETSUnionType()) {
        return GetNameForETSUnionType(typeAnnotation);
    }
    if (typeAnnotation->IsETSPrimitiveType()) {
        return PrimitiveTypeToName(typeAnnotation->AsETSPrimitiveType()->GetPrimitiveType());
    }

    if (typeAnnotation->IsETSTypeReference()) {
        std::string typeParamNames;
        auto typeParam = typeAnnotation->AsETSTypeReference()->Part()->TypeParams();
        if (typeParam != nullptr && typeParam->IsTSTypeParameterInstantiation()) {
            typeParamNames = "<";
            auto paramList = typeParam->Params();
            for (auto param : paramList) {
                std::string typeParamName = GetNameForTypeNode(param);
                typeParamNames += typeParamName + ",";
            }
            typeParamNames.pop_back();
            typeParamNames += ">";
        }
        return typeAnnotation->AsETSTypeReference()->Part()->Name()->AsIdentifier()->Name().Mutf8() + typeParamNames;
    }

    if (typeAnnotation->IsETSFunctionType()) {
        std::string lambdaParams = " ";

        for (const auto *const param : typeAnnotation->AsETSFunctionType()->Params()) {
            lambdaParams += param->AsETSParameterExpression()->Ident()->Name().Mutf8();
            lambdaParams += ":";
            lambdaParams += GetNameForTypeNode(param->AsETSParameterExpression()->Ident()->TypeAnnotation());
            lambdaParams += ",";
        }

        lambdaParams.pop_back();
        const std::string returnTypeName = GetNameForTypeNode(typeAnnotation->AsETSFunctionType()->ReturnType());

        return "((" + lambdaParams + ") => " + returnTypeName + ")";
    }

    if (typeAnnotation->IsTSArrayType()) {
        // Note! array is required for the rest parameter.
        return GetNameForTypeNode(typeAnnotation->AsTSArrayType()->ElementType()) + "[]";
    }

    if (typeAnnotation->IsETSNullType()) {
        return "null";
    }

    if (typeAnnotation->IsETSUndefinedType()) {
        return "undefined";
    }

    UNREACHABLE();
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

std::optional<lexer::SourcePosition> ETSParser::GetDefaultParamPosition(ArenaVector<ir::Expression *> params)
{
    for (auto &param : params) {
        if (param->IsETSParameterExpression() && param->AsETSParameterExpression()->IsDefault()) {
            return param->AsETSParameterExpression()->Initializer()->Start();
        }
    }
    return {};
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
        case lexer::TokenType::KEYW_VOID: {
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::VOID);
            break;
        }
        case lexer::TokenType::KEYW_BOOLEAN: {
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::BOOLEAN);
            break;
        }
        case lexer::TokenType::KEYW_BYTE: {
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::BYTE);
            break;
        }
        case lexer::TokenType::KEYW_CHAR: {
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::CHAR);
            break;
        }
        case lexer::TokenType::KEYW_DOUBLE: {
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::DOUBLE);
            break;
        }
        case lexer::TokenType::KEYW_FLOAT: {
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::FLOAT);
            break;
        }
        case lexer::TokenType::KEYW_INT: {
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::INT);
            break;
        }
        case lexer::TokenType::KEYW_LONG: {
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::LONG);
            break;
        }
        case lexer::TokenType::KEYW_SHORT: {
            typeAnnotation = ParsePrimitiveType(options, ir::PrimitiveType::SHORT);
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

            if (((*options) & TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE) == 0 &&
                (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS ||
                 Lexer()->Lookahead() == lexer::LEX_CHAR_COLON)) {
                typeAnnotation = ParseFunctionType();
                typeAnnotation->SetStart(startLoc);

                if (auto position = GetDefaultParamPosition(typeAnnotation->AsETSFunctionType()->Params())) {
                    ThrowSyntaxError("Default parameters can not be used in functional type", position.value());
                }

                return std::make_pair(typeAnnotation, false);
            }

            typeAnnotation = ParseTypeAnnotation(options);
            typeAnnotation->SetStart(startLoc);

            if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_BITWISE_OR) {
                typeAnnotation = ParseUnionType(typeAnnotation);
            }

            if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_QUESTION_MARK) {
                ThrowSyntaxError("Default parameters can not be used in functional type");
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

ir::AnnotatedExpression *ETSParser::GetAnnotatedExpressionFromParam()
{
    ir::AnnotatedExpression *parameter;

    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::LITERAL_IDENT: {
            parameter = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
            if (parameter->AsIdentifier()->Decorators().empty()) {
                parameter->SetRange(Lexer()->GetToken().Loc());
            } else {
                parameter->SetRange(
                    {parameter->AsIdentifier()->Decorators().front()->Start(), Lexer()->GetToken().End()});
            }
            break;
        }

        case lexer::TokenType::PUNCTUATOR_PERIOD_PERIOD_PERIOD: {
            const auto startLoc = Lexer()->GetToken().Start();
            Lexer()->NextToken();

            if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
                ThrowSyntaxError("Unexpected token, expected an identifier.");
            }

            auto *const restIdent = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
            restIdent->SetRange(Lexer()->GetToken().Loc());

            parameter = AllocNode<ir::SpreadElement>(ir::AstNodeType::REST_ELEMENT, Allocator(), restIdent);
            parameter->SetRange({startLoc, Lexer()->GetToken().End()});
            break;
        }

        default: {
            ThrowSyntaxError("Unexpected token, expected an identifier.");
        }
    }

    Lexer()->NextToken();
    return parameter;
}

ir::Expression *ETSParser::ResolveArgumentUnaryExpr(ExpressionParseFlags flags)
{
    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::PUNCTUATOR_PLUS:
        case lexer::TokenType::PUNCTUATOR_MINUS:
        case lexer::TokenType::PUNCTUATOR_TILDE:
        case lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK:
        case lexer::TokenType::PUNCTUATOR_DOLLAR_DOLLAR:
        case lexer::TokenType::PUNCTUATOR_PLUS_PLUS:
        case lexer::TokenType::PUNCTUATOR_MINUS_MINUS:
        case lexer::TokenType::KEYW_TYPEOF: {
            return ParseUnaryOrPrefixUpdateExpression();
        }
        default: {
            return ParseLeftHandSideExpression(flags);
        }
    }
}

bool ETSParser::IsStructKeyword() const
{
    return (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT &&
            Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_STRUCT);
}

std::unique_ptr<lexer::Lexer> ETSParser::InitLexer(const SourceFile &sourceFile)
{
    GetProgram()->SetSource(sourceFile);
    auto lexer = std::make_unique<lexer::ETSLexer>(&GetContext());
    SetLexer(lexer.get());
    return lexer;
}

}  // namespace ark::es2panda::parser