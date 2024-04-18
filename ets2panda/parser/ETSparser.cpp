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
#include "parser/parserFlags.h"
#include "util/arktsconfig.h"
#include "util/helpers.h"
#include "util/language.h"
#include "varbinder/varbinder.h"
#include "varbinder/scope.h"
#include "varbinder/ETSBinder.h"
#include "lexer/lexer.h"
#include "ir/astNode.h"
#include "ir/base/catchClause.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/functionExpression.h"
#include "ir/statements/functionDeclaration.h"
#include "ir/statements/variableDeclarator.h"
#include "ir/expressions/arrayExpression.h"
#include "ir/expressions/assignmentExpression.h"
#include "ir/expressions/sequenceExpression.h"
#include "ir/expressions/callExpression.h"
#include "ir/expressions/superExpression.h"
#include "ir/expressions/typeofExpression.h"
#include "ir/expressions/memberExpression.h"
#include "ir/expressions/updateExpression.h"
#include "ir/expressions/arrowFunctionExpression.h"
#include "ir/expressions/unaryExpression.h"
#include "ir/expressions/yieldExpression.h"
#include "ir/expressions/awaitExpression.h"
#include "ir/expressions/literals/booleanLiteral.h"
#include "ir/expressions/literals/charLiteral.h"
#include "ir/expressions/literals/nullLiteral.h"
#include "ir/expressions/literals/numberLiteral.h"
#include "ir/expressions/literals/stringLiteral.h"
#include "ir/expressions/literals/undefinedLiteral.h"
#include "ir/expressions/templateLiteral.h"
#include "ir/expressions/objectExpression.h"
#include "ir/module/importDeclaration.h"
#include "ir/module/importDefaultSpecifier.h"
#include "ir/module/importSpecifier.h"
#include "ir/module/exportSpecifier.h"
#include "ir/module/exportNamedDeclaration.h"
#include "ir/statements/assertStatement.h"
#include "ir/statements/blockStatement.h"
#include "ir/statements/tryStatement.h"
#include "ir/statements/debuggerStatement.h"
#include "ir/ets/etsLaunchExpression.h"
#include "ir/ets/etsPrimitiveType.h"
#include "ir/ets/etsReExportDeclaration.h"
#include "ir/ets/etsWildcardType.h"
#include "ir/ets/etsNewArrayInstanceExpression.h"
#include "ir/ets/etsTuple.h"
#include "ir/ets/etsNewClassInstanceExpression.h"
#include "ir/ets/etsNewMultiDimArrayInstanceExpression.h"
#include "ir/ets/etsScript.h"
#include "ir/ets/etsTypeReference.h"
#include "ir/ets/etsTypeReferencePart.h"
#include "ir/ets/etsUnionType.h"
#include "ir/ets/etsImportDeclaration.h"
#include "ir/ets/etsStructDeclaration.h"
#include "ir/module/importNamespaceSpecifier.h"
#include "ir/ts/tsAsExpression.h"
#include "ir/ts/tsTypeParameterInstantiation.h"
#include "ir/ts/tsArrayType.h"
#include "ir/ts/tsTypeReference.h"
#include "ir/ts/tsTypeParameter.h"
#include "ir/ts/tsIntersectionType.h"
#include "ir/ts/tsFunctionType.h"
#include "ir/ts/tsNonNullExpression.h"

namespace ark::es2panda::parser {

void ETSParser::ParseProgram(ScriptKind kind)
{
    lexer::SourcePosition startLoc = Lexer()->GetToken().Start();
    Lexer()->NextToken();
    GetProgram()->SetKind(kind);

    if (GetProgram()->SourceFilePath().Utf8()[0] == '@') {
        // NOTE(user): handle multiple sourceFiles
    }

    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
    auto decl = ParsePackageDeclaration();
    if (decl != nullptr) {
        statements.emplace_back(decl);
    }
    auto script = ParseETSGlobalScript(startLoc, statements);

    AddExternalSource(ParseSources());
    GetProgram()->VarBinder()->AsETSBinder()->SetModuleList(this->ModuleList());
    GetProgram()->SetAst(script);
}

ir::ETSScript *ETSParser::ParseETSGlobalScript(lexer::SourcePosition startLoc, ArenaVector<ir::Statement *> &statements)
{
    auto imports = ParseImportDeclarations();
    statements.insert(statements.end(), imports.begin(), imports.end());

    auto topLevelStatements = ParseTopLevelDeclaration();
    statements.insert(statements.end(), topLevelStatements.begin(), topLevelStatements.end());

    auto *etsScript = AllocNode<ir::ETSScript>(Allocator(), std::move(statements), GetProgram());
    etsScript->SetRange({startLoc, Lexer()->GetToken().End()});
    return etsScript;
}

ArenaVector<ir::ETSImportDeclaration *> ETSParser::ParseDefaultSources(std::string_view srcFile,
                                                                       std::string_view importSrc)
{
    auto isp = InnerSourceParser(this);
    SourceFile source(srcFile, importSrc);
    auto lexer = InitLexer(source);

    Lexer()->NextToken();

    GetContext().Status() |= ParserStatus::IN_DEFAULT_IMPORTS;
    auto statements = ParseImportDeclarations();
    GetContext().Status() &= ~ParserStatus::IN_DEFAULT_IMPORTS;

    AddExternalSource(ParseSources());
    return statements;
}

std::vector<Program *> ETSParser::ParseSources()
{
    std::vector<Program *> programs;

    auto &parseList = importPathManager_->ParseList();

    // This parse list `paths` can grow in the meantime, so keep this index-based iteration
    // NOLINTNEXTLINE(modernize-loop-convert)
    for (size_t idx = 0; idx < parseList.size(); idx++) {
        // check if already parsed
        if (parseList[idx].isParsed) {
            continue;
        }
        std::ifstream inputStream(parseList[idx].sourcePath.Mutf8());
        const auto data = importPathManager_->GetImportData(parseList[idx].sourcePath, Extension());
        if (!data.hasDecl) {
            continue;
        }

        if (GetProgram()->SourceFilePath().Is(parseList[idx].sourcePath.Mutf8())) {
            break;
        }

        if (inputStream.fail()) {
            ThrowSyntaxError({"Failed to open file: ", parseList[idx].sourcePath.Mutf8()});
        }

        std::stringstream ss;
        ss << inputStream.rdbuf();
        auto externalSource = ss.str();

        auto currentLang = GetContext().SetLanguage(data.lang);
        auto extSrc = Allocator()->New<util::UString>(externalSource, Allocator());
        auto newProg = ParseSource(
            {parseList[idx].sourcePath.Utf8(), extSrc->View().Utf8(), parseList[idx].sourcePath.Utf8(), false});

        programs.emplace_back(newProg);
        GetContext().SetLanguage(currentLang);
    }

    return programs;
}

parser::Program *ETSParser::ParseSource(const SourceFile &sourceFile)
{
    importPathManager_->MarkAsParsed(sourceFile.filePath);
    auto *program = Allocator()->New<parser::Program>(Allocator(), GetProgram()->VarBinder());
    auto esp = ExternalSourceParser(this, program);
    auto lexer = InitLexer(sourceFile);

    lexer::SourcePosition startLoc = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
    auto decl = ParsePackageDeclaration();
    if (decl != nullptr) {
        statements.emplace_back(decl);
    }
    auto script = ParseETSGlobalScript(startLoc, statements);
    program->SetAst(script);
    return program;
}

ArenaVector<ir::Statement *> ETSParser::ParseTopLevelStatements()
{
    ArenaVector<ir::Statement *> statements(Allocator()->Adapter());
    while (Lexer()->GetToken().Type() != lexer::TokenType::EOS) {
        if (Lexer()->TryEatTokenType(lexer::TokenType::PUNCTUATOR_SEMI_COLON)) {
            continue;
        }
        auto stmt = ParseTopLevelStatement();
        GetContext().Status() &= ~ParserStatus::IN_AMBIENT_CONTEXT;
        if (stmt != nullptr) {
            statements.emplace_back(stmt);
        }
    }

    return statements;
}

ir::Statement *ETSParser::ParseTopLevelDeclStatement(StatementParsingFlags flags)
{
    auto [memberModifiers, startLoc] = ParseMemberModifiers();
    if ((memberModifiers & (ir::ModifierFlags::EXPORT | ir::ModifierFlags::DEFAULT_EXPORT)) != 0U &&
        (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MULTIPLY ||
         Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE)) {
        return ParseExport(startLoc, memberModifiers);
    }

    ir::Statement *result = nullptr;
    auto token = Lexer()->GetToken();
    switch (token.Type()) {
        case lexer::TokenType::KEYW_FUNCTION: {
            result = ParseFunctionDeclaration(false, memberModifiers);
            result->SetStart(startLoc);
            break;
        }
        case lexer::TokenType::KEYW_CONST: {
            memberModifiers |= ir::ModifierFlags::CONST;
            [[fallthrough]];
        }
        case lexer::TokenType::KEYW_LET: {
            result = ParseStatement(flags);
            break;
        }
        case lexer::TokenType::KEYW_NAMESPACE:
        case lexer::TokenType::KEYW_STATIC:
        case lexer::TokenType::KEYW_ABSTRACT:
        case lexer::TokenType::KEYW_FINAL:
        case lexer::TokenType::KEYW_ENUM:
        case lexer::TokenType::KEYW_INTERFACE:
        case lexer::TokenType::KEYW_CLASS: {
            result = ParseTypeDeclaration(false);
            break;
        }
        case lexer::TokenType::LITERAL_IDENT: {
            result = ParseIdentKeyword();
            break;
        }
        default: {
            break;
        }
    }
    if (result != nullptr) {
        result->AddModifier(memberModifiers);
    }
    return result;
}

ir::Statement *ETSParser::ParseTopLevelStatement()
{
    const auto flags = StatementParsingFlags::ALLOW_LEXICAL;
    static const std::unordered_set<lexer::TokenType> ALLOWED_TOP_LEVEL_STMTS = {
        lexer::TokenType::PUNCTUATOR_LEFT_BRACE,
        lexer::TokenType::PUNCTUATOR_SEMI_COLON,
        lexer::TokenType::KEYW_ASSERT,
        lexer::TokenType::KEYW_IF,
        lexer::TokenType::KEYW_DO,
        lexer::TokenType::KEYW_FOR,
        lexer::TokenType::KEYW_TRY,
        lexer::TokenType::KEYW_WHILE,
        lexer::TokenType::KEYW_BREAK,
        lexer::TokenType::KEYW_CONTINUE,
        lexer::TokenType::KEYW_THROW,
        lexer::TokenType::KEYW_SWITCH,
        lexer::TokenType::KEYW_DEBUGGER,
        lexer::TokenType::LITERAL_IDENT,
    };

    auto result = ParseTopLevelDeclStatement(flags);
    if (result == nullptr) {
        auto const tokenType = Lexer()->GetToken().Type();
        if (ALLOWED_TOP_LEVEL_STMTS.count(tokenType) != 0U) {
            result = ParseStatement(flags);
        } else {
            ThrowUnexpectedToken(tokenType);
        }
    }
    return result;
}

ArenaVector<ir::Statement *> ETSParser::ParseTopLevelDeclaration()
{
    auto topStatements = ParseTopLevelStatements();
    Lexer()->NextToken();
    return topStatements;
}

ir::Statement *ETSParser::ParseIdentKeyword()
{
    const auto token = Lexer()->GetToken();
    ASSERT(token.Type() == lexer::TokenType::LITERAL_IDENT);
    switch (token.KeywordType()) {
        case lexer::TokenType::KEYW_STRUCT: {
            return ParseTypeDeclaration(false);
        }
        case lexer::TokenType::KEYW_TYPE: {
            return ParseTypeAliasDeclaration();
        }
        default: {
            break;
        }
    }
    return nullptr;
}

ir::Expression *ETSParser::ParseLaunchExpression(ExpressionParseFlags flags)
{
    lexer::SourcePosition start = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat launch

    ir::Expression *expr = ParseLeftHandSideExpression(flags);
    if (!expr->IsCallExpression()) {
        ThrowSyntaxError("Only call expressions are allowed after 'launch'", expr->Start());
    }
    auto call = expr->AsCallExpression();
    auto *launchExpression = AllocNode<ir::ETSLaunchExpression>(call);
    launchExpression->SetRange({start, call->End()});

    return launchExpression;
}

std::string ETSParser::PrimitiveTypeToName(ir::PrimitiveType type)
{
    switch (type) {
        case ir::PrimitiveType::BYTE:
            return "byte";
        case ir::PrimitiveType::INT:
            return "int";
        case ir::PrimitiveType::LONG:
            return "long";
        case ir::PrimitiveType::SHORT:
            return "short";
        case ir::PrimitiveType::FLOAT:
            return "float";
        case ir::PrimitiveType::DOUBLE:
            return "double";
        case ir::PrimitiveType::BOOLEAN:
            return "boolean";
        case ir::PrimitiveType::CHAR:
            return "char";
        case ir::PrimitiveType::VOID:
            return "void";
        default:
            UNREACHABLE();
    }
}

std::string ETSParser::GetNameForETSUnionType(const ir::TypeNode *typeAnnotation) const
{
    ASSERT(typeAnnotation->IsETSUnionType());
    std::string newstr;
    for (size_t i = 0; i < typeAnnotation->AsETSUnionType()->Types().size(); i++) {
        auto type = typeAnnotation->AsETSUnionType()->Types()[i];
        std::string str = GetNameForTypeNode(type);
        newstr += str;
        if (i != typeAnnotation->AsETSUnionType()->Types().size() - 1) {
            newstr += "|";
        }
    }
    return newstr;
}

ir::AstNode *ETSParser::ParseTypeLiteralOrInterfaceMember()
{
    auto startLoc = Lexer()->GetToken().Start();
    ir::ModifierFlags methodFlags = ParseInterfaceMethodModifiers();

    if (methodFlags != ir::ModifierFlags::NONE) {
        if ((methodFlags & ir::ModifierFlags::PRIVATE) == 0) {
            methodFlags |= ir::ModifierFlags::PUBLIC;
        }

        auto *method = ParseInterfaceMethod(methodFlags, ir::MethodDefinitionKind::METHOD);
        method->SetStart(startLoc);
        return method;
    }

    if (Lexer()->Lookahead() != lexer::LEX_CHAR_LEFT_PAREN && Lexer()->Lookahead() != lexer::LEX_CHAR_LESS_THAN &&
        (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_GET ||
         Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_SET)) {
        return ParseInterfaceGetterSetterMethod(methodFlags);
    }

    if (Lexer()->TryEatTokenKeyword(lexer::TokenType::KEYW_READONLY)) {
        auto *field = ParseInterfaceField();
        field->SetStart(startLoc);
        field->AddModifier(ir::ModifierFlags::READONLY);
        return field;
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT) {
        char32_t nextCp = Lexer()->Lookahead();
        if (nextCp == lexer::LEX_CHAR_LEFT_PAREN || nextCp == lexer::LEX_CHAR_LESS_THAN) {
            auto *method = ParseInterfaceMethod(ir::ModifierFlags::PUBLIC, ir::MethodDefinitionKind::METHOD);
            method->SetStart(startLoc);
            return method;
        }

        auto *field = ParseInterfaceField();
        field->SetStart(startLoc);
        return field;
    }

    return ParseTypeDeclaration(true);
}

std::tuple<ir::Expression *, ir::TSTypeParameterInstantiation *> ETSParser::ParseTypeReferencePart(
    TypeAnnotationParsingOptions *options)
{
    ExpressionParseFlags flags = ExpressionParseFlags::NO_OPTS;

    if (((*options) & TypeAnnotationParsingOptions::POTENTIAL_CLASS_LITERAL) != 0) {
        flags |= ExpressionParseFlags::POTENTIAL_CLASS_LITERAL;
    }

    auto *typeName = ParseQualifiedName(flags);
    if (typeName == nullptr) {
        return {nullptr, nullptr};
    }

    if (((*options) & TypeAnnotationParsingOptions::POTENTIAL_CLASS_LITERAL) != 0 &&
        (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_CLASS || IsStructKeyword())) {
        return {typeName, nullptr};
    }

    ir::TSTypeParameterInstantiation *typeParamInst = nullptr;
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SHIFT ||
        Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SHIFT) {
            Lexer()->BackwardToken(lexer::TokenType::PUNCTUATOR_LESS_THAN, 1);
        }
        *options |= TypeAnnotationParsingOptions::ALLOW_WILDCARD;
        typeParamInst = ParseTypeParameterInstantiation(options);
        *options &= ~TypeAnnotationParsingOptions::ALLOW_WILDCARD;
    }

    return {typeName, typeParamInst};
}

ir::TypeNode *ETSParser::ParseTypeReference(TypeAnnotationParsingOptions *options)
{
    auto startPos = Lexer()->GetToken().Start();
    ir::ETSTypeReferencePart *typeRefPart = nullptr;

    while (true) {
        auto partPos = Lexer()->GetToken().Start();
        auto [typeName, typeParams] = ParseTypeReferencePart(options);
        if (typeName == nullptr) {
            return nullptr;
        }

        typeRefPart = AllocNode<ir::ETSTypeReferencePart>(typeName, typeParams, typeRefPart);
        typeRefPart->SetRange({partPos, Lexer()->GetToken().End()});

        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_PERIOD) {
            break;
        }

        Lexer()->NextToken();

        if (((*options) & TypeAnnotationParsingOptions::POTENTIAL_CLASS_LITERAL) != 0 &&
            (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_CLASS || IsStructKeyword())) {
            break;
        }
    }

    auto *typeReference = AllocNode<ir::ETSTypeReference>(typeRefPart);
    typeReference->SetRange({startPos, Lexer()->GetToken().End()});
    return typeReference;
}

ir::TypeNode *ETSParser::ParseBaseTypeReference(TypeAnnotationParsingOptions *options)
{
    ir::TypeNode *typeAnnotation = nullptr;

    switch (Lexer()->GetToken().KeywordType()) {
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

        default: {
            break;
        }
    }

    return typeAnnotation;
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

ir::TSIntersectionType *ETSParser::ParseIntersectionType(ir::Expression *type)
{
    auto startLoc = type->Start();
    ArenaVector<ir::Expression *> types(Allocator()->Adapter());
    types.push_back(type);
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;

    while (true) {
        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_BITWISE_AND) {
            break;
        }

        Lexer()->NextToken();  // eat '&'
        types.push_back(ParseTypeReference(&options));
    }

    lexer::SourcePosition endLoc = types.back()->End();
    auto *intersectionType = AllocNode<ir::TSIntersectionType>(std::move(types));
    intersectionType->SetRange({startLoc, endLoc});
    return intersectionType;
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

ir::TypeNode *ETSParser::ParseLiteralIdent(TypeAnnotationParsingOptions *options)
{
    if (const auto keyword = Lexer()->GetToken().KeywordType();
        keyword == lexer::TokenType::KEYW_IN || keyword == lexer::TokenType::KEYW_OUT) {
        return ParseWildcardType(options);
    }

    if (Lexer()->GetToken().IsDefinableTypeName()) {
        return GetTypeAnnotationOfPrimitiveType(Lexer()->GetToken().KeywordType(), options);
    }

    return ParseTypeReference(options);
}

void ETSParser::ParseRightParenthesis(TypeAnnotationParsingOptions *options, ir::TypeNode *&typeAnnotation,
                                      lexer::LexerPosition savedPos)
{
    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
        if (((*options) & TypeAnnotationParsingOptions::THROW_ERROR) != 0) {
            ThrowExpectedToken(lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS);
        }

        Lexer()->Rewind(savedPos);
        typeAnnotation = nullptr;
    } else {
        Lexer()->NextToken();  // eat ')'
    }
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

    if (((*options) & TypeAnnotationParsingOptions::ALLOW_INTERSECTION) != 0 &&
        Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_BITWISE_AND) {
        if (typeAnnotation->IsETSPrimitiveType()) {
            if (throwError) {
                ThrowSyntaxError("Invalid intersection type.");
            }
            return nullptr;
        }

        return ParseIntersectionType(typeAnnotation);
    }

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

ir::DebuggerStatement *ETSParser::ParseDebuggerStatement()
{
    ThrowUnexpectedToken(lexer::TokenType::KEYW_DEBUGGER);
}

ir::Statement *ETSParser::ParseExport(lexer::SourcePosition startLoc, ir::ModifierFlags modifiers)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MULTIPLY ||
           Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE);
    ArenaVector<ir::AstNode *> specifiers(Allocator()->Adapter());

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MULTIPLY) {
        ParseNameSpaceSpecifier(&specifiers, true);
    } else {
        auto specs = ParseNamedSpecifiers();

        if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_FROM) {
            specifiers = util::Helpers::ConvertVector<ir::AstNode>(specs);
        } else {
            ArenaVector<ir::ExportSpecifier *> exports(Allocator()->Adapter());
            for (auto spec : specs) {
                exports.emplace_back(AllocNode<ir::ExportSpecifier>(spec->Local(), spec->Imported()));
            }
            auto result = AllocNode<ir::ExportNamedDeclaration>(Allocator(), static_cast<ir::StringLiteral *>(nullptr),
                                                                std::move(exports));
            result->AddModifier(modifiers);
            return result;
        }
    }

    // re-export directive
    ir::ImportSource *reExportSource = ParseSourceFromClause(true);

    lexer::SourcePosition endLoc = reExportSource->Source()->End();
    auto *reExportDeclaration = AllocNode<ir::ETSImportDeclaration>(reExportSource, specifiers);
    reExportDeclaration->SetRange({startLoc, endLoc});

    ConsumeSemicolon(reExportDeclaration);

    auto reExport = AllocNode<ir::ETSReExportDeclaration>(reExportDeclaration, std::vector<std::string>(),
                                                          GetProgram()->SourceFilePath(), Allocator());
    reExport->AddModifier(modifiers);
    return reExport;
}

ir::ImportSource *ETSParser::ParseSourceFromClause(bool requireFrom)
{
    if (Lexer()->GetToken().KeywordType() != lexer::TokenType::KEYW_FROM) {
        if (requireFrom) {
            ThrowSyntaxError("Unexpected token.");
        }
    } else {
        Lexer()->NextToken();  // eat `from`
    }

    if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_STRING) {
        ThrowSyntaxError("Unexpected token.");
    }

    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_STRING);
    auto importPath = Lexer()->GetToken().Ident();

    auto resolvedImportPath = importPathManager_->ResolvePath(GetProgram()->AbsoluteName(), importPath);
    importPathManager_->AddToParseList(resolvedImportPath,
                                       (GetContext().Status() & ParserStatus::IN_DEFAULT_IMPORTS) != 0U);

    auto *resolvedSource = AllocNode<ir::StringLiteral>(resolvedImportPath);
    auto importData = importPathManager_->GetImportData(resolvedImportPath, Extension());
    auto *source = AllocNode<ir::StringLiteral>(importPath);
    source->SetRange(Lexer()->GetToken().Loc());

    Lexer()->NextToken();

    return Allocator()->New<ir::ImportSource>(source, resolvedSource, importData.lang, importData.hasDecl);
}

ArenaVector<ir::ImportSpecifier *> ETSParser::ParseNamedSpecifiers()
{
    // NOTE(user): handle qualifiedName in file bindings: qualifiedName '.' '*'
    if (!Lexer()->TryEatTokenType(lexer::TokenType::PUNCTUATOR_LEFT_BRACE)) {
        ThrowExpectedToken(lexer::TokenType::PUNCTUATOR_LEFT_BRACE);
    }

    auto fileName = GetProgram()->SourceFilePath().Mutf8();

    ArenaVector<ir::ImportSpecifier *> result(Allocator()->Adapter());

    while (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_BRACE) {
        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_MULTIPLY) {
            ThrowSyntaxError("The '*' token is not allowed as a selective binding (between braces)");
        }

        if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
            ThrowSyntaxError("Unexpected token");
        }

        lexer::Token importedToken = Lexer()->GetToken();
        auto *imported = AllocNode<ir::Identifier>(importedToken.Ident(), Allocator());
        ir::Identifier *local = nullptr;
        imported->SetReference();
        imported->SetRange(Lexer()->GetToken().Loc());

        Lexer()->NextToken();  // eat import/export name

        if (CheckModuleAsModifier() && Lexer()->TryEatTokenType(lexer::TokenType::KEYW_AS)) {
            local = ParseNamedImport(Lexer()->GetToken());
            Lexer()->NextToken();  // eat local name
        } else {
            local = ParseNamedImport(importedToken);
        }

        auto *specifier = AllocNode<ir::ImportSpecifier>(imported, local);
        specifier->SetRange({imported->Start(), local->End()});

        util::Helpers::CheckImportedName(result, specifier, fileName);

        result.emplace_back(specifier);

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
            Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);  // eat comma
        }
    }

    Lexer()->NextToken();  // eat '}'

    return result;
}

void ETSParser::ParseNameSpaceSpecifier(ArenaVector<ir::AstNode *> *specifiers, bool isReExport)
{
    lexer::SourcePosition namespaceStart = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat `*` character

    if (!CheckModuleAsModifier()) {
        ThrowSyntaxError("Unexpected token.");
    }

    auto *local = AllocNode<ir::Identifier>(util::StringView(""), Allocator());
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA ||
        Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_FROM || isReExport) {
        local->SetReference();
        auto *specifier = AllocNode<ir::ImportNamespaceSpecifier>(local);
        specifier->SetRange({namespaceStart, Lexer()->GetToken().End()});
        specifiers->push_back(specifier);
        return;
    }

    Lexer()->NextToken();  // eat `as` literal
    local = ParseNamedImport(Lexer()->GetToken());

    auto *specifier = AllocNode<ir::ImportNamespaceSpecifier>(local);
    specifier->SetRange({namespaceStart, Lexer()->GetToken().End()});
    specifiers->push_back(specifier);

    Lexer()->NextToken();  // eat local name
}

ir::AstNode *ETSParser::ParseImportDefaultSpecifier(ArenaVector<ir::AstNode *> *specifiers)
{
    if (Lexer()->GetToken().Type() != lexer::TokenType::LITERAL_IDENT) {
        ThrowSyntaxError("Unexpected token, expected an identifier");
    }

    auto *imported = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
    imported->SetReference();
    imported->SetRange(Lexer()->GetToken().Loc());
    Lexer()->NextToken();  // Eat import specifier.

    if (Lexer()->GetToken().KeywordType() != lexer::TokenType::KEYW_FROM) {
        ThrowSyntaxError("Unexpected token, expected 'from'");
    }

    auto *specifier = AllocNode<ir::ImportDefaultSpecifier>(imported);
    specifier->SetRange({imported->Start(), imported->End()});
    specifiers->push_back(specifier);

    return nullptr;
}

ir::AnnotatedExpression *ETSParser::ParseVariableDeclaratorKey([[maybe_unused]] VariableParsingFlags flags)
{
    ir::Identifier *init = ExpectIdentifier();
    ir::TypeNode *typeAnnotation = nullptr;
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_QUESTION_MARK) {
        if ((flags & VariableParsingFlags::FOR_OF) != 0U) {
            ThrowSyntaxError("Optional variable is not allowed in for of statements");
        }
        Lexer()->NextToken();  // eat '?'
        init->AddModifier(ir::ModifierFlags::OPTIONAL);
    }

    if (auto const tokenType = Lexer()->GetToken().Type(); tokenType == lexer::TokenType::PUNCTUATOR_COLON) {
        Lexer()->NextToken();  // eat ':'
        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
        typeAnnotation = ParseTypeAnnotation(&options);
    } else if (tokenType != lexer::TokenType::PUNCTUATOR_SUBSTITUTION && (flags & VariableParsingFlags::FOR_OF) == 0U) {
        ThrowSyntaxError("Variable must be initialized or it's type must be declared");
    }

    if (typeAnnotation != nullptr) {
        init->SetTsTypeAnnotation(typeAnnotation);
        typeAnnotation->SetParent(init);
    }

    return init;
}

ir::VariableDeclarator *ETSParser::ParseVariableDeclaratorInitializer(ir::Expression *init, VariableParsingFlags flags,
                                                                      const lexer::SourcePosition &startLoc)
{
    if ((flags & VariableParsingFlags::DISALLOW_INIT) != 0) {
        ThrowSyntaxError("for-await-of loop variable declaration may not have an initializer");
    }

    Lexer()->NextToken();

    ir::Expression *initializer = ParseExpression();

    lexer::SourcePosition endLoc = initializer->End();

    auto *declarator = AllocNode<ir::VariableDeclarator>(GetFlag(flags), init, initializer);
    declarator->SetRange({startLoc, endLoc});

    return declarator;
}

ir::VariableDeclarator *ETSParser::ParseVariableDeclarator(ir::Expression *init, lexer::SourcePosition startLoc,
                                                           VariableParsingFlags flags)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        return ParseVariableDeclaratorInitializer(init, flags, startLoc);
    }

    if ((flags & VariableParsingFlags::CONST) != 0 &&
        static_cast<uint32_t>(flags & VariableParsingFlags::ACCEPT_CONST_NO_INIT) == 0U) {
        ThrowSyntaxError("Missing initializer in const declaration");
    }

    if (init->AsIdentifier()->TypeAnnotation() == nullptr && (flags & VariableParsingFlags::FOR_OF) == 0U) {
        ThrowSyntaxError("Variable must be initialized or it's type must be declared");
    }

    lexer::SourcePosition endLoc = init->End();
    auto declarator = AllocNode<ir::VariableDeclarator>(GetFlag(flags), init);
    declarator->SetRange({startLoc, endLoc});

    // NOTE (psiket)  Transfer the OPTIONAL flag from the init to the declarator?
    return declarator;
}

ir::Statement *ETSParser::ParseAssertStatement()
{
    lexer::SourcePosition startLoc = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    ir::Expression *test = ParseExpression();
    lexer::SourcePosition endLoc = test->End();
    ir::Expression *second = nullptr;

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COLON) {
        Lexer()->NextToken();  // eat ':'
        second = ParseExpression();
        endLoc = second->End();
    }

    auto *asStatement = AllocNode<ir::AssertStatement>(test, second);
    asStatement->SetRange({startLoc, endLoc});
    ConsumeSemicolon(asStatement);

    return asStatement;
}

ir::Expression *ETSParser::ParseCatchParam()
{
    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS) {
        ThrowSyntaxError("Unexpected token, expected '('");
    }

    ir::AnnotatedExpression *param = nullptr;

    Lexer()->NextToken();  // eat left paren

    if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT) {
        CheckRestrictedBinding();
        param = ExpectIdentifier();
    } else {
        ThrowSyntaxError("Unexpected token in catch parameter, expected an identifier");
    }

    ParseCatchParamTypeAnnotation(param);

    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
        ThrowSyntaxError("Unexpected token, expected ')'");
    }

    Lexer()->NextToken();  // eat right paren

    return param;
}

void ETSParser::ParseCatchParamTypeAnnotation([[maybe_unused]] ir::AnnotatedExpression *param)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COLON) {
        Lexer()->NextToken();  // eat ':'

        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
        if (auto *typeAnnotation = ParseTypeAnnotation(&options); typeAnnotation != nullptr) {
            typeAnnotation->SetParent(param);
            param->SetTsTypeAnnotation(typeAnnotation);
        }
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        ThrowSyntaxError("Catch clause variable cannot have an initializer");
    }
}

ir::Statement *ETSParser::ParseTryStatement()
{
    lexer::SourcePosition startLoc = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat the 'try' keyword

    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        ThrowSyntaxError("Unexpected token, expected '{'");
    }

    ir::BlockStatement *body = ParseBlockStatement();

    ArenaVector<ir::CatchClause *> catchClauses(Allocator()->Adapter());

    while (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_CATCH) {
        ir::CatchClause *clause {};

        clause = ParseCatchClause();

        catchClauses.push_back(clause);
    }

    ir::BlockStatement *finalizer = nullptr;
    if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_FINALLY) {
        Lexer()->NextToken();  // eat 'finally' keyword

        finalizer = ParseBlockStatement();
    }

    if (catchClauses.empty() && finalizer == nullptr) {
        ThrowSyntaxError("A try statement should contain either finally clause or at least one catch clause.",
                         startLoc);
    }

    lexer::SourcePosition endLoc = finalizer != nullptr ? finalizer->End() : catchClauses.back()->End();

    ArenaVector<std::pair<compiler::LabelPair, const ir::Statement *>> finalizerInsertions(Allocator()->Adapter());

    auto *tryStatement = AllocNode<ir::TryStatement>(body, std::move(catchClauses), finalizer, finalizerInsertions);
    tryStatement->SetRange({startLoc, endLoc});
    ConsumeSemicolon(tryStatement);

    return tryStatement;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Expression *ETSParser::ParseUnaryOrPrefixUpdateExpression(ExpressionParseFlags flags)
{
    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::PUNCTUATOR_PLUS_PLUS:
        case lexer::TokenType::PUNCTUATOR_MINUS_MINUS:
        case lexer::TokenType::PUNCTUATOR_PLUS:
        case lexer::TokenType::PUNCTUATOR_MINUS:
        case lexer::TokenType::PUNCTUATOR_TILDE:
        case lexer::TokenType::PUNCTUATOR_DOLLAR_DOLLAR:
        case lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK:
        case lexer::TokenType::KEYW_TYPEOF: {
            break;
        }
        case lexer::TokenType::KEYW_LAUNCH: {
            return ParseLaunchExpression(flags);
        }
        default: {
            return ParseLeftHandSideExpression(flags);
        }
    }

    lexer::TokenType operatorType = Lexer()->GetToken().Type();
    auto start = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    ir::Expression *argument = ResolveArgumentUnaryExpr(flags);

    if (lexer::Token::IsUpdateToken(operatorType)) {
        if (!argument->IsIdentifier() && !argument->IsMemberExpression()) {
            ThrowSyntaxError("Invalid left-hand side in prefix operation");
        }
    }

    lexer::SourcePosition end = argument->End();

    ir::Expression *returnExpr = nullptr;
    if (lexer::Token::IsUpdateToken(operatorType)) {
        returnExpr = AllocNode<ir::UpdateExpression>(argument, operatorType, true);
    } else if (operatorType == lexer::TokenType::KEYW_TYPEOF) {
        returnExpr = AllocNode<ir::TypeofExpression>(argument);
    } else {
        returnExpr = AllocNode<ir::UnaryExpression>(argument, operatorType);
    }

    returnExpr->SetRange({start, end});

    return returnExpr;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Expression *ETSParser::ParseDefaultPrimaryExpression(ExpressionParseFlags flags)
{
    auto startLoc = Lexer()->GetToken().Start();
    auto savedPos = Lexer()->Save();
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::POTENTIAL_CLASS_LITERAL |
                                           TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE |
                                           TypeAnnotationParsingOptions::DISALLOW_UNION;
    ir::TypeNode *potentialType = ParseTypeAnnotation(&options);

    if (potentialType != nullptr) {
        if (potentialType->IsTSArrayType()) {
            return potentialType;
        }

        if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_PERIOD) {
            Lexer()->NextToken();  // eat '.'
        }

        if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_CLASS || IsStructKeyword()) {
            Lexer()->NextToken();  // eat 'class' and 'struct'
            auto *classLiteral = AllocNode<ir::ETSClassLiteral>(potentialType);
            classLiteral->SetRange({startLoc, Lexer()->GetToken().End()});
            return classLiteral;
        }
    }

    Lexer()->Rewind(savedPos);

    Lexer()->NextToken();
    bool pretendArrow = Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_ARROW;
    Lexer()->Rewind(savedPos);

    if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT && !pretendArrow) {
        return ParsePrimaryExpressionIdent(flags);
    }

    ThrowSyntaxError({"Unexpected token '", lexer::TokenToString(Lexer()->GetToken().Type()), "'."});
    return nullptr;
}

ir::Expression *ETSParser::ParsePrimaryExpressionWithLiterals(ExpressionParseFlags flags)
{
    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::LITERAL_TRUE:
        case lexer::TokenType::LITERAL_FALSE: {
            return ParseBooleanLiteral();
        }
        case lexer::TokenType::LITERAL_NULL: {
            return ParseNullLiteral();
        }
        case lexer::TokenType::KEYW_UNDEFINED: {
            return ParseUndefinedLiteral();
        }
        case lexer::TokenType::LITERAL_NUMBER: {
            return ParseCoercedNumberLiteral();
        }
        case lexer::TokenType::LITERAL_STRING: {
            return ParseStringLiteral();
        }
        case lexer::TokenType::LITERAL_CHAR: {
            return ParseCharLiteral();
        }
        default: {
            return ParseDefaultPrimaryExpression(flags);
        }
    }
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Expression *ETSParser::ParsePrimaryExpression(ExpressionParseFlags flags)
{
    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS: {
            return ParseCoverParenthesizedExpressionAndArrowParameterList(flags);
        }
        case lexer::TokenType::KEYW_THIS: {
            return ParseThisExpression();
        }
        case lexer::TokenType::KEYW_SUPER: {
            return ParseSuperExpression();
        }
        case lexer::TokenType::KEYW_NEW: {
            return ParseNewExpression();
        }
        case lexer::TokenType::KEYW_ASYNC: {
            return ParseAsyncExpression();
        }
        case lexer::TokenType::KEYW_AWAIT: {
            return ParseAwaitExpression();
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET: {
            return ParseArrayExpression(CarryPatternFlags(flags));
        }
        case lexer::TokenType::PUNCTUATOR_LEFT_BRACE: {
            return ParseObjectExpression(CarryPatternFlags(flags));
        }
        case lexer::TokenType::PUNCTUATOR_BACK_TICK: {
            return ParseTemplateLiteral();
        }
        case lexer::TokenType::KEYW_TYPE: {
            ThrowSyntaxError("Type alias is allowed only as top-level declaration");
        }
        case lexer::TokenType::PUNCTUATOR_FORMAT: {
            return ParseExpressionFormatPlaceholder();
        }
        case lexer::TokenType::KEYW_TYPEOF: {
            return ParseUnaryOrPrefixUpdateExpression();
        }
        default: {
            return ParsePrimaryExpressionWithLiterals(flags);
        }
    }
}

ir::Expression *ETSParser::ParseExpressionOrTypeAnnotation(lexer::TokenType type,
                                                           [[maybe_unused]] ExpressionParseFlags flags)
{
    if (type == lexer::TokenType::KEYW_INSTANCEOF) {
        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;

        if (Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_NULL) {
            auto *typeAnnotation = AllocNode<ir::NullLiteral>();
            typeAnnotation->SetRange(Lexer()->GetToken().Loc());
            Lexer()->NextToken();

            return typeAnnotation;
        }

        return ParseTypeAnnotation(&options);
    }

    return ParseExpression(ExpressionParseFlags::DISALLOW_YIELD);
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Expression *ETSParser::ParseCoverParenthesizedExpressionAndArrowParameterList(ExpressionParseFlags flags)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS);
    if (IsArrowFunctionExpressionStart()) {
        return ParseArrowFunctionExpression();
    }

    lexer::SourcePosition start = Lexer()->GetToken().Start();
    Lexer()->NextToken();

    ExpressionParseFlags newFlags = ExpressionParseFlags::ACCEPT_COMMA;
    if ((flags & ExpressionParseFlags::INSTANCEOF) != 0) {
        newFlags |= ExpressionParseFlags::INSTANCEOF;
    };

    ir::Expression *expr = ParseExpression(newFlags);

    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_RIGHT_PARENTHESIS) {
        ThrowSyntaxError("Unexpected token, expected ')'");
    }

    expr->SetGrouped();
    expr->SetRange({start, Lexer()->GetToken().End()});
    Lexer()->NextToken();

    return expr;
}

ir::Expression *ETSParser::ParsePostPrimaryExpression(ir::Expression *primaryExpr, lexer::SourcePosition startLoc,
                                                      bool ignoreCallExpression,
                                                      [[maybe_unused]] bool *isChainExpression)
{
    ir::Expression *returnExpression = primaryExpr;

    while (true) {
        switch (Lexer()->GetToken().Type()) {
            case lexer::TokenType::PUNCTUATOR_QUESTION_DOT: {
                if (*isChainExpression) {
                    break;  // terminate current chain
                }
                *isChainExpression = true;
                Lexer()->NextToken();  // eat ?.

                if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET) {
                    returnExpression = ParseElementAccess(returnExpression, true);
                    continue;
                }

                if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS) {
                    returnExpression = ParseCallExpression(returnExpression, true, false);
                    continue;
                }

                returnExpression = ParsePropertyAccess(returnExpression, true);
                continue;
            }
            case lexer::TokenType::PUNCTUATOR_PERIOD: {
                Lexer()->NextToken();  // eat period

                returnExpression = ParsePropertyAccess(returnExpression);
                continue;
            }
            case lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET: {
                returnExpression = ParseElementAccess(returnExpression);
                continue;
            }
            case lexer::TokenType::PUNCTUATOR_LEFT_SHIFT:
            case lexer::TokenType::PUNCTUATOR_LESS_THAN: {
                if (ParsePotentialGenericFunctionCall(returnExpression, &returnExpression, startLoc,
                                                      ignoreCallExpression)) {
                    break;
                }

                continue;
            }
            case lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS: {
                if (ignoreCallExpression) {
                    break;
                }
                returnExpression = ParseCallExpression(returnExpression, false, false);
                continue;
            }
            case lexer::TokenType::PUNCTUATOR_EXCLAMATION_MARK: {
                const bool shouldBreak = ParsePotentialNonNullExpression(&returnExpression, startLoc);

                if (shouldBreak) {
                    break;
                }

                continue;
            }
            case lexer::TokenType::PUNCTUATOR_FORMAT: {
                ThrowUnexpectedToken(lexer::TokenType::PUNCTUATOR_FORMAT);
            }
            default: {
                break;
            }
        }

        break;
    }

    return returnExpression;
}

ir::Expression *ETSParser::ParsePotentialAsExpression(ir::Expression *primaryExpr)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::KEYW_AS);
    Lexer()->NextToken();

    TypeAnnotationParsingOptions options =
        TypeAnnotationParsingOptions::THROW_ERROR | TypeAnnotationParsingOptions::ALLOW_INTERSECTION;
    ir::TypeNode *type = ParseTypeAnnotation(&options);

    auto *asExpression = AllocNode<ir::TSAsExpression>(primaryExpr, type, false);
    asExpression->SetRange(primaryExpr->Range());
    return asExpression;
}

ir::Expression *ETSParser::ParseNewExpression()
{
    lexer::SourcePosition start = Lexer()->GetToken().Start();

    Lexer()->NextToken();  // eat new

    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
    ir::TypeNode *baseTypeReference = ParseBaseTypeReference(&options);
    ir::TypeNode *typeReference = baseTypeReference;
    if (typeReference == nullptr) {
        options |= TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE | TypeAnnotationParsingOptions::ALLOW_WILDCARD;
        typeReference = ParseTypeReference(&options);
    } else if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        ThrowSyntaxError("Invalid { after base types.");
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET) {
        Lexer()->NextToken();
        ir::Expression *dimension = ParseExpression();

        auto endLoc = Lexer()->GetToken().End();
        ExpectToken(lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET);

        if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET) {
            auto *arrInstance = AllocNode<ir::ETSNewArrayInstanceExpression>(typeReference, dimension);
            arrInstance->SetRange({start, endLoc});
            return arrInstance;
        }

        ArenaVector<ir::Expression *> dimensions(Allocator()->Adapter());
        dimensions.push_back(dimension);

        do {
            Lexer()->NextToken();
            dimensions.push_back(ParseExpression());

            endLoc = Lexer()->GetToken().End();
            ExpectToken(lexer::TokenType::PUNCTUATOR_RIGHT_SQUARE_BRACKET);
        } while (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_SQUARE_BRACKET);

        auto *multiArray = AllocNode<ir::ETSNewMultiDimArrayInstanceExpression>(typeReference, std::move(dimensions));
        multiArray->SetRange({start, endLoc});
        return multiArray;
    }

    ArenaVector<ir::Expression *> arguments(Allocator()->Adapter());
    ir::ClassDefinition *classDefinition =
        CreateClassDefinitionForNewExpression(arguments, typeReference, baseTypeReference);

    auto *newExprNode =
        AllocNode<ir::ETSNewClassInstanceExpression>(typeReference, std::move(arguments), classDefinition);
    newExprNode->SetRange({start, Lexer()->GetToken().End()});

    return newExprNode;
}

ir::Expression *ETSParser::ParseAsyncExpression()
{
    Lexer()->NextToken();  // eat 'async'
    if (Lexer()->GetToken().Type() != lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS ||
        !IsArrowFunctionExpressionStart()) {
        ThrowSyntaxError("Unexpected token. expected '('");
    }

    auto newStatus = ParserStatus::NEED_RETURN_TYPE | ParserStatus::ARROW_FUNCTION | ParserStatus::ASYNC_FUNCTION;
    auto *func = ParseFunction(newStatus);
    auto *arrowFuncNode = AllocNode<ir::ArrowFunctionExpression>(Allocator(), func);
    arrowFuncNode->SetRange(func->Range());
    return arrowFuncNode;
}

ir::Expression *ETSParser::ParseAwaitExpression()
{
    lexer::SourcePosition start = Lexer()->GetToken().Start();
    Lexer()->NextToken();
    ir::Expression *argument = ParseExpression();
    auto *awaitExpression = AllocNode<ir::AwaitExpression>(argument);
    awaitExpression->SetRange({start, Lexer()->GetToken().End()});
    return awaitExpression;
}

ir::ModifierFlags ETSParser::ParseTypeVarianceModifier(TypeAnnotationParsingOptions *const options)
{
    if ((*options & TypeAnnotationParsingOptions::ALLOW_WILDCARD) == 0 &&
        (*options & TypeAnnotationParsingOptions::ALLOW_DECLARATION_SITE_VARIANCE) == 0) {
        ThrowSyntaxError("Variance modifier is not allowed here.");
    }

    switch (Lexer()->GetToken().KeywordType()) {
        case lexer::TokenType::KEYW_IN: {
            Lexer()->NextToken();
            return ir::ModifierFlags::IN;
        }
        case lexer::TokenType::KEYW_OUT: {
            Lexer()->NextToken();
            return ir::ModifierFlags::OUT;
        }
        default: {
            return ir::ModifierFlags::NONE;
        }
    }
}

ir::TSTypeParameter *ETSParser::ParseTypeParameter([[maybe_unused]] TypeAnnotationParsingOptions *options)
{
    lexer::SourcePosition startLoc = Lexer()->GetToken().Start();

    const auto varianceModifier = [this, options] {
        switch (Lexer()->GetToken().KeywordType()) {
            case lexer::TokenType::KEYW_IN:
            case lexer::TokenType::KEYW_OUT:
                return ParseTypeVarianceModifier(options);
            default:
                return ir::ModifierFlags::NONE;
        }
    }();

    auto *paramIdent = ExpectIdentifier();

    ir::TypeNode *constraint = nullptr;
    if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_EXTENDS) {
        Lexer()->NextToken();
        TypeAnnotationParsingOptions newOptions = TypeAnnotationParsingOptions::THROW_ERROR |
                                                  TypeAnnotationParsingOptions::ALLOW_INTERSECTION |
                                                  TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE;
        constraint = ParseTypeAnnotation(&newOptions);
    }

    ir::TypeNode *defaultType = nullptr;

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        Lexer()->NextToken();  // eat '='
        defaultType = ParseTypeAnnotation(options);
    }

    auto *typeParam = AllocNode<ir::TSTypeParameter>(paramIdent, constraint, defaultType, varianceModifier);

    typeParam->SetRange({startLoc, Lexer()->GetToken().End()});
    return typeParam;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::ETSStructDeclaration *ETSParser::ParseStructStatement([[maybe_unused]] StatementParsingFlags flags,
                                                          [[maybe_unused]] ir::ClassDefinitionModifiers modifiers,
                                                          [[maybe_unused]] ir::ModifierFlags modFlags)
{
    ThrowSyntaxError("Illegal start of expression", Lexer()->GetToken().Start());
}

ir::Expression *ETSParser::ParsePotentialExpressionSequence(ir::Expression *expr, ExpressionParseFlags flags)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA &&
        (flags & ExpressionParseFlags::ACCEPT_COMMA) != 0 && (flags & ExpressionParseFlags::IN_FOR) != 0U) {
        return ParseSequenceExpression(expr, (flags & ExpressionParseFlags::ACCEPT_REST) != 0);
    }

    return expr;
}

bool ETSParser::ParsePotentialNonNullExpression(ir::Expression **expression, const lexer::SourcePosition startLoc)
{
    if (expression == nullptr || Lexer()->GetToken().NewLine()) {
        return true;
    }

    const auto nonNullExpr = AllocNode<ir::TSNonNullExpression>(*expression);
    nonNullExpr->SetRange({startLoc, Lexer()->GetToken().End()});

    *expression = nonNullExpr;

    Lexer()->NextToken();

    return false;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Expression *ETSParser::ParseExpression(ExpressionParseFlags flags)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_YIELD &&
        (flags & ExpressionParseFlags::DISALLOW_YIELD) == 0U) {
        ir::YieldExpression *yieldExpr = ParseYieldExpression();

        return ParsePotentialExpressionSequence(yieldExpr, flags);
    }

    ir::Expression *unaryExpressionNode = ParseUnaryOrPrefixUpdateExpression(flags);
    if ((flags & ExpressionParseFlags::INSTANCEOF) != 0) {
        ValidateInstanceOfExpression(unaryExpressionNode);
    }

    ir::Expression *assignmentExpression = ParseAssignmentExpression(unaryExpressionNode, flags);

    if (Lexer()->GetToken().NewLine()) {
        return assignmentExpression;
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA &&
        (flags & ExpressionParseFlags::ACCEPT_COMMA) != 0U && (flags & ExpressionParseFlags::IN_FOR) != 0U) {
        return ParseSequenceExpression(assignmentExpression, (flags & ExpressionParseFlags::ACCEPT_REST) != 0U);
    }

    return assignmentExpression;
}

void ETSParser::ParseTrailingBlock(ir::CallExpression *callExpr)
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        callExpr->SetIsTrailingBlockInNewLine(Lexer()->GetToken().NewLine());
        callExpr->SetTrailingBlock(ParseBlockStatement());
    }
}

ir::Expression *ETSParser::ParseCoercedNumberLiteral()
{
    if ((Lexer()->GetToken().Flags() & lexer::TokenFlags::NUMBER_FLOAT) != 0U) {
        auto *number = AllocNode<ir::NumberLiteral>(Lexer()->GetToken().GetNumber());
        number->SetRange(Lexer()->GetToken().Loc());
        auto *floatType = AllocNode<ir::ETSPrimitiveType>(ir::PrimitiveType::FLOAT);
        floatType->SetRange(Lexer()->GetToken().Loc());
        auto *asExpression = AllocNode<ir::TSAsExpression>(number, floatType, true);
        asExpression->SetRange(Lexer()->GetToken().Loc());

        Lexer()->NextToken();
        return asExpression;
    }
    return ParseNumberLiteral();
}

//================================================================================================//
//  Methods to create AST node(s) from the specified string (part of valid ETS-code!)
//================================================================================================//

// NOLINTBEGIN(modernize-avoid-c-arrays)
static constexpr char const INVALID_NUMBER_NODE[] = "Invalid node number in format expression.";
static constexpr char const INVALID_FORMAT_NODE[] = "Invalid node type in format expression.";
static constexpr char const INSERT_NODE_ABSENT[] = "There is no any node to insert at the placeholder position.";
static constexpr char const INVALID_INSERT_NODE[] =
    "Inserting node type differs from that required by format specification.";
// NOLINTEND(modernize-avoid-c-arrays)

ParserImpl::NodeFormatType ETSParser::GetFormatPlaceholderIdent() const
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_FORMAT);
    Lexer()->NextToken();
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT);

    char const *const identData = Lexer()->GetToken().Ident().Bytes();

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, cert-err34-c)
    auto identNumber = std::atoi(identData + 1U);
    if (identNumber <= 0) {
        ThrowSyntaxError(INVALID_NUMBER_NODE, Lexer()->GetToken().Start());
    }

    return {*identData, static_cast<decltype(std::declval<ParserImpl::NodeFormatType>().second)>(identNumber - 1)};
}

ir::AstNode *ETSParser::ParseFormatPlaceholder()
{
    if (insertingNodes_.empty()) {
        ThrowSyntaxError(INSERT_NODE_ABSENT, Lexer()->GetToken().Start());
    }

    if (auto nodeFormat = GetFormatPlaceholderIdent(); nodeFormat.first == EXPRESSION_FORMAT_NODE) {
        return ParseExpressionFormatPlaceholder(std::make_optional(nodeFormat));
    } else if (nodeFormat.first == IDENTIFIER_FORMAT_NODE) {  // NOLINT(readability-else-after-return)
        return ParseIdentifierFormatPlaceholder(std::make_optional(nodeFormat));
    } else if (nodeFormat.first == TYPE_FORMAT_NODE) {  // NOLINT(readability-else-after-return)
        return ParseTypeFormatPlaceholder(std::make_optional(nodeFormat));
    } else if (nodeFormat.first == STATEMENT_FORMAT_NODE) {  // NOLINT(readability-else-after-return)
        return ParseStatementFormatPlaceholder(std::make_optional(nodeFormat));
    }

    ThrowSyntaxError(INVALID_FORMAT_NODE, Lexer()->GetToken().Start());
}

ir::Expression *ETSParser::ParseExpressionFormatPlaceholder(std::optional<ParserImpl::NodeFormatType> nodeFormat)
{
    if (!nodeFormat.has_value()) {
        if (insertingNodes_.empty()) {
            ThrowSyntaxError(INSERT_NODE_ABSENT, Lexer()->GetToken().Start());
        }

        if (nodeFormat = GetFormatPlaceholderIdent(); nodeFormat->first == TYPE_FORMAT_NODE) {
            return ParseTypeFormatPlaceholder(std::move(nodeFormat));
        } else if (nodeFormat->first == IDENTIFIER_FORMAT_NODE) {  // NOLINT(readability-else-after-return)
            return ParseIdentifierFormatPlaceholder(std::move(nodeFormat));
        } else if (nodeFormat->first != EXPRESSION_FORMAT_NODE) {  // NOLINT(readability-else-after-return)
            ThrowSyntaxError(INVALID_FORMAT_NODE, Lexer()->GetToken().Start());
        }
    }

    auto *const insertingNode =
        nodeFormat->second < insertingNodes_.size() ? insertingNodes_[nodeFormat->second] : nullptr;
    if (insertingNode == nullptr || !insertingNode->IsExpression()) {
        ThrowSyntaxError(INVALID_INSERT_NODE, Lexer()->GetToken().Start());
    }

    auto *const insertExpression = insertingNode->AsExpression();
    Lexer()->NextToken();
    return insertExpression;
}

ir::TypeNode *ETSParser::ParseTypeFormatPlaceholder(std::optional<ParserImpl::NodeFormatType> nodeFormat)
{
    if (!nodeFormat.has_value()) {
        if (insertingNodes_.empty()) {
            ThrowSyntaxError(INSERT_NODE_ABSENT, Lexer()->GetToken().Start());
        }

        if (nodeFormat = GetFormatPlaceholderIdent(); nodeFormat->first != TYPE_FORMAT_NODE) {
            ThrowSyntaxError(INVALID_FORMAT_NODE, Lexer()->GetToken().Start());
        }
    }

    auto *const insertingNode =
        nodeFormat->second < insertingNodes_.size() ? insertingNodes_[nodeFormat->second] : nullptr;
    if (insertingNode == nullptr || !insertingNode->IsExpression() || !insertingNode->AsExpression()->IsTypeNode()) {
        ThrowSyntaxError(INVALID_INSERT_NODE, Lexer()->GetToken().Start());
    }

    auto *const insertType = insertingNode->AsExpression()->AsTypeNode();
    Lexer()->NextToken();
    return insertType;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::Identifier *ETSParser::ParseIdentifierFormatPlaceholder(std::optional<ParserImpl::NodeFormatType> nodeFormat)
{
    if (!nodeFormat.has_value()) {
        if (insertingNodes_.empty()) {
            ThrowSyntaxError(INSERT_NODE_ABSENT, Lexer()->GetToken().Start());
        }

        if (nodeFormat = GetFormatPlaceholderIdent(); nodeFormat->first != IDENTIFIER_FORMAT_NODE) {
            ThrowSyntaxError(INVALID_FORMAT_NODE, Lexer()->GetToken().Start());
        }
    }

    auto *const insertingNode =
        nodeFormat->second < insertingNodes_.size() ? insertingNodes_[nodeFormat->second] : nullptr;
    if (insertingNode == nullptr || !insertingNode->IsExpression() || !insertingNode->AsExpression()->IsIdentifier()) {
        ThrowSyntaxError(INVALID_INSERT_NODE, Lexer()->GetToken().Start());
    }

    auto *const insertIdentifier = insertingNode->AsExpression()->AsIdentifier();
    Lexer()->NextToken();
    return insertIdentifier;
}

ir::Statement *ETSParser::ParseStatementFormatPlaceholder(std::optional<ParserImpl::NodeFormatType> nodeFormat)
{
    if (!nodeFormat.has_value()) {
        if (insertingNodes_.empty()) {
            ThrowSyntaxError(INSERT_NODE_ABSENT, Lexer()->GetToken().Start());
        }

        if (nodeFormat = GetFormatPlaceholderIdent(); nodeFormat->first != STATEMENT_FORMAT_NODE) {
            ThrowSyntaxError(INVALID_FORMAT_NODE, Lexer()->GetToken().Start());
        }
    }

    auto *const insertingNode =
        nodeFormat->second < insertingNodes_.size() ? insertingNodes_[nodeFormat->second] : nullptr;
    if (insertingNode == nullptr || !insertingNode->IsStatement()) {
        ThrowSyntaxError(INVALID_INSERT_NODE, Lexer()->GetToken().Start());
    }

    auto *const insertStatement = insertingNode->AsStatement();
    Lexer()->NextToken();
    return insertStatement;
}

std::pair<ir::ModifierFlags, lexer::SourcePosition> ETSParser::ParseMemberModifiers()
{
    auto memberModifiers = ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC;

    if (Lexer()->TryEatTokenType(lexer::TokenType::KEYW_EXPORT)) {
        if (Lexer()->TryEatTokenKeyword(lexer::TokenType::KEYW_DEFAULT)) {
            memberModifiers |= ir::ModifierFlags::DEFAULT_EXPORT;
        } else {
            memberModifiers |= ir::ModifierFlags::EXPORT;
        }
    }

    lexer::SourcePosition startLoc = Lexer()->GetToken().Start();

    if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_DECLARE) {
        CheckDeclare();
        memberModifiers |= ir::ModifierFlags::DECLARE;
    }
    const auto tokenType = Lexer()->GetToken().KeywordType();
    if (tokenType == lexer::TokenType::KEYW_ASYNC || tokenType == lexer::TokenType::KEYW_NATIVE) {
        bool isAsync = tokenType == lexer::TokenType::KEYW_ASYNC;

        if (isAsync) {
            memberModifiers |= ir::ModifierFlags::ASYNC;
        } else {
            memberModifiers |= ir::ModifierFlags::NATIVE;
        }
        Lexer()->NextToken();

        if (Lexer()->GetToken().Type() != lexer::TokenType::KEYW_FUNCTION) {
            ThrowSyntaxError(
                {isAsync ? "'async'" : "'native'", " flags must be used for functions only at top-level."});
        }
    }
    return std::make_pair(memberModifiers, startLoc);
}

}  // namespace ark::es2panda::parser
