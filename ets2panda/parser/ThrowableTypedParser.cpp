/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <sstream>

#include "ThrowableTypedParser.h"
#include "lexer/lexer.h"

namespace ark::es2panda::parser {

void ThrowableTypedParser::ThrowUnexpectedToken(lexer::TokenType tokenType) const
{
    ThrowSyntaxError({"Unexpected token: '", lexer::TokenToString(tokenType), "'."});
}

void ThrowableTypedParser::ThrowSyntaxError(std::string_view errorMessage) const
{
    ThrowSyntaxError(errorMessage, Lexer()->GetToken().Start());
}

void ThrowableTypedParser::ThrowSyntaxError(std::initializer_list<std::string_view> list) const
{
    ThrowSyntaxError(list, Lexer()->GetToken().Start());
}

void ThrowableTypedParser::ThrowSyntaxError(std::initializer_list<std::string_view> list,
                                            const lexer::SourcePosition &pos) const
{
    std::stringstream ss;
    for (const auto &it : list) {
        ss << it;
    }

    std::string err = ss.str();
    ThrowSyntaxError(std::string_view {err}, pos);
}

void ThrowableTypedParser::ThrowSyntaxError(std::string_view errorMessage, const lexer::SourcePosition &pos) const
{
    lexer::LineIndex index(GetProgram()->SourceCode());
    lexer::SourceLocation loc = index.GetLocation(pos);

    throw Error {ErrorType::SYNTAX, GetProgram()->SourceFilePath().Utf8(), errorMessage, loc.line, loc.col};
}

}  // namespace ark::es2panda::parser
