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

ir::AstNode *ETSParser::ParseInnerTypeDeclaration(ir::ModifierFlags memberModifiers, lexer::LexerPosition savedPos,
                                                  bool isStepToken, bool seenStatic)
{
    if ((GetContext().Status() & ParserStatus::IN_NAMESPACE) == 0) {
        ThrowSyntaxError("Local type declaration (class, struct, interface and enum) support is not yet implemented.");
    }

    // remove saved_pos nolint
    Lexer()->Rewind(savedPos);
    if (isStepToken) {
        Lexer()->NextToken();
    }

    Lexer()->GetToken().SetTokenType(Lexer()->GetToken().KeywordType());
    ir::AstNode *typeDecl = ParseTypeDeclaration(true);
    memberModifiers &= (ir::ModifierFlags::PUBLIC | ir::ModifierFlags::PROTECTED | ir::ModifierFlags::PRIVATE |
                        ir::ModifierFlags::INTERNAL);
    typeDecl->AddModifier(memberModifiers);

    if (!seenStatic) {
        if (typeDecl->IsClassDeclaration()) {
            typeDecl->AsClassDeclaration()->Definition()->AsClassDefinition()->SetInnerModifier();
        } else if (typeDecl->IsETSStructDeclaration()) {
            typeDecl->AsETSStructDeclaration()->Definition()->AsClassDefinition()->SetInnerModifier();
        }
    }

    return typeDecl;
}

ir::Statement *ETSParser::ParseTypeDeclaration(bool allowStatic)
{
    auto savedPos = Lexer()->Save();

    auto modifiers = ir::ClassDefinitionModifiers::ID_REQUIRED | ir::ClassDefinitionModifiers::CLASS_DECL;

    auto tokenType = Lexer()->GetToken().Type();
    switch (tokenType) {
        case lexer::TokenType::KEYW_STATIC: {
            if (!allowStatic) {
                ThrowUnexpectedToken(Lexer()->GetToken().Type());
            }

            Lexer()->NextToken();

            if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_INTERFACE) {
                return ParseInterfaceDeclaration(true);
            }

            Lexer()->Rewind(savedPos);
            [[fallthrough]];
        }
        case lexer::TokenType::KEYW_ABSTRACT:
        case lexer::TokenType::KEYW_FINAL: {
            return ParseTypeDeclarationAbstractFinal(allowStatic, modifiers);
        }
        case lexer::TokenType::KEYW_ENUM: {
            return ParseEnumDeclaration(false);
        }
        case lexer::TokenType::KEYW_INTERFACE: {
            return ParseInterfaceDeclaration(false);
        }
        case lexer::TokenType::KEYW_NAMESPACE: {
            if (!InAmbientContext()) {
                ThrowSyntaxError("Namespaces are declare only");
            }
            GetContext().Status() |= ParserStatus::IN_NAMESPACE;
            auto *ns = ParseClassDeclaration(modifiers, ir::ModifierFlags::STATIC);
            GetContext().Status() &= ~ParserStatus::IN_NAMESPACE;
            return ns;
        }
        case lexer::TokenType::KEYW_CLASS: {
            return ParseClassDeclaration(modifiers);
        }
        case lexer::TokenType::KEYW_TYPE: {
            return ParseTypeAliasDeclaration();
        }
        case lexer::TokenType::LITERAL_IDENT: {
            if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_STRUCT) {
                return ParseStructDeclaration(modifiers);
            }
            [[fallthrough]];
        }
        case lexer::TokenType::LITERAL_NUMBER:
        case lexer::TokenType::LITERAL_NULL:
        case lexer::TokenType::KEYW_UNDEFINED:
        case lexer::TokenType::LITERAL_STRING:
        case lexer::TokenType::LITERAL_FALSE:
        case lexer::TokenType::LITERAL_TRUE:
        case lexer::TokenType::LITERAL_CHAR: {
            std::string errMsg("Cannot used in global scope '");

            std::string text = tokenType == lexer::TokenType::LITERAL_CHAR
                                   ? util::Helpers::UTF16toUTF8(Lexer()->GetToken().Utf16())
                                   : Lexer()->GetToken().Ident().Mutf8();

            if ((Lexer()->GetToken().Flags() & lexer::TokenFlags::HAS_ESCAPE) == 0) {
                errMsg.append(text);
            } else {
                errMsg.append(util::Helpers::CreateEscapedString(text));
            }

            errMsg.append("'");
            ThrowSyntaxError(errMsg.c_str());
        }
        default: {
            ThrowUnexpectedToken(Lexer()->GetToken().Type());
        }
    }
}

ir::TSTypeAliasDeclaration *ETSParser::ParseTypeAliasDeclaration()
{
    ASSERT(Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_TYPE);

    if ((GetContext().Status() & parser::ParserStatus::FUNCTION) != 0U) {
        ThrowSyntaxError("Type alias is allowed only as top-level declaration");
    }

    lexer::SourcePosition typeStart = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat type keyword

    if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
        ThrowSyntaxError("Identifier expected");
    }

    if (Lexer()->GetToken().IsReservedTypeName()) {
        std::string errMsg("Type alias name cannot be '");
        errMsg.append(TokenToString(Lexer()->GetToken().KeywordType()));
        errMsg.append("'");
        ThrowSyntaxError(errMsg.c_str());
    }

    const util::StringView ident = Lexer()->GetToken().Ident();
    auto *id = AllocNode<ir::Identifier>(ident, Allocator());
    id->SetRange(Lexer()->GetToken().Loc());

    auto *typeAliasDecl = AllocNode<ir::TSTypeAliasDeclaration>(Allocator(), id);

    Lexer()->NextToken();  // eat alias name

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        auto options =
            TypeAnnotationParsingOptions::THROW_ERROR | TypeAnnotationParsingOptions::ALLOW_DECLARATION_SITE_VARIANCE;
        ir::TSTypeParameterDeclaration *params = ParseTypeParameterDeclaration(&options);
        typeAliasDecl->SetTypeParameters(params);
        params->SetParent(typeAliasDecl);
    }

    if (!Lexer()->TryEatTokenType(lexer::TokenType::PUNCTUATOR_SUBSTITUTION)) {
        ThrowSyntaxError("'=' expected");
    }

    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
    ir::TypeNode *typeAnnotation = ParseTypeAnnotation(&options);
    typeAliasDecl->SetTsTypeAnnotation(typeAnnotation);
    typeAliasDecl->SetRange({typeStart, Lexer()->GetToken().End()});
    typeAnnotation->SetParent(typeAliasDecl);

    return typeAliasDecl;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Statement *ETSParser::ParseEnumDeclaration(bool isConst, bool isStatic)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::KEYW_ENUM);

    if ((GetContext().Status() & parser::ParserStatus::FUNCTION) != 0U) {
        ThrowSyntaxError("Local enum declaration support is not yet implemented.");
    }

    lexer::SourcePosition enumStart = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat enum keyword

    auto *key = ExpectIdentifier(false, true);

    auto *declNode = ParseEnumMembers(key, enumStart, isConst, isStatic);

    return declNode;
}

void ETSParser::ThrowIfVarDeclaration(VariableParsingFlags flags)
{
    if ((flags & VariableParsingFlags::VAR) != 0) {
        ThrowUnexpectedToken(lexer::TokenType::KEYW_VAR);
    }
}

ir::ETSPackageDeclaration *ETSParser::ParsePackageDeclaration()
{
    auto startLoc = Lexer()->GetToken().Start();

    if (Lexer()->GetToken().Type() != lexer::TokenType::KEYW_PACKAGE) {
        if (!IsETSModule() && GetProgram()->IsEntryPoint()) {
            // NOTE(rsipka): consider adding a filename name as module name to entry points as well
            importPathManager_->InsertModuleInfo(GetProgram()->AbsoluteName(),
                                                 util::ImportPathManager::ModuleInfo {util::StringView(""), false});
            return nullptr;
        }
        importPathManager_->InsertModuleInfo(GetProgram()->AbsoluteName(),
                                             util::ImportPathManager::ModuleInfo {GetProgram()->FileName(), false});
        GetProgram()->SetPackageName(GetProgram()->FileName());
        return nullptr;
    }

    Lexer()->NextToken();

    ir::Expression *name = ParseQualifiedName();

    auto *packageDeclaration = AllocNode<ir::ETSPackageDeclaration>(name);
    packageDeclaration->SetRange({startLoc, Lexer()->GetToken().End()});

    ConsumeSemicolon(packageDeclaration);

    auto packageName =
        name->IsIdentifier() ? name->AsIdentifier()->Name() : name->AsTSQualifiedName()->ToString(Allocator());

    GetProgram()->SetPackageName(packageName);
    // NOTE(rsipka): handle these two cases, check that is it really required
    importPathManager_->InsertModuleInfo(GetProgram()->AbsoluteName(),
                                         util::ImportPathManager::ModuleInfo {packageName, true});
    importPathManager_->InsertModuleInfo(GetProgram()->ResolvedFilePath(),
                                         util::ImportPathManager::ModuleInfo {packageName, true});

    return packageDeclaration;
}

ArenaVector<ir::ETSImportDeclaration *> ETSParser::ParseImportDeclarations()
{
    std::vector<std::string> userPaths;
    ArenaVector<ir::ETSImportDeclaration *> statements(Allocator()->Adapter());

    while (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_IMPORT) {
        auto startLoc = Lexer()->GetToken().Start();
        Lexer()->NextToken();  // eat import

        ArenaVector<ir::AstNode *> specifiers(Allocator()->Adapter());

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MULTIPLY) {
            ParseNameSpaceSpecifier(&specifiers);
        } else if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
            auto specs = ParseNamedSpecifiers();
            specifiers = util::Helpers::ConvertVector<ir::AstNode>(specs);
        } else {
            ParseImportDefaultSpecifier(&specifiers);
        }

        ir::ImportSource *importSource = ParseSourceFromClause(true);

        lexer::SourcePosition endLoc = importSource->Source()->End();
        auto *importDeclaration = AllocNode<ir::ETSImportDeclaration>(importSource, std::move(specifiers));
        importDeclaration->SetRange({startLoc, endLoc});

        ConsumeSemicolon(importDeclaration);

        statements.push_back(importDeclaration);
    }

    std::sort(statements.begin(), statements.end(), [](const auto *s1, const auto *s2) -> bool {
        return s1->Specifiers()[0]->IsImportNamespaceSpecifier() && !s2->Specifiers()[0]->IsImportNamespaceSpecifier();
    });

    return statements;
}

ir::Statement *ETSParser::ParseImportDeclaration([[maybe_unused]] StatementParsingFlags flags)
{
    char32_t nextChar = Lexer()->Lookahead();
    if (nextChar == lexer::LEX_CHAR_LEFT_PAREN || nextChar == lexer::LEX_CHAR_DOT) {
        return ParseExpressionStatement();
    }

    lexer::SourcePosition startLoc = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat import

    ArenaVector<ir::AstNode *> specifiers(Allocator()->Adapter());

    ir::ImportSource *importSource = nullptr;

    if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_STRING) {
        ir::AstNode *astNode = ParseImportSpecifiers(&specifiers);
        if (astNode != nullptr) {
            ASSERT(astNode->IsTSImportEqualsDeclaration());
            astNode->SetRange({startLoc, Lexer()->GetToken().End()});
            ConsumeSemicolon(astNode->AsTSImportEqualsDeclaration());
            return astNode->AsTSImportEqualsDeclaration();
        }
        importSource = ParseSourceFromClause(true);
    } else {
        importSource = ParseSourceFromClause(false);
    }

    lexer::SourcePosition endLoc = importSource->Source()->End();
    auto *importDeclaration = AllocNode<ir::ETSImportDeclaration>(importSource, std::move(specifiers));
    importDeclaration->SetRange({startLoc, endLoc});

    ConsumeSemicolon(importDeclaration);

    return importDeclaration;
}

ir::Statement *ETSParser::ParseExportDeclaration([[maybe_unused]] StatementParsingFlags flags)
{
    ThrowUnexpectedToken(lexer::TokenType::KEYW_EXPORT);
}

}  // namespace ark::es2panda::parser