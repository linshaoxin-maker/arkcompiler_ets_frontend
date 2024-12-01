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
    ir::ETSNamespace *result = AllocNode<ir::ETSNamespace>(Allocator(), ArenaVector<ir::Statement *>(Allocator()->Adapter()), ExpectIdentifier());
    ir::ETSNamespace *parent = result;
    ir::ETSNamespace *child = nullptr;
    while (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_PERIOD) {
        Lexer()->NextToken();
        child = AllocNode<ir::ETSNamespace>(Allocator(), ArenaVector<ir::Statement *>(Allocator()->Adapter()), ExpectIdentifier());
        child->SetParent(parent);
        parent->Statements().emplace_back(child);
        parent = child;
    }
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
        if (stmt == nullptr) {
            LogSyntaxError("Failed to parse statement.");
            break;
        } else {
            statements.emplace_back(stmt); 
        }
    }
    Lexer()->NextToken();
    if (child != nullptr) {
        child->SetStatements(std::move(statements));
    } else {
        result->SetStatements(std::move(statements));
    }
    return result;
}

}  // namespace ark::es2panda::parser
