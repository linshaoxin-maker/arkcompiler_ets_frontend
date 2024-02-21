/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "enumConstantExpressionLowering.h"

#include "checker/ETSchecker.h"
#include "ir/astNode.h"
#include "compiler/core/compilerContext.h"

namespace ark::es2panda::compiler {
namespace {
std::optional<int32_t> SearchInEnum(ArenaVector<ir::AstNode *> members, const util::StringView &name)
{
    for (auto &member : members) {
        if (std::string(member->AsTSEnumMember()->Name()) == std::string(name)) {
            return member->AsTSEnumMember()->Init()->AsNumberLiteral()->Number().GetInt();
        }
    }
    return {};
}

int32_t GetIntValFromNode(const ir::Expression *node, ir::AstNode *declNode)
{
    util::StringView name;
    if (node->IsMemberExpression()) {
        name = node->AsMemberExpression()->Property()->AsIdentifier()->Name();
    } else {
        name = node->AsIdentifier()->Name();
    }

    if (declNode->IsClassProperty()) {
        return declNode->AsClassProperty()->Value()->AsNumberLiteral()->Number().GetInt();
    }
    if (declNode->IsTSEnumDeclaration()) {
        auto val = SearchInEnum(declNode->AsTSEnumDeclaration()->Members(), name);
        if (val.has_value()) {
            return val.value();
        }
    }
    UNREACHABLE();
}

int32_t GetValFromNode(const ir::Expression *node, parser::Program *const program, ir::AstNode *ast)
{
    if (node->IsIdentifier()) {
        // Search in current enum
        std::optional<int32_t> val = SearchInEnum(ast->AsTSEnumMember()->Parent()->AsTSEnumDeclaration()->Members(),
                                                  node->AsIdentifier()->Name());
        if (val.has_value()) {
            return val.value();
        }

        auto localScope = program->VarBinder()
                              ->GetScope()
                              ->Bindings()
                              .find(ast->AsTSEnumMember()->Parent()->AsTSEnumDeclaration()->Key()->Name())
                              ->second;
        for (auto &decl : localScope->GetScope()->Decls()) {
            if (std::string(decl->Name()) == std::string(node->AsIdentifier()->Name())) {
                return GetIntValFromNode(node, decl->Node());
            }
        }
    } else if (node->IsMemberExpression()) {
        // Search in local scope variable
        auto localScope = program->VarBinder()
                              ->GetScope()
                              ->Bindings()
                              .find(ast->AsTSEnumMember()->Parent()->AsTSEnumDeclaration()->Key()->Name())
                              ->second;
        for (auto &decl : localScope->GetScope()->Decls()) {
            if (std::string(decl->Name()) ==
                std::string(node->AsMemberExpression()->Object()->AsIdentifier()->Name())) {
                return GetIntValFromNode(node, decl->Node());
            }
        }
    } else if (node->IsNumberLiteral()) {
        return node->AsNumberLiteral()->Number().GetInt();
    }
    UNREACHABLE();
}

uint32_t ToUInt(double num)
{
    if (num >= std::numeric_limits<uint32_t>::min() && num <= std::numeric_limits<uint32_t>::max()) {
        return static_cast<int32_t>(num);
    }
    return 0;
}

int32_t ToInt(double num)
{
    if (num >= std::numeric_limits<int32_t>::min() && num <= std::numeric_limits<int32_t>::max()) {
        return static_cast<int32_t>(num);
    }
    return 0;
}

varbinder::EnumMemberResult GetOperationResulForDouble(lexer::TokenType type, varbinder::EnumMemberResult left,
                                                       varbinder::EnumMemberResult right)
{
    switch (type) {
        case lexer::TokenType::PUNCTUATOR_BITWISE_OR: {
            return static_cast<double>(ToUInt(std::get<double>(left)) | ToUInt(std::get<double>(right)));
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_AND: {
            return static_cast<double>(ToUInt(std::get<double>(left)) & ToUInt(std::get<double>(right)));
        }
        case lexer::TokenType::PUNCTUATOR_BITWISE_XOR: {
            return static_cast<double>(ToUInt(std::get<double>(left)) ^ ToUInt(std::get<double>(right)));
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT: {  // NOLINTNEXTLINE(hicpp-signed-bitwise)
            return static_cast<double>(ToInt(std::get<double>(left)) << ToUInt(std::get<double>(right)));
        }
        case lexer::TokenType::PUNCTUATOR_RIGHT_SHIFT: {  // NOLINTNEXTLINE(hicpp-signed-bitwise)
            return static_cast<double>(ToInt(std::get<double>(left)) >> ToUInt(std::get<double>(right)));
        }
        case lexer::TokenType::PUNCTUATOR_UNSIGNED_RIGHT_SHIFT: {
            return static_cast<double>(ToUInt(std::get<double>(left)) >> ToUInt(std::get<double>(right)));
        }
        case lexer::TokenType::PUNCTUATOR_PLUS: {
            return std::get<double>(left) + std::get<double>(right);
        }
        case lexer::TokenType::PUNCTUATOR_MINUS: {
            return std::get<double>(left) - std::get<double>(right);
        }
        case lexer::TokenType::PUNCTUATOR_MULTIPLY: {
            return std::get<double>(left) * std::get<double>(right);
        }
        case lexer::TokenType::PUNCTUATOR_DIVIDE: {
            return std::get<double>(left) / std::get<double>(right);
        }
        case lexer::TokenType::PUNCTUATOR_MOD: {
            return std::fmod(std::get<double>(left), std::get<double>(right));
        }
        case lexer::TokenType::PUNCTUATOR_EXPONENTIATION: {
            return std::pow(std::get<double>(left), std::get<double>(right));
        }
        default: {
            return false;
        }
    }
}
}  // namespace

bool EnumConstantExpressionLowering::Perform(public_lib::Context *const ctx, parser::Program *const program)
{
    for (auto &[_, ext_programs] : program->ExternalSources()) {
        for (auto *extProg : ext_programs) {
            Perform(ctx, extProg);
        }
    }

    checker::ETSChecker *checker = ctx->checker->AsETSChecker();
    program->Ast()->TransformChildrenRecursively([checker, program](ir::AstNode *ast) -> ir::AstNode* {
        UNUSED_VAR(checker);
        if (ast->IsTSEnumMember()) {
            auto init = ast->AsTSEnumMember()->Init();
            auto *allocator = checker->Allocator();
            if (init->IsIdentifier()) {
                auto number =
                    allocator->New<ir::NumberLiteral>(ark::es2panda::lexer::Number(GetValFromNode(init, program, ast)));
                number->SetParent(ast);
                ast->AsTSEnumMember()->SetInit(number);
                return ast;
            }
            if (init->IsMemberExpression()) {
                auto number =
                    allocator->New<ir::NumberLiteral>(ark::es2panda::lexer::Number(GetValFromNode(init, program, ast)));
                number->SetParent(ast);
                ast->AsTSEnumMember()->SetInit(number);
                return ast;
            }
            if (init->IsBinaryExpression()) {
                auto left = init->AsBinaryExpression()->Left();
                auto right = init->AsBinaryExpression()->Right();
                int32_t leftVal = GetValFromNode(left, program, ast);
                int32_t rightVal = GetValFromNode(right, program, ast);

                varbinder::EnumMemberResult evalExpr =
                    GetOperationResulForDouble(init->AsBinaryExpression()->OperatorType(), static_cast<double>(leftVal),
                                               static_cast<double>(rightVal));

                auto number = allocator->New<ir::NumberLiteral>(
                    ark::es2panda::lexer::Number(std::holds_alternative<double>(evalExpr)));
                number->SetParent(ast);
                ast->AsTSEnumMember()->SetInit(number);
                return ast;
            }
        }
        return ast;
    });

    return true;
}

}  // namespace ark::es2panda::compiler