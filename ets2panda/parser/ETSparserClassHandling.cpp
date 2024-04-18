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

static bool IsInterfaceMethodModifier(lexer::TokenType type)
{
    // NOTE (psiket) Rewrite this
    return type == lexer::TokenType::KEYW_STATIC || type == lexer::TokenType::KEYW_PRIVATE ||
           type == lexer::TokenType::KEYW_PROTECTED || type == lexer::TokenType::KEYW_PUBLIC;
}

static bool IsClassModifier(lexer::TokenType type)
{
    return type == lexer::TokenType::KEYW_STATIC || type == lexer::TokenType::KEYW_ABSTRACT ||
           type == lexer::TokenType::KEYW_FINAL;
}

ir::ModifierFlags ETSParser::ParseClassModifiers()
{
    ir::ModifierFlags flags = ir::ModifierFlags::NONE;

    while (IsClassModifier(Lexer()->GetToken().KeywordType())) {
        ir::ModifierFlags currentFlag = ir::ModifierFlags::NONE;

        lexer::TokenFlags tokenFlags = Lexer()->GetToken().Flags();
        if ((tokenFlags & lexer::TokenFlags::HAS_ESCAPE) != 0) {
            ThrowSyntaxError("Keyword must not contain escaped characters");
        }

        switch (Lexer()->GetToken().KeywordType()) {
            case lexer::TokenType::KEYW_STATIC: {
                currentFlag = ir::ModifierFlags::STATIC;
                break;
            }
            case lexer::TokenType::KEYW_FINAL: {
                currentFlag = ir::ModifierFlags::FINAL;
                break;
            }
            case lexer::TokenType::KEYW_ABSTRACT: {
                currentFlag = ir::ModifierFlags::ABSTRACT;
                break;
            }
            default: {
                UNREACHABLE();
            }
        }

        if ((flags & currentFlag) != 0) {
            ThrowSyntaxError("Duplicated modifier is not allowed");
        }

        Lexer()->NextToken();
        flags |= currentFlag;
    }

    return flags;
}

std::tuple<ir::Expression *, ir::TSTypeParameterInstantiation *> ETSParser::ParseClassImplementsElement()
{
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR |
                                           TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE |
                                           TypeAnnotationParsingOptions::ALLOW_WILDCARD;
    return {ParseTypeReference(&options), nullptr};
}

ir::Expression *ETSParser::ParseSuperClassReference()
{
    if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_EXTENDS) {
        Lexer()->NextToken();

        TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR |
                                               TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE |
                                               TypeAnnotationParsingOptions::ALLOW_WILDCARD;
        return ParseTypeReference(&options);
    }

    return nullptr;
}

static bool IsClassMemberAccessModifier(lexer::TokenType type)
{
    return type == lexer::TokenType::KEYW_PUBLIC || type == lexer::TokenType::KEYW_PRIVATE ||
           type == lexer::TokenType::KEYW_PROTECTED || type == lexer::TokenType::KEYW_INTERNAL;
}

std::tuple<ir::ModifierFlags, bool> ETSParser::ParseClassMemberAccessModifiers()
{
    if (IsClassMemberAccessModifier(Lexer()->GetToken().Type())) {
        char32_t nextCp = Lexer()->Lookahead();
        if (!(nextCp != lexer::LEX_CHAR_EQUALS && nextCp != lexer::LEX_CHAR_COLON &&
              nextCp != lexer::LEX_CHAR_LEFT_PAREN)) {
            return {ir::ModifierFlags::NONE, false};
        }

        lexer::TokenFlags tokenFlags = Lexer()->GetToken().Flags();
        if ((tokenFlags & lexer::TokenFlags::HAS_ESCAPE) != 0) {
            ThrowSyntaxError("Keyword must not contain escaped characters");
        }

        ir::ModifierFlags accessFlag = ir::ModifierFlags::NONE;

        switch (Lexer()->GetToken().KeywordType()) {
            case lexer::TokenType::KEYW_PUBLIC: {
                accessFlag = ir::ModifierFlags::PUBLIC;
                break;
            }
            case lexer::TokenType::KEYW_PRIVATE: {
                accessFlag = ir::ModifierFlags::PRIVATE;
                break;
            }
            case lexer::TokenType::KEYW_PROTECTED: {
                accessFlag = ir::ModifierFlags::PROTECTED;
                break;
            }
            case lexer::TokenType::KEYW_INTERNAL: {
                Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);
                if (Lexer()->GetToken().KeywordType() != lexer::TokenType::KEYW_PROTECTED) {
                    accessFlag = ir::ModifierFlags::INTERNAL;
                    return {accessFlag, true};
                }
                accessFlag = ir::ModifierFlags::INTERNAL_PROTECTED;
                break;
            }
            default: {
                UNREACHABLE();
            }
        }
        if (((GetContext().Status() & ParserStatus::FUNCTION) != 0) &&
            (accessFlag == ir::ModifierFlags::PUBLIC || accessFlag == ir::ModifierFlags::PRIVATE ||
             accessFlag == ir::ModifierFlags::PROTECTED)) {
            ThrowSyntaxError("Local class declaration members can not have access modifies",
                             Lexer()->GetToken().Start());
        }

        Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);
        return {accessFlag, true};
    }

    return {ir::ModifierFlags::PUBLIC, false};
}

static bool IsClassFieldModifier(lexer::TokenType type)
{
    return type == lexer::TokenType::KEYW_STATIC || type == lexer::TokenType::KEYW_READONLY;
}

ir::ModifierFlags ETSParser::ParseClassFieldModifiers(bool seenStatic)
{
    ir::ModifierFlags flags = seenStatic ? ir::ModifierFlags::STATIC : ir::ModifierFlags::NONE;

    while (IsClassFieldModifier(Lexer()->GetToken().KeywordType())) {
        char32_t nextCp = Lexer()->Lookahead();
        if (!(nextCp != lexer::LEX_CHAR_EQUALS && nextCp != lexer::LEX_CHAR_COLON)) {
            return flags;
        }

        ir::ModifierFlags currentFlag;

        lexer::TokenFlags tokenFlags = Lexer()->GetToken().Flags();
        if ((tokenFlags & lexer::TokenFlags::HAS_ESCAPE) != 0) {
            ThrowSyntaxError("Keyword must not contain escaped characters");
        }

        switch (Lexer()->GetToken().KeywordType()) {
            case lexer::TokenType::KEYW_STATIC: {
                currentFlag = ir::ModifierFlags::STATIC;
                break;
            }
            case lexer::TokenType::KEYW_READONLY: {
                // NOTE(OCs): Use ir::ModifierFlags::READONLY once compiler is ready for it.
                currentFlag = ir::ModifierFlags::CONST;
                break;
            }
            default: {
                UNREACHABLE();
            }
        }

        if ((flags & currentFlag) != 0) {
            ThrowSyntaxError("Duplicated modifier is not allowed");
        }

        Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);
        flags |= currentFlag;
    }

    return flags;
}

static bool IsClassMethodModifier(lexer::TokenType type)
{
    switch (type) {
        case lexer::TokenType::KEYW_STATIC:
        case lexer::TokenType::KEYW_FINAL:
        case lexer::TokenType::KEYW_NATIVE:
        case lexer::TokenType::KEYW_ASYNC:
        case lexer::TokenType::KEYW_OVERRIDE:
        case lexer::TokenType::KEYW_ABSTRACT: {
            return true;
        }
        default: {
            break;
        }
    }

    return false;
}

ir::ModifierFlags ETSParser::ParseClassMethodModifiers(bool seenStatic)
{
    ir::ModifierFlags flags = seenStatic ? ir::ModifierFlags::STATIC : ir::ModifierFlags::NONE;

    while (IsClassMethodModifier(Lexer()->GetToken().KeywordType())) {
        char32_t nextCp = Lexer()->Lookahead();
        if (!(nextCp != lexer::LEX_CHAR_LEFT_PAREN)) {
            return flags;
        }

        ir::ModifierFlags currentFlag = ir::ModifierFlags::NONE;

        lexer::TokenFlags tokenFlags = Lexer()->GetToken().Flags();
        if ((tokenFlags & lexer::TokenFlags::HAS_ESCAPE) != 0) {
            ThrowSyntaxError("Keyword must not contain escaped characters");
        }

        switch (Lexer()->GetToken().KeywordType()) {
            case lexer::TokenType::KEYW_STATIC: {
                currentFlag = ir::ModifierFlags::STATIC;
                break;
            }
            case lexer::TokenType::KEYW_FINAL: {
                currentFlag = ir::ModifierFlags::FINAL;
                break;
            }
            case lexer::TokenType::KEYW_NATIVE: {
                currentFlag = ir::ModifierFlags::NATIVE;
                break;
            }
            case lexer::TokenType::KEYW_ASYNC: {
                currentFlag = ir::ModifierFlags::ASYNC;
                break;
            }
            case lexer::TokenType::KEYW_OVERRIDE: {
                currentFlag = ir::ModifierFlags::OVERRIDE;
                break;
            }
            case lexer::TokenType::KEYW_ABSTRACT: {
                currentFlag = ir::ModifierFlags::ABSTRACT;
                break;
            }
            case lexer::TokenType::KEYW_DECLARE: {
                currentFlag = ir::ModifierFlags::DECLARE;
                break;
            }
            default: {
                UNREACHABLE();
            }
        }

        if ((flags & currentFlag) != 0) {
            ThrowSyntaxError("Duplicated modifier is not allowed");
        }

        Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);
        flags |= currentFlag;
        if ((flags & ir::ModifierFlags::ASYNC) != 0 && (flags & ir::ModifierFlags::NATIVE) != 0) {
            ThrowSyntaxError("Native method cannot be async");
        }
    }

    return flags;
}

// NOLINTNEXTLINE(google-default-arguments)
void ETSParser::ParseClassFieldDefinition(ir::Identifier *fieldName, ir::ModifierFlags modifiers,
                                          ArenaVector<ir::AstNode *> *declarations)
{
    lexer::SourcePosition endLoc = fieldName->End();
    ir::TypeNode *typeAnnotation = nullptr;
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
    bool optionalField = false;

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_QUESTION_MARK) {
        Lexer()->NextToken();  // eat '?'
        optionalField = true;
    }
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COLON) {
        Lexer()->NextToken();  // eat ':'
        typeAnnotation = ParseTypeAnnotation(&options);
        endLoc = typeAnnotation->End();
    }

    ir::Expression *initializer = nullptr;
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_SUBSTITUTION) {
        Lexer()->NextToken();  // eat '='
        initializer = ParseExpression();
    } else if (typeAnnotation == nullptr) {
        ThrowSyntaxError("Field type annotation expected");
    }

    bool isDeclare = (modifiers & ir::ModifierFlags::DECLARE) != 0;

    if (isDeclare && initializer != nullptr) {
        ThrowSyntaxError("Initializers are not allowed in ambient contexts.");
    }

    auto *field = AllocNode<ir::ClassProperty>(fieldName, initializer, typeAnnotation, modifiers, Allocator(), false);
    field->SetRange({fieldName->Start(), initializer != nullptr ? initializer->End() : endLoc});
    if (optionalField) {
        field->AddModifier(ir::ModifierFlags::OPTIONAL);
    }

    declarations->push_back(field);

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_COMMA) {
        Lexer()->NextToken();
        ir::Identifier *nextName = ExpectIdentifier(false, true);
        ParseClassFieldDefinition(nextName, modifiers, declarations);
    }
}

ir::MethodDefinition *ETSParser::ParseClassMethodDefinition(ir::Identifier *methodName, ir::ModifierFlags modifiers,
                                                            ir::Identifier *className,
                                                            [[maybe_unused]] ir::Identifier *identNode)
{
    auto newStatus = ParserStatus::NEED_RETURN_TYPE | ParserStatus::ALLOW_SUPER;
    auto methodKind = ir::MethodDefinitionKind::METHOD;

    if (className != nullptr) {
        methodKind = ir::MethodDefinitionKind::EXTENSION_METHOD;
        newStatus |= ParserStatus::IN_EXTENSION_FUNCTION;
    }

    if ((modifiers & ir::ModifierFlags::CONSTRUCTOR) != 0) {
        newStatus = ParserStatus::CONSTRUCTOR_FUNCTION | ParserStatus::ALLOW_SUPER | ParserStatus::ALLOW_SUPER_CALL;
        methodKind = ir::MethodDefinitionKind::CONSTRUCTOR;
    }

    if ((modifiers & ir::ModifierFlags::ASYNC) != 0) {
        newStatus |= ParserStatus::ASYNC_FUNCTION;
    }

    if ((modifiers & ir::ModifierFlags::STATIC) == 0) {
        newStatus |= ParserStatus::ALLOW_THIS_TYPE;
    }

    ir::ScriptFunction *func = ParseFunction(newStatus, className);
    func->SetIdent(methodName);
    auto *funcExpr = AllocNode<ir::FunctionExpression>(func);
    funcExpr->SetRange(func->Range());
    func->AddModifier(modifiers);

    if (className != nullptr) {
        func->AddFlag(ir::ScriptFunctionFlags::INSTANCE_EXTENSION_METHOD);
    }
    auto *method = AllocNode<ir::MethodDefinition>(methodKind, methodName->Clone(Allocator(), nullptr)->AsExpression(),
                                                   funcExpr, modifiers, Allocator(), false);
    method->SetRange(funcExpr->Range());
    func->Id()->SetReference();
    return method;
}

ir::MethodDefinition *ETSParser::ParseClassMethod(ClassElementDescriptor *desc,
                                                  const ArenaVector<ir::AstNode *> &properties,
                                                  ir::Expression *propName, lexer::SourcePosition *propEnd)
{
    if (desc->methodKind != ir::MethodDefinitionKind::SET &&
        (desc->newStatus & ParserStatus::CONSTRUCTOR_FUNCTION) == 0) {
        desc->newStatus |= ParserStatus::NEED_RETURN_TYPE;
    }

    ir::ScriptFunction *func = ParseFunction(desc->newStatus);
    if (propName->IsIdentifier()) {
        func->SetIdent(propName->AsIdentifier()->Clone(Allocator(), nullptr));
        func->Id()->SetReference();
    }

    auto *funcExpr = AllocNode<ir::FunctionExpression>(func);
    funcExpr->SetRange(func->Range());

    if (desc->methodKind == ir::MethodDefinitionKind::SET) {
        ValidateClassSetter(desc, properties, propName, func);
    } else if (desc->methodKind == ir::MethodDefinitionKind::GET) {
        ValidateClassGetter(desc, properties, propName, func);
    }

    *propEnd = func->End();
    func->AddFlag(ir::ScriptFunctionFlags::METHOD);
    auto *method =
        AllocNode<ir::MethodDefinition>(desc->methodKind, propName->Clone(Allocator(), nullptr)->AsExpression(),
                                        funcExpr, desc->modifiers, Allocator(), desc->isComputed);
    method->SetRange(funcExpr->Range());

    return method;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::AstNode *ETSParser::ParseClassElement([[maybe_unused]] const ArenaVector<ir::AstNode *> &properties,
                                          [[maybe_unused]] ir::ClassDefinitionModifiers modifiers,
                                          [[maybe_unused]] ir::ModifierFlags flags,
                                          [[maybe_unused]] ir::Identifier *identNode)
{
    auto startLoc = Lexer()->GetToken().Start();
    auto savedPos = Lexer()->Save();  // NOLINT(clang-analyzer-deadcode.DeadStores)

    if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_STATIC &&
        Lexer()->Lookahead() == lexer::LEX_CHAR_LEFT_BRACE) {
        return ParseClassStaticBlock();
    }

    auto [memberModifiers, isStepToken] = ParseClassMemberAccessModifiers();

    if (InAmbientContext()) {
        memberModifiers |= ir::ModifierFlags::DECLARE;
    }

    bool seenStatic = false;
    char32_t nextCp = Lexer()->Lookahead();

    if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_STATIC && nextCp != lexer::LEX_CHAR_EQUALS &&
        nextCp != lexer::LEX_CHAR_COLON && nextCp != lexer::LEX_CHAR_LEFT_PAREN &&
        nextCp != lexer::LEX_CHAR_LESS_THAN) {
        Lexer()->NextToken();
        memberModifiers |= ir::ModifierFlags::STATIC;
        seenStatic = true;
    }

    if (IsClassFieldModifier(Lexer()->GetToken().KeywordType())) {
        memberModifiers |= ParseClassFieldModifiers(seenStatic);
    } else if (IsClassMethodModifier(Lexer()->GetToken().Type())) {
        memberModifiers |= ParseClassMethodModifiers(seenStatic);
    }

    switch (Lexer()->GetToken().Type()) {
        case lexer::TokenType::KEYW_INTERFACE:
        case lexer::TokenType::KEYW_CLASS:
        case lexer::TokenType::KEYW_ENUM: {
            return ParseInnerTypeDeclaration(memberModifiers, savedPos, isStepToken, seenStatic);
        }
        case lexer::TokenType::KEYW_CONSTRUCTOR: {
            return ParseInnerConstructorDeclaration(memberModifiers, startLoc);
        }
        case lexer::TokenType::KEYW_PUBLIC:
        case lexer::TokenType::KEYW_PRIVATE:
        case lexer::TokenType::KEYW_PROTECTED: {
            ThrowSyntaxError("Access modifier must precede field and method modifiers.");
            break;
        }
        default: {
            break;
        }
    }

    return ParseInnerRest(properties, modifiers, memberModifiers, identNode, startLoc);
}

ir::MethodDefinition *ETSParser::ParseClassGetterSetterMethod(const ArenaVector<ir::AstNode *> &properties,
                                                              const ir::ClassDefinitionModifiers modifiers,
                                                              const ir::ModifierFlags memberModifiers)
{
    ClassElementDescriptor desc(Allocator());
    desc.methodKind = Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_GET ? ir::MethodDefinitionKind::GET
                                                                                      : ir::MethodDefinitionKind::SET;
    Lexer()->NextToken();  // eat get/set
    auto *methodName = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
    if (desc.methodKind == ir::MethodDefinitionKind::GET) {
        methodName->SetAccessor();
    } else {
        methodName->SetMutator();
    }

    Lexer()->NextToken(lexer::NextTokenFlags::KEYWORD_TO_IDENT);

    desc.newStatus = ParserStatus::ALLOW_SUPER;
    desc.hasSuperClass = (modifiers & ir::ClassDefinitionModifiers::HAS_SUPER) != 0U;
    desc.propStart = Lexer()->GetToken().Start();
    desc.modifiers = memberModifiers;

    lexer::SourcePosition propEnd = methodName->End();
    ir::MethodDefinition *method = ParseClassMethod(&desc, properties, methodName, &propEnd);
    method->Function()->AddModifier(desc.modifiers);
    method->SetRange({desc.propStart, propEnd});
    if (desc.methodKind == ir::MethodDefinitionKind::GET) {
        method->Function()->AddFlag(ir::ScriptFunctionFlags::GETTER);
    } else {
        method->Function()->AddFlag(ir::ScriptFunctionFlags::SETTER);
    }

    return method;
}

ir::AstNode *ETSParser::ParseInnerRest(const ArenaVector<ir::AstNode *> &properties,
                                       ir::ClassDefinitionModifiers modifiers, ir::ModifierFlags memberModifiers,
                                       ir::Identifier *identNode, const lexer::SourcePosition &startLoc)
{
    if (Lexer()->Lookahead() != lexer::LEX_CHAR_LEFT_PAREN && Lexer()->Lookahead() != lexer::LEX_CHAR_LESS_THAN &&
        (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_GET ||
         Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_SET)) {
        return ParseClassGetterSetterMethod(properties, modifiers, memberModifiers);
    }

    if ((GetContext().Status() & ParserStatus::IN_NAMESPACE) != 0) {
        auto type = Lexer()->GetToken().Type();
        if (type == lexer::TokenType::KEYW_FUNCTION || type == lexer::TokenType::KEYW_LET ||
            type == lexer::TokenType::KEYW_CONST) {
            Lexer()->NextToken();
        }
    }

    auto *memberName = ExpectIdentifier();

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_PARENTHESIS ||
        Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        auto *classMethod = ParseClassMethodDefinition(memberName, memberModifiers, nullptr, identNode);
        classMethod->SetStart(startLoc);
        return classMethod;
    }

    ArenaVector<ir::AstNode *> fieldDeclarations(Allocator()->Adapter());
    auto *placeholder = AllocNode<ir::TSInterfaceBody>(std::move(fieldDeclarations));
    ParseClassFieldDefinition(memberName, memberModifiers, placeholder->BodyPtr());
    return placeholder;
}

ir::AstNode *ETSParser::ParseInnerConstructorDeclaration(ir::ModifierFlags memberModifiers,
                                                         const lexer::SourcePosition &startLoc)
{
    if ((GetContext().Status() & ParserStatus::IN_NAMESPACE) != 0) {
        ThrowSyntaxError({"Namespaces should not have a constructor"});
    }
    if ((memberModifiers & ir::ModifierFlags::ASYNC) != 0) {
        ThrowSyntaxError({"Constructor should not be async."});
    }
    auto *memberName = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
    memberModifiers |= ir::ModifierFlags::CONSTRUCTOR;
    Lexer()->NextToken();
    auto *classMethod = ParseClassMethodDefinition(memberName, memberModifiers);
    classMethod->SetStart(startLoc);

    return classMethod;
}

ir::MethodDefinition *ETSParser::ParseInterfaceGetterSetterMethod(const ir::ModifierFlags modifiers)
{
    auto methodKind = Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_GET ? ir::MethodDefinitionKind::GET
                                                                                      : ir::MethodDefinitionKind::SET;
    Lexer()->NextToken();  // eat get/set
    ir::MethodDefinition *method = ParseInterfaceMethod(modifiers, methodKind);
    method->SetRange({Lexer()->GetToken().Start(), method->Id()->End()});
    if (methodKind == ir::MethodDefinitionKind::GET) {
        method->Id()->SetAccessor();
        method->Function()->AddFlag(ir::ScriptFunctionFlags::GETTER);
    } else {
        method->Id()->SetMutator();
        method->Function()->AddFlag(ir::ScriptFunctionFlags::SETTER);
    }

    method->Function()->SetIdent(method->Id()->Clone(Allocator(), nullptr));
    method->Function()->AddModifier(method->Modifiers());

    return method;
}

ir::Statement *ETSParser::ParseTypeDeclarationAbstractFinal(bool allowStatic, ir::ClassDefinitionModifiers modifiers)
{
    auto flags = ParseClassModifiers();
    if (allowStatic && (flags & ir::ModifierFlags::STATIC) == 0U) {
        modifiers |= ir::ClassDefinitionModifiers::INNER;
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_CLASS) {
        return ParseClassDeclaration(modifiers, flags);
    }

    if (IsStructKeyword()) {
        return ParseStructDeclaration(modifiers, flags);
    }

    ThrowUnexpectedToken(Lexer()->GetToken().Type());
}

ir::TSInterfaceDeclaration *ETSParser::ParseInterfaceBody(ir::Identifier *name, bool isStatic)
{
    GetContext().Status() |= ParserStatus::ALLOW_THIS_TYPE;

    ir::TSTypeParameterDeclaration *typeParamDecl = nullptr;
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        auto options =
            TypeAnnotationParsingOptions::THROW_ERROR | TypeAnnotationParsingOptions::ALLOW_DECLARATION_SITE_VARIANCE;
        typeParamDecl = ParseTypeParameterDeclaration(&options);
    }

    ArenaVector<ir::TSInterfaceHeritage *> extends(Allocator()->Adapter());
    if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_EXTENDS) {
        extends = ParseInterfaceExtendsClause();
    }

    lexer::SourcePosition bodyStart = Lexer()->GetToken().Start();
    auto members = ParseTypeLiteralOrInterface();

    for (auto &member : members) {
        if (member->Type() == ir::AstNodeType::CLASS_DECLARATION ||
            member->Type() == ir::AstNodeType::STRUCT_DECLARATION ||
            member->Type() == ir::AstNodeType::TS_ENUM_DECLARATION ||
            member->Type() == ir::AstNodeType::TS_INTERFACE_DECLARATION) {
            ThrowSyntaxError(
                "Local type declaration (class, struct, interface and enum) support is not yet implemented.");
        }
    }

    auto *body = AllocNode<ir::TSInterfaceBody>(std::move(members));
    body->SetRange({bodyStart, Lexer()->GetToken().End()});

    const auto isExternal = IsExternal();
    auto *interfaceDecl = AllocNode<ir::TSInterfaceDeclaration>(
        Allocator(), name, typeParamDecl, body, std::move(extends), isStatic, isExternal, GetContext().GetLanguage());

    Lexer()->NextToken();
    GetContext().Status() &= ~ParserStatus::ALLOW_THIS_TYPE;

    return interfaceDecl;
}

ir::Statement *ETSParser::ParseInterfaceDeclaration(bool isStatic)
{
    lexer::SourcePosition interfaceStart = Lexer()->GetToken().Start();
    Lexer()->NextToken();  // eat interface keyword

    auto *id = ExpectIdentifier(false, true);

    auto *declNode = ParseInterfaceBody(id, isStatic);

    declNode->SetRange({interfaceStart, Lexer()->GetToken().End()});
    return declNode;
}

// NOLINTNEXTLINE(google-default-arguments)
ir::ClassDefinition *ETSParser::ParseClassDefinition(ir::ClassDefinitionModifiers modifiers, ir::ModifierFlags flags)
{
    Lexer()->NextToken();

    ir::Identifier *identNode = ParseClassIdent(modifiers);

    ir::TSTypeParameterDeclaration *typeParamDecl = nullptr;
    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LESS_THAN) {
        auto options =
            TypeAnnotationParsingOptions::THROW_ERROR | TypeAnnotationParsingOptions::ALLOW_DECLARATION_SITE_VARIANCE;
        typeParamDecl = ParseTypeParameterDeclaration(&options);
    }

    // Parse SuperClass
    auto [superClass, superTypeParams] = ParseSuperClass();

    if (superClass != nullptr) {
        modifiers |= ir::ClassDefinitionModifiers::HAS_SUPER;
        GetContext().Status() |= ParserStatus::ALLOW_SUPER;
    }

    if (InAmbientContext()) {
        flags |= ir::ModifierFlags::DECLARE;
    }

    // Parse implements clause
    ArenaVector<ir::TSClassImplements *> implements(Allocator()->Adapter());
    if (Lexer()->GetToken().KeywordType() == lexer::TokenType::KEYW_IMPLEMENTS) {
        Lexer()->NextToken();
        implements = ParseClassImplementClause();
    }

    ExpectToken(lexer::TokenType::PUNCTUATOR_LEFT_BRACE, false);

    // Parse ClassBody
    auto [ctor, properties, bodyRange] = ParseClassBody(modifiers, flags, identNode);

    auto *classDefinition = AllocNode<ir::ClassDefinition>(
        util::StringView(), identNode, typeParamDecl, superTypeParams, std::move(implements), ctor, superClass,
        std::move(properties), modifiers, flags, GetContext().GetLanguage());

    classDefinition->SetRange(bodyRange);

    GetContext().Status() &= ~ParserStatus::ALLOW_SUPER;

    return classDefinition;
}

ir::ModifierFlags ETSParser::ParseInterfaceMethodModifiers()
{
    ir::ModifierFlags flags = ir::ModifierFlags::NONE;

    while (IsInterfaceMethodModifier(Lexer()->GetToken().Type())) {
        ir::ModifierFlags currentFlag = ir::ModifierFlags::NONE;

        if ((GetContext().Status() & ParserStatus::FUNCTION) != 0) {
            if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_PUBLIC ||
                Lexer()->GetToken().Type() == lexer::TokenType::KEYW_PROTECTED ||
                Lexer()->GetToken().Type() == lexer::TokenType::KEYW_PRIVATE) {
                ThrowSyntaxError("Local interface declaration members can not have access modifies",
                                 Lexer()->GetToken().Start());
            }
        } else if (Lexer()->GetToken().Type() == lexer::TokenType::KEYW_PUBLIC ||
                   Lexer()->GetToken().Type() == lexer::TokenType::KEYW_PROTECTED) {
            break;
        }
        switch (Lexer()->GetToken().Type()) {
            case lexer::TokenType::KEYW_STATIC: {
                currentFlag = ir::ModifierFlags::STATIC;
                break;
            }
            case lexer::TokenType::KEYW_PRIVATE: {
                currentFlag = ir::ModifierFlags::PRIVATE;
                break;
            }
            default: {
                UNREACHABLE();
            }
        }

        char32_t nextCp = Lexer()->Lookahead();
        if (nextCp == lexer::LEX_CHAR_COLON || nextCp == lexer::LEX_CHAR_LEFT_PAREN ||
            nextCp == lexer::LEX_CHAR_EQUALS) {
            break;
        }

        if ((flags & currentFlag) != 0) {
            ThrowSyntaxError("Duplicated modifier is not allowed");
        }

        Lexer()->NextToken();
        flags |= currentFlag;
    }

    return flags;
}

ir::ClassProperty *ETSParser::ParseInterfaceField()
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT);
    auto *name = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
    name->SetRange(Lexer()->GetToken().Loc());
    Lexer()->NextToken();
    bool optionalField = false;

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_QUESTION_MARK) {
        Lexer()->NextToken();  // eat '?'
        optionalField = true;
    }

    ir::TypeNode *typeAnnotation = nullptr;
    if (!Lexer()->TryEatTokenType(lexer::TokenType::PUNCTUATOR_COLON)) {
        ThrowSyntaxError("Interface fields must have type annotation.");
    }
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR;
    typeAnnotation = ParseTypeAnnotation(&options);

    name->SetTsTypeAnnotation(typeAnnotation);
    typeAnnotation->SetParent(name);

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_EQUAL) {
        ThrowSyntaxError("Initializers are not allowed on interface properties.");
    }

    ir::ModifierFlags fieldModifiers = ir::ModifierFlags::PUBLIC;

    if (InAmbientContext()) {
        fieldModifiers |= ir::ModifierFlags::DECLARE;
    }

    auto *field = AllocNode<ir::ClassProperty>(name, nullptr, typeAnnotation->Clone(Allocator(), nullptr),
                                               fieldModifiers, Allocator(), false);
    if (optionalField) {
        field->AddModifier(ir::ModifierFlags::OPTIONAL);
    }
    field->SetEnd(Lexer()->GetToken().End());

    return field;
}

ir::MethodDefinition *ETSParser::ParseInterfaceMethod(ir::ModifierFlags flags, ir::MethodDefinitionKind methodKind)
{
    ASSERT(Lexer()->GetToken().Type() == lexer::TokenType::LITERAL_IDENT);
    auto *name = AllocNode<ir::Identifier>(Lexer()->GetToken().Ident(), Allocator());
    name->SetRange(Lexer()->GetToken().Loc());
    Lexer()->NextToken();

    FunctionContext functionContext(this, ParserStatus::FUNCTION);

    lexer::SourcePosition startLoc = Lexer()->GetToken().Start();

    auto [signature, throwMarker] = ParseFunctionSignature(ParserStatus::NEED_RETURN_TYPE);

    ir::BlockStatement *body = nullptr;

    bool isDeclare = InAmbientContext();
    if (isDeclare) {
        flags |= ir::ModifierFlags::DECLARE;
    }

    if (Lexer()->GetToken().Type() == lexer::TokenType::PUNCTUATOR_LEFT_BRACE) {
        if (methodKind == ir::MethodDefinitionKind::SET || methodKind == ir::MethodDefinitionKind::GET) {
            ThrowSyntaxError("Getter and setter methods must be abstracts in the interface body", startLoc);
        }
        body = ParseBlockStatement();
    } else if ((flags & (ir::ModifierFlags::PRIVATE | ir::ModifierFlags::STATIC)) != 0 && !isDeclare) {
        ThrowSyntaxError("Private or static interface methods must have body", startLoc);
    }

    functionContext.AddFlag(throwMarker);

    if ((GetContext().Status() & ParserStatus::FUNCTION_HAS_RETURN_STATEMENT) != 0) {
        functionContext.AddFlag(ir::ScriptFunctionFlags::HAS_RETURN);
        GetContext().Status() ^= ParserStatus::FUNCTION_HAS_RETURN_STATEMENT;
    }

    auto *func = AllocNode<ir::ScriptFunction>(
        Allocator(), ir::ScriptFunction::ScriptFunctionData {body, std::move(signature), functionContext.Flags(), flags,
                                                             true, GetContext().GetLanguage()});

    if ((flags & ir::ModifierFlags::STATIC) == 0 && body == nullptr) {
        func->AddModifier(ir::ModifierFlags::ABSTRACT);
    }
    func->SetRange({startLoc, body != nullptr                           ? body->End()
                              : func->ReturnTypeAnnotation() != nullptr ? func->ReturnTypeAnnotation()->End()
                              : func->Params().empty()                  ? Lexer()->GetToken().End()
                                                                        : (*func->Params().end())->End()});

    auto *funcExpr = AllocNode<ir::FunctionExpression>(func);
    funcExpr->SetRange(func->Range());
    func->AddFlag(ir::ScriptFunctionFlags::METHOD);

    func->SetIdent(name);
    auto *method = AllocNode<ir::MethodDefinition>(ir::MethodDefinitionKind::METHOD,
                                                   name->Clone(Allocator(), nullptr)->AsExpression(), funcExpr, flags,
                                                   Allocator(), false);
    method->SetRange(funcExpr->Range());

    func->Id()->SetReference();

    ConsumeSemicolon(method);

    return method;
}

ir::Identifier *ETSParser::ParseClassIdent([[maybe_unused]] ir::ClassDefinitionModifiers modifiers)
{
    return ExpectIdentifier(false, true);
}

// NOLINTNEXTLINE(google-default-arguments)
ir::ClassDeclaration *ETSParser::ParseClassStatement([[maybe_unused]] StatementParsingFlags flags,
                                                     [[maybe_unused]] ir::ClassDefinitionModifiers modifiers,
                                                     [[maybe_unused]] ir::ModifierFlags modFlags)
{
    return ParseClassDeclaration(modifiers | ir::ClassDefinitionModifiers::ID_REQUIRED |
                                     ir::ClassDefinitionModifiers::CLASS_DECL | ir::ClassDefinitionModifiers::LOCAL,
                                 modFlags);
}

//================================================================================================//
//  ExternalSourceParser class
//================================================================================================//

ExternalSourceParser::ExternalSourceParser(ETSParser *parser, Program *newProgram)
    : parser_(parser),
      savedProgram_(parser_->GetProgram()),
      savedLexer_(parser_->Lexer()),
      savedTopScope_(parser_->GetProgram()->VarBinder()->TopScope())
{
    parser_->SetProgram(newProgram);
    parser_->GetContext().SetProgram(newProgram);
}

ExternalSourceParser::~ExternalSourceParser()
{
    parser_->SetLexer(savedLexer_);
    parser_->SetProgram(savedProgram_);
    parser_->GetContext().SetProgram(savedProgram_);
    parser_->GetProgram()->VarBinder()->ResetTopScope(savedTopScope_);
}

//================================================================================================//
//  InnerSourceParser class
//================================================================================================//

InnerSourceParser::InnerSourceParser(ETSParser *parser)
    : parser_(parser),
      savedLexer_(parser_->Lexer()),
      savedSourceCode_(parser_->GetProgram()->SourceCode()),
      savedSourceFile_(parser_->GetProgram()->SourceFilePath()),
      savedSourceFilePath_(parser_->GetProgram()->SourceFileFolder())
{
}

InnerSourceParser::~InnerSourceParser()
{
    parser_->SetLexer(savedLexer_);
    parser_->GetProgram()->SetSource(savedSourceCode_, savedSourceFile_, savedSourceFilePath_);
}

ir::TypeNode *ETSParser::ParseInterfaceExtendsElement()
{
    TypeAnnotationParsingOptions options = TypeAnnotationParsingOptions::THROW_ERROR |
                                           TypeAnnotationParsingOptions::IGNORE_FUNCTION_TYPE |
                                           TypeAnnotationParsingOptions::ALLOW_WILDCARD;
    return ParseTypeReference(&options);
}

}  // namespace ark::es2panda::parser