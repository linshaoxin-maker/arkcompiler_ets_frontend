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

#include "ETSparser.h"
#include "lexer/lexer.h"
#include "ir/astNode.h"
#include "ir/typeNode.h"
#include "ir/statements/etsNamespace.h"
#include "utils/arena_containers.h"

namespace ark::es2panda::parser {
class Expression;

using namespace std::literals::string_literals;

ir::ETSNamespace *ETSParser::ParseNamespace(ir::ModifierFlags flags)
{
    if ((GetContext().Status() & ParserStatus::IN_NAMESPACE) == 0) {
        LogSyntaxError("Namespace not enabled in here.");
    }
    ir::ETSNamespace *ns = ParseNamespaceImp(flags);
    return ns;
}

ir::ETSNamespace *ETSParser::ParseNamespaceImp(ir::ModifierFlags flags)
{
    Lexer()->NextToken();
    flags |= ir::ModifierFlags::DECLARE;
    auto opt = TypeAnnotationParsingOptions::NO_OPTS;
    ir::Expression *expr = ParseTypeReference(&opt);
    ExpectToken(lexer::TokenType::PUNCTUATOR_LEFT_BRACE);
    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
    while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        if (Lexer()->GetToken().Type() == lexer::TokenType::EOS) {
            LogSyntaxError("Unexpected token, expected '}'");
            break;
        }
        if (Lexer()->TryEatTokenType(lexer::TokenType::PUNCTUATOR_SEMI_COLON)) {
            continue;
        }
        auto stmt = ParseTopLevelStatement();
        if (stmt != nullptr) {
            statements.emplace_back(stmt);
        }
    }
    Lexer()->NextToken();
    auto ns = AllocNode<ir::ETSNamespace>(Allocator(), std::move(statements), expr);
    return ns;
}


}  // namespace ark::es2panda::parser
