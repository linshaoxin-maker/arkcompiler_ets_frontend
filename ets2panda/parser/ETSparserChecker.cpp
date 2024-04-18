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
#include "ir/ets/etsTuple.h"

namespace ark::es2panda::parser {

std::pair<bool, std::size_t> ETSParser::CheckDefaultParameters(const ir::ScriptFunction *const function) const
{
    bool hasDefaultParameter = false;
    bool hasRestParameter = false;
    std::size_t requiredParametersNumber = 0U;

    for (auto *const it : function->Params()) {
        auto const *const param = it->AsETSParameterExpression();

        if (param->IsRestParameter()) {
            hasRestParameter = true;
            continue;
        }

        if (hasRestParameter) {
            ThrowSyntaxError("Rest parameter should be the last one.", param->Start());
        }

        if (param->IsDefault()) {
            hasDefaultParameter = true;
            continue;
        }

        if (hasDefaultParameter) {
            ThrowSyntaxError("Required parameter follows default parameter(s).", param->Start());
        }

        ++requiredParametersNumber;
    }

    if (hasDefaultParameter && hasRestParameter) {
        ThrowSyntaxError("Both optional and rest parameters are not allowed in function's parameter list.",
                         function->Start());
    }

    return std::make_pair(hasDefaultParameter, requiredParametersNumber);
}

bool ETSParser::CheckModuleAsModifier()
{
    if ((Lexer()->GetToken().Flags() & lexer::TokenFlags::HAS_ESCAPE) != 0U) {
        ThrowSyntaxError("Escape sequences are not allowed in 'as' keyword");
    }

    return true;
}

bool ETSParser::CheckClassElement(ir::AstNode *property, [[maybe_unused]] ir::MethodDefinition *&ctor,
                                  [[maybe_unused]] ArenaVector<ir::AstNode *> &properties)
{
    if (property->IsClassStaticBlock()) {
        if (std::any_of(properties.cbegin(), properties.cend(),
                        [](const auto *prop) { return prop->IsClassStaticBlock(); })) {
            ThrowSyntaxError("Only one static block is allowed", property->Start());
        }

        auto *id = AllocNode<ir::Identifier>(compiler::Signatures::CCTOR, Allocator());
        property->AsClassStaticBlock()->Function()->SetIdent(id);
    }

    if (property->IsTSInterfaceBody()) {
        return CheckClassElementInterfaceBody(property, properties);
    }

    if (!property->IsMethodDefinition()) {
        return false;
    }

    auto const *const method = property->AsMethodDefinition();
    auto const *const function = method->Function();

    //  Check the special '$_get' and '$_set' methods using for object's index access
    if (method->Kind() == ir::MethodDefinitionKind::METHOD) {
        CheckPredefinedMethods(function, property->Start());
    }

    return false;  // resolve overloads later on scopes stage
}

void ETSParser::CheckPredefinedMethods(ir::ScriptFunction const *function, const lexer::SourcePosition &position) const
{
    auto const name = function->Id()->Name();

    auto const checkAsynchronous = [this, function, &name, &position]() -> void {
        if (function->IsAsyncFunc()) {
            ThrowSyntaxError(std::string {ir::PREDEFINED_METHOD} + std::string {name.Utf8()} +
                                 std::string {"' cannot be asynchronous."},
                             position);
        }
    };

    if (name.Is(compiler::Signatures::GET_INDEX_METHOD)) {
        checkAsynchronous();

        bool isValid = function->Params().size() == 1U;
        if (isValid) {
            auto const *const param = function->Params()[0]->AsETSParameterExpression();
            isValid = !param->IsDefault() && !param->IsRestParameter();
        }

        if (!isValid) {
            ThrowSyntaxError(std::string {ir::PREDEFINED_METHOD} + std::string {name.Utf8()} +
                                 std::string {"' should have exactly one required parameter."},
                             position);
        }
    } else if (name.Is(compiler::Signatures::SET_INDEX_METHOD)) {
        checkAsynchronous();

        bool isValid = function->Params().size() == 2U;
        if (isValid) {
            auto const *const param1 = function->Params()[0]->AsETSParameterExpression();
            auto const *const param2 = function->Params()[1]->AsETSParameterExpression();
            isValid = !param1->IsDefault() && !param1->IsRestParameter() && !param2->IsDefault() &&
                      !param2->IsRestParameter();
        }

        if (!isValid) {
            ThrowSyntaxError(std::string {ir::PREDEFINED_METHOD} + std::string {name.Utf8()} +
                                 std::string {"' should have exactly two required parameters."},
                             position);
        }
    } else if (name.Is(compiler::Signatures::ITERATOR_METHOD)) {
        checkAsynchronous();

        if (!function->Params().empty()) {
            ThrowSyntaxError(std::string {ir::PREDEFINED_METHOD} + std::string {name.Utf8()} +
                                 std::string {"' should not have parameters."},
                             position);
        }
    }
}

void ETSParser::CheckDeclare()
{
    ASSERT(Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_DECLARE);

    if (InAmbientContext()) {
        ThrowSyntaxError("A 'declare' modifier cannot be used in an already ambient context.");
    }

    GetContext().Status() |= ParserStatus::IN_AMBIENT_CONTEXT;

    Lexer()->NextToken();  // eat 'declare'

    switch (Lexer()->GetToken().KeywordType()) {
        case lexer::TokenType::KEYW_LET:
        case lexer::TokenType::KEYW_CONST:
        case lexer::TokenType::KEYW_FUNCTION:
        case lexer::TokenType::KEYW_CLASS:
        case lexer::TokenType::KEYW_NAMESPACE:
        case lexer::TokenType::KEYW_ENUM:
        case lexer::TokenType::KEYW_TYPE:
        case lexer::TokenType::KEYW_ABSTRACT:
        case lexer::TokenType::KEYW_INTERFACE: {
            return;
        }
        default: {
            ThrowSyntaxError("Unexpected token.");
        }
    }
}

void ETSParser::ValidateLabeledStatement(lexer::TokenType type)
{
    if (type != lexer::TokenType::KEYW_DO && type != lexer::TokenType::KEYW_WHILE &&
        type != lexer::TokenType::KEYW_FOR && type != lexer::TokenType::KEYW_SWITCH) {
        ThrowSyntaxError("Label must be followed by a loop statement", Lexer()->GetToken().Start());
    }
}

void ETSParser::ValidateRestParameter(ir::Expression *param)
{
    if (param->IsETSParameterExpression()) {
        if (param->AsETSParameterExpression()->IsRestParameter()) {
            GetContext().Status() |= ParserStatus::HAS_COMPLEX_PARAM;

            if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
                ThrowSyntaxError("Rest parameter must be the last formal parameter.");
            }
        }
    }
}

void ETSParser::ValidateForInStatement()
{
    ThrowUnexpectedToken(lexer::TokenType::KEYW_IN);
}

void ETSParser::ValidateInstanceOfExpression(ir::Expression *expr)
{
    ValidateGroupedExpression(expr);
    lexer::TokenType tokenType = Lexer()->GetToken().Type();
    if (tokenType == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        auto options = TypeAnnotationParsingOptions::NO_OPTS;

        // Run checks to validate type declarations
        // Should provide helpful messages with incorrect declarations like the following:
        // `instanceof A<String;`
        ParseTypeParameterDeclaration(&options);

        // Display error message even when type declaration is correct
        // `instanceof A<String>;`
        ThrowSyntaxError("Invalid right-hand side in 'instanceof' expression");
    }
}

}  // namespace ark::es2panda::parser