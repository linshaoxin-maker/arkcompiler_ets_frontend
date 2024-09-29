
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

#include "etsWarningAnalyzer.h"

#include "parser/program/program.h"
#include "util/options.h"
#include "ir/expressions/binaryExpression.h"
#include "ir/base/methodDefinition.h"
#include "ir/base/scriptFunction.h"
#include "ir/statements/classDeclaration.h"
#include "ir/statements/expressionStatement.h"
#include "ir/statements/blockStatement.h"
#include "ir/expressions/assignmentExpression.h"
#include "ir/expressions/callExpression.h"
#include "ir/expressions/identifier.h"
#include "ir/expressions/memberExpression.h"
#include "ir/ets/etsTypeReferencePart.h"
#include "ir/ets/etsTypeReference.h"
#include "ir/base/classDefinition.h"
#include "ir/statements/forOfStatement.h"
#include "ir/statements/variableDeclarator.h"
#include "ir/statements/variableDeclaration.h"
#include "ir/expressions/updateExpression.h"

namespace ark::es2panda::checker {

size_t DeleteSpaces(std::string& string)
{
    size_t strBegin=string.find_first_not_of(' ');
    string.erase(0, strBegin);
    return strBegin;
}

void ETSWarningAnalyzer::AutoFixSourceFile(const util::StringView &sourcePath, const std::string &tmpPath, size_t whereLine, const ETSWarnings warning, [[maybe_unused]] ir::AstNode *node)
{
    std::ifstream sourceFile(static_cast<std::string>(sourcePath));
    std::rename((static_cast<std::string>(sourcePath)).c_str(), (static_cast<std::string>(tmpPath)).c_str());
    std::ofstream destFile(static_cast<std::string>(sourcePath));

    std::string line;
    size_t currentLine = 0;
    std::string space = "";

    switch (warning) {
        case ETSWarnings::SUGGEST_FINAL: {
            while (std::getline(sourceFile, line) && !sourceFile.eof()) {
                if (currentLine == whereLine) {
                    size_t shift = DeleteSpaces(line);
                    space.resize(shift, ' ');
                    line = space + "final " + line;
                }
                destFile << line << std::endl;
                currentLine++;
            }
            break;
        }
        case ETSWarnings::BOOST_EQUALITY_EXPRESSION: {
            // auto binExpr = node->AsBinaryExpression();
            // auto operatorType = binExpr->OperatorType();

            while (std::getline(sourceFile, line) && !sourceFile.eof()) {
                // std::string lhsBefore = line;
                // std::string rhsAfter = line;
                // std::string lhs = line;
                // std::string rhs = line;
                // if (currentLine == whereLine) {
                //     rhs = rhs.substr(binExpr->Right()->Start().index, binExpr->Right()->End().index - binExpr->Right()->Start().index + 1);
                //     lhs = lhs.substr(binExpr->Left()->Start().index, binExpr->Left()->End().index - binExpr->Left()->Start().index + 1);
                //     lhsBefore = lhsBefore.substr(0, binExpr->Left()->Start().index + 1);
                //     rhsAfter = rhsAfter.substr(binExpr->Left()->End().index, line.size() - 1 - binExpr->Left()->End().index);
                //     line = lhsBefore + rhs + lexer::TokenToString(operatorType) + lhs + rhsAfter;
                // }
                destFile << line << std::endl;
                currentLine++;
            }
            break;
        }
        case ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS: {
            ETSThrowWarning("Autofixing for ets-wrap-top-level-statements is not supported yet", node->Start());
            break;
        }
        case ETSWarnings::REMOVE_ASYNC_FUNCTIONS: {
            ETSThrowWarning("Autofixing for ets-remove-async is not supported yet", node->Start());
            break;
        }
        case ETSWarnings::REMOVE_LAMBDA: {
            ETSThrowWarning("Autofixing for ets-remove-lambda is not supported yet", node->Start());
            break;
        }
        case ETSWarnings::IMPLICIT_BOXING_UNBOXING: {
            ETSThrowWarning("Autofixing for ets-implicit-boxing-unboxing is not supported yet", node->Start());
            break;
        }
        case ETSWarnings::REMOVE_REST_PARAMETERS: {
            ETSThrowWarning("Autofixing for ets-rest-parameters is not supported yet", node->Start());
            break;
        }
        default:
            break;
    }

    sourceFile.close();
    destFile.close();
}

bool ETSWarningAnalyzer::AnalyzeLocalClasssesForFinalModifierInMethodDef(const ir::AstNode *body,
                                                                         const ir::ClassDefinition *classDef)
{
    for (const auto *statement : body->AsMethodDefinition()->Function()->Body()->AsBlockStatement()->Statements()) {
        if (program_->NodeContainsETSNolint(statement, ETSWarnings::SUGGEST_FINAL)) {
            continue;
        }

        if (!statement->IsClassDeclaration()) {
            continue;
        }

        const auto *superClass = statement->AsClassDeclaration()->Definition()->Super();

        if (superClass == nullptr) {
            continue;
        }

        if (superClass->IsETSTypeReference() && superClass->AsETSTypeReference()->Part()->Name()->IsIdentifier() &&
            (superClass->AsETSTypeReference()->Part()->Name()->AsIdentifier()->Name() == classDef->Ident()->Name())) {
            return true;
        }
    }
    return false;
}

void ETSWarningAnalyzer::AnalyzeClassDefForFinalModifier(const ir::ClassDefinition *classDef)
{
    ASSERT(classDef != nullptr);

    if (program_ == nullptr || classDef->IsFinal() || classDef->IsAbstract() || classDef->IsStatic() ||
        classDef->IsGlobal() || classDef->IsExported()) {
        return;
    }

    const auto statements = program_->Ast()->Statements();
    for (const auto *it : statements) {
        if (!it->IsClassDeclaration() ||
            classDef->Ident()->Name() == it->AsClassDeclaration()->Definition()->Ident()->Name()) {
            continue;
        }

        const auto *itAsClassDef = it->AsClassDeclaration()->Definition();

        if (!itAsClassDef->IsGlobal()) {
            const auto *superClass = itAsClassDef->Super();
            if (superClass == nullptr) {
                continue;
            }

            if (superClass->IsETSTypeReference() && superClass->AsETSTypeReference()->Part()->Name()->IsIdentifier() &&
                (superClass->AsETSTypeReference()->Part()->Name()->AsIdentifier()->Name() ==
                 classDef->Ident()->Name())) {
                return;
            }
            continue;
        }

        for (const auto *itBody : itAsClassDef->Body()) {
            if (!itBody->IsMethodDefinition() ||
                itBody->AsMethodDefinition()->Id()->Name() == compiler::Signatures::INIT_METHOD) {
                continue;
            }
            if (AnalyzeLocalClasssesForFinalModifierInMethodDef(itBody, classDef)) {
                return;
            }
            // NOTE: Add checks for local classes inside classes, ... after supporting in front-end
        }
    }

    if(etsAutoFix_) {
        size_t whereLine = classDef->Ident()->Start().line;
        auto file = program_->SourceFilePath().Utf8();
        auto fileTmp = static_cast<std::string>(program_->SourceFileFolder()) + "/new_tmp_final.ets";
        AutoFixSourceFile(file, fileTmp, whereLine, ETSWarnings::SUGGEST_FINAL, nullptr);
    }

    ETSThrowWarning("Suggest 'final' modifier for class '" + std::string(classDef->Ident()->Name()) + "'",
                    classDef->Ident()->Start());
}

void ETSWarningAnalyzer::AnalyzeClassMethodForFinalModifier(const ir::MethodDefinition *methodDef,
                                                            const ir::ClassDefinition *classDef)
{
    ASSERT(methodDef != nullptr && classDef != nullptr);

    if (methodDef->IsAbstract() || methodDef->IsStatic() || classDef->IsFinal() || program_ == nullptr ||
        methodDef->IsFinal() || methodDef->IsConstructor() || classDef->IsGlobal()) {
        return;
    }

    bool suggestFinal = true;

    const auto statements = program_->Ast()->Statements();
    for (const auto *it : statements) {
        if (!it->IsClassDeclaration() || it->AsClassDeclaration()->Definition()->IsGlobal() ||
            classDef->Ident()->Name() == it->AsClassDeclaration()->Definition()->Ident()->Name()) {
            continue;
        }

        const auto *statementDef = it->AsClassDeclaration()->Definition();
        for (const auto *bodyPart : statementDef->Body()) {
            if (!bodyPart->IsMethodDefinition()) {
                continue;
            }
            static auto classAsETSObject = classDef->TsType()->AsETSObjectType();
            static auto potentialDescendant = statementDef->TsType()->AsETSObjectType();
            if (!potentialDescendant->IsDescendantOf(classAsETSObject)) {
                continue;
            }
            const util::StringView bodyMethodName =
                ETSChecker::GetSignatureFromMethodDefinition(bodyPart->AsMethodDefinition())->Function()->Id()->Name();
            if (bodyPart->IsOverride() && bodyMethodName != compiler::Signatures::CTOR &&
                bodyMethodName == methodDef->Function()->Id()->Name()) {
                suggestFinal = false;
                break;
            }
        }
    }

    if (suggestFinal) {
        if(etsAutoFix_) {
            size_t whereLine = methodDef->Function()->Start().line;
            auto file = program_->SourceFilePath().Utf8();
            auto fileTmp = static_cast<std::string>(program_->SourceFileFolder()) + "/new_tmp_final.ets";
            AutoFixSourceFile(file, fileTmp, whereLine, ETSWarnings::SUGGEST_FINAL, nullptr);
        }
        ETSThrowWarning("Suggest 'final' modifier for method '" + std::string(methodDef->Function()->Id()->Name()) +
                            "'",
                        methodDef->Function()->Start());
    }
}

void ETSWarningAnalyzer::ETSWarningSuggestFinal(const ir::AstNode *node)
{
    if (node->IsClassDeclaration()) {
        if (node->AsClassDeclaration()->Definition()->IsClassDefinition() &&
            !program_->NodeContainsETSNolint(node, ETSWarnings::SUGGEST_FINAL)) {
            AnalyzeClassDefForFinalModifier(node->AsClassDeclaration()->Definition());
        }

        const auto classBody = node->AsClassDeclaration()->Definition()->Body();
        for (const auto *it : classBody) {
            if (it->IsMethodDefinition() && !program_->NodeContainsETSNolint(it, ETSWarnings::SUGGEST_FINAL) &&
                !program_->NodeContainsETSNolint(node, ETSWarnings::SUGGEST_FINAL)) {
                AnalyzeClassMethodForFinalModifier(it->AsMethodDefinition(), node->AsClassDeclaration()->Definition());
            }
        }
    }
    node->Iterate([&](auto *childNode) { ETSWarningSuggestFinal(childNode); });
}

void ETSWarningAnalyzer::CheckTopLevelExpressions(const ir::Expression *expression)
{
    if (expression->IsCallExpression()) {
        const auto exprCallee = expression->AsCallExpression()->Callee();
        lexer::SourcePosition pos = exprCallee->Start();
        if (exprCallee->IsMemberExpression()) {
            pos = exprCallee->AsMemberExpression()->Object()->Start();
            ETSThrowWarning("Wrap top-level call expressions", pos);
        }
    } else if (expression->IsAssignmentExpression()) {
        const auto assignmentExpr = expression->AsAssignmentExpression();
        // This is special for assignment statements with declaration like 'let i: int = 0;'
        // That is just because ETSParser.cpp changed top-level statements parsing
        if (expression->IsIdentifier() && !program_->NodeContainsETSNolint(expression->AsIdentifier(), ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS)) {
            ETSThrowWarning("Wrap top-level assignment expressions", assignmentExpr->Left()->Start());
            return;
        }
        ETSThrowWarning("Wrap top-level assignment expressions", assignmentExpr->Left()->Start());
    }
}

void ETSWarningAnalyzer::CheckProhibitedTopLevelStatements(const ir::Statement *statement)
{
    switch (statement->Type()) {
        case ir::AstNodeType::ARROW_FUNCTION_EXPRESSION:
        case ir::AstNodeType::AWAIT_EXPRESSION:
        case ir::AstNodeType::FUNCTION_DECLARATION:
        case ir::AstNodeType::SCRIPT_FUNCTION:
        case ir::AstNodeType::ETS_FUNCTION_TYPE:
        case ir::AstNodeType::CLASS_DECLARATION:
        case ir::AstNodeType::CLASS_DEFINITION:
        case ir::AstNodeType::EXPORT_ALL_DECLARATION:
        case ir::AstNodeType::EXPORT_DEFAULT_DECLARATION:
        case ir::AstNodeType::EXPORT_NAMED_DECLARATION:
        case ir::AstNodeType::EXPORT_SPECIFIER:
        case ir::AstNodeType::IMPORT_DECLARATION:
        case ir::AstNodeType::IMPORT_EXPRESSION:
        case ir::AstNodeType::IMPORT_DEFAULT_SPECIFIER:
        case ir::AstNodeType::IMPORT_NAMESPACE_SPECIFIER:
        case ir::AstNodeType::IMPORT_SPECIFIER:
        case ir::AstNodeType::REEXPORT_STATEMENT:
        case ir::AstNodeType::ETS_PACKAGE_DECLARATION:
        case ir::AstNodeType::ETS_IMPORT_DECLARATION:
        case ir::AstNodeType::STRUCT_DECLARATION:
            break;
        default:
            // if (etsAutoFix_) {
            //     // size_t whereLine = statement->Start().line;
            //     auto file = program_->SourceFilePath().Utf8();
            //     auto fileTmp = static_cast<std::string>(program_->SourceFileFolder()) + "/new_tmp_top_level.ets";
            //     AutoFixSourceFile(file, fileTmp, 0, ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS, nullptr);
            // }
            ETSThrowWarning("Wrap top-level statements", statement->Start());
            break;
    }
}

void ETSWarningAnalyzer::ETSWarningWrapTopLevelStatements(const ir::AstNode *node)
{
    if (!node->IsClassDeclaration() ||
        program_->NodeContainsETSNolint(node, ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS)) {
        node->Iterate([&](auto *childNode) { ETSWarningWrapTopLevelStatements(childNode); });
        return;
    }

    const auto *classDef = node->AsClassDeclaration()->Definition();
    if (!classDef->IsGlobal()) {
        return;
    }

    for (const auto *itBody : classDef->Body()) {
        //if (itBody->IsIdentifier()) {
            // std::cout << itBody->DumpEtsSrc() << " in body" << std::endl;
        //}
        if (itBody->IsClassProperty()) {
            // std::cout << itBody->DumpEtsSrc() << " in class prop "<< std::endl;
            // This is special for assignment statements with class property like let i: int;
            // That is just because ETSParser.cpp changed top-level statements parsing
            // std::cout << itBody->DumpEtsSrc() << std::endl;
            if (itBody->IsIdentifier() && !program_->NodeContainsETSNolint(itBody->AsIdentifier(), ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS)) {
                // if (etsAutoFix_) {
                //     size_t whereLine = itBody->Start().line;
                //     auto file = program_->SourceFilePath().Utf8();
                //     auto fileTmp = static_cast<std::string>(program_->SourceFileFolder()) + "/new_tmp_top_level.ets";
                //     AutoFixSourceFile(file, fileTmp, whereLine, ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS, nullptr);
                // }
                ETSThrowWarning("Wrap top-level declarations", itBody->Start());
                return;
            }
            // if (etsAutoFix_) {
            //     size_t whereLine = itBody->Start().line;
            //     auto file = program_->SourceFilePath().Utf8();
            //     auto fileTmp = static_cast<std::string>(program_->SourceFileFolder()) + "/new_tmp_top_level.ets";
            //     AutoFixSourceFile(file, fileTmp, whereLine, ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS, nullptr);
            // }
            ETSThrowWarning("Wrap top-level declarations", itBody->Start());
            return;
        }
        if (!itBody->IsMethodDefinition() ||
            itBody->AsMethodDefinition()->Id()->Name() != compiler::Signatures::INIT_METHOD) {
            continue;
        }
        for (const auto *statement :
             itBody->AsMethodDefinition()->Function()->Body()->AsBlockStatement()->Statements()) {
            if (statement->IsExpressionStatement() && statement->AsExpressionStatement()->GetExpression()->IsAssignmentExpression()) {
                //std::cout << statement->DumpEtsSrc() << " gotcha after init check" << std::endl;
                if (itBody->IsIdentifier() && program_->NodeContainsETSNolint(itBody->AsIdentifier(), ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS)) {
                    std::cout << statement->DumpEtsSrc() << " gotcha AGAIN after init check" << std::endl;
                }
            }
            // std::cout << statement->DumpEtsSrc() << " after init check" << std::endl;
            if (program_->NodeContainsETSNolint(itBody->AsMethodDefinition()->Function()->Body()->AsBlockStatement(), ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS)) {
                //std::cout << "gotcha" << std::endl;
                continue;
            }
            if (program_->NodeContainsETSNolint(statement, ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS) ||
                program_->NodeContainsETSNolint(node, ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS)) {
                continue;
            }

            if (!statement->IsExpressionStatement()) {
                CheckProhibitedTopLevelStatements(statement);
                continue;
            }

            // Rewrite this part after fixing AST issue about top-level
            CheckTopLevelExpressions(statement->AsExpressionStatement()->GetExpression());
        }
    }
}

void ETSWarningAnalyzer::ETSWarningBoostEqualityExpression(const ir::AstNode *node)
{
    ASSERT(node != nullptr);

    if (node->IsBinaryExpression() && !program_->NodeContainsETSNolint(node, ETSWarnings::BOOST_EQUALITY_EXPRESSION)) {
        auto binExpr = node->AsBinaryExpression();
        if (binExpr->OperatorType() == lexer::TokenType::PUNCTUATOR_EQUAL ||
            binExpr->OperatorType() == lexer::TokenType::PUNCTUATOR_NOT_EQUAL) {
            if (binExpr->Right()->IsNullLiteral() && !binExpr->Left()->IsNullLiteral()) {
                if (etsAutoFix_) {
                    size_t whereLine =  node->Start().line;
                    auto file = program_->SourceFilePath().Utf8();
                    auto fileTmp = static_cast<std::string>(program_->SourceFileFolder()) + "/new_tmp_equality.ets";
                    AutoFixSourceFile(file, fileTmp, whereLine, ETSWarnings::BOOST_EQUALITY_EXPRESSION, const_cast<ir::AstNode *>(node));
                }
                ETSThrowWarning("Boost Equality Expression. Change sides of binary expression", node->Start());
            }
        }
    }
    node->Iterate([&](auto *childNode) { ETSWarningBoostEqualityExpression(childNode); });
}

void ETSWarningAnalyzer::ETSWarningRemoveAsync(const ir::AstNode *node)
{
    if (node->IsMethodDefinition() && !program_->NodeContainsETSNolint(node, ETSWarnings::REMOVE_ASYNC_FUNCTIONS)) {
        const auto methodDefinition = node->AsMethodDefinition();
        if (methodDefinition->IsAsync() && methodDefinition->Function()->IsAsyncFunc() &&
            !program_->NodeContainsETSNolint(methodDefinition->Function(), ETSWarnings::REMOVE_ASYNC_FUNCTIONS)) {
            ETSThrowWarning("Replace asynchronous function with coroutine", methodDefinition->Start());
        }
    }
    node->Iterate([&](auto *childNode) { ETSWarningRemoveAsync(childNode); });
}

void ETSWarningAnalyzer::ETSWarningRemoveLambda(const ir::AstNode *node)
{
    ASSERT(node != nullptr);

    if (node->IsArrowFunctionExpression() && !program_->NodeContainsETSNolint(node, ETSWarnings::REMOVE_LAMBDA)) {
        ETSThrowWarning("Replace the lambda function with a regular function", node->Start());
    }
    node->Iterate([&](auto *childNode) { ETSWarningRemoveLambda(childNode); });
}

void ETSWarningAnalyzer::CheckTypeOfBoxing(const ir::AstNode *node)
{
    ASSERT(node != nullptr);
    const auto flags = node->GetBoxingUnboxingFlags();
    if ((flags & ir::BoxingUnboxingFlags::BOXING_FLAG) != 0) {
        switch (static_cast<ir::BoxingUnboxingFlags>(flags & ir::BoxingUnboxingFlags::BOXING_FLAG)) {
            case ir::BoxingUnboxingFlags::BOX_TO_INT:
                ETSThrowWarning("Implicit Boxing to Int" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::BOX_TO_BOOLEAN:
                ETSThrowWarning("Implicit Boxing to Boolean" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::BOX_TO_BYTE:
                ETSThrowWarning("Implicit Boxing to Byte" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::BOX_TO_CHAR:
                ETSThrowWarning("Implicit Boxing to Char" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::BOX_TO_DOUBLE:
                ETSThrowWarning("Implicit Boxing to Double" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::BOX_TO_FLOAT:
                ETSThrowWarning("Implicit Boxing to Float" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::BOX_TO_LONG:
                ETSThrowWarning("Implicit Boxing to Long" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::BOX_TO_SHORT:
                ETSThrowWarning("Implicit Boxing to Short" + GetBoxingUnboxingType(node), node->Start());
                break;
            default:
                break;
        }
    }
}

void ETSWarningAnalyzer::CheckTypeOfUnboxing(const ir::AstNode *node)
{
    ASSERT(node != nullptr);
    const auto flags = node->GetBoxingUnboxingFlags();
    if ((flags & ir::BoxingUnboxingFlags::UNBOXING_FLAG) != 0) {
        switch (static_cast<ir::BoxingUnboxingFlags>(flags & ir::BoxingUnboxingFlags::UNBOXING_FLAG)) {
            case ir::BoxingUnboxingFlags::UNBOX_TO_INT:
                ETSThrowWarning("Implicit Unboxing to int" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::UNBOX_TO_BOOLEAN:
                ETSThrowWarning("Implicit Unboxing to boolean" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::UNBOX_TO_BYTE:
                ETSThrowWarning("Implicit Unboxing to byte" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::UNBOX_TO_CHAR:
                ETSThrowWarning("Implicit Unboxing to char" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::UNBOX_TO_DOUBLE:
                ETSThrowWarning("Implicit Unboxing to double" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::UNBOX_TO_FLOAT:
                ETSThrowWarning("Implicit Unboxing to float" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::UNBOX_TO_LONG:
                ETSThrowWarning("Implicit Unboxing to long" + GetBoxingUnboxingType(node), node->Start());
                break;
            case ir::BoxingUnboxingFlags::UNBOX_TO_SHORT:
                ETSThrowWarning("Implicit Unboxing to short" + GetBoxingUnboxingType(node), node->Start());
                break;
            default:
                break;
        }
    }
}

void ETSWarningAnalyzer::CheckTypeOfBoxingUnboxing(const ir::AstNode *node)
{
    ASSERT(node != nullptr);

    CheckTypeOfBoxing(node);
    CheckTypeOfUnboxing(node);
}

std::string ETSWarningAnalyzer::GetBoxingUnboxingType(const ir::AstNode *node)
{
    ASSERT(node->Parent() != nullptr);
    switch (node->Parent()->Type()) {
        case ir::AstNodeType::VARIABLE_DECLARATOR: {
            return " in Variable Declaration";
        }
        case ir::AstNodeType::CALL_EXPRESSION: {
            return " in Call Method/Function";
        }
        case ir::AstNodeType::SWITCH_STATEMENT: {
            return " in Switch-case Statement";
        }
        case ir::AstNodeType::ASSIGNMENT_EXPRESSION: {
            return " in Assignment Expression";
        }
        case ir::AstNodeType::BINARY_EXPRESSION: {
            return " in Binary Expression";
        }
        case ir::AstNodeType::UNARY_EXPRESSION: {
            return " in Unary Expression";
        }
        case ir::AstNodeType::UPDATE_EXPRESSION: {
            return " in Update Expression";
        }
        case ir::AstNodeType::MEMBER_EXPRESSION: {
            return " in Member Expression";
        }
        default:
            return "";
    }
}

void ETSWarningAnalyzer::ETSWarningImplicitBoxingUnboxing(const ir::AstNode *node)
{
    ASSERT(node != nullptr);

    switch (node->Type()) {
        case ir::AstNodeType::VARIABLE_DECLARATOR:
        case ir::AstNodeType::SWITCH_STATEMENT:
        case ir::AstNodeType::CALL_EXPRESSION:
        case ir::AstNodeType::BINARY_EXPRESSION:
        case ir::AstNodeType::ASSIGNMENT_EXPRESSION:
        case ir::AstNodeType::UNARY_EXPRESSION:
        case ir::AstNodeType::UPDATE_EXPRESSION:
        case ir::AstNodeType::MEMBER_EXPRESSION: {
            if (!program_->NodeContainsETSNolint(node, ETSWarnings::IMPLICIT_BOXING_UNBOXING)) {
                node->Iterate([this](auto *childNode) { CheckTypeOfBoxingUnboxing(childNode); });
            }
            break;
        }
        default: {
            break;
        }
    }

    node->Iterate([&](auto *childNode) { ETSWarningImplicitBoxingUnboxing(childNode); });
}

void ETSWarningAnalyzer::ETSWarningRemoveRestParameters(const ir::AstNode *node)
{
    ASSERT(node != nullptr);

    if (node->IsScriptFunction() && !program_->NodeContainsETSNolint(node, ETSWarnings::REMOVE_REST_PARAMETERS)) {
        for (auto *const it : node->AsScriptFunction()->Params()) {
            auto const *const param = it->AsETSParameterExpression();

            if (param->IsRestParameter()) {
                ETSThrowWarning("Replace rest parameters with usual function or array of parameters", it->Start());
                continue;
            }
        }
    }
    node->Iterate([&](auto *childNode) { ETSWarningRemoveRestParameters(childNode); });
}

void ETSWarningAnalyzer::ETSThrowWarning(const std::string &message, const lexer::SourcePosition &pos)
{
    lexer::LineIndex index(program_->SourceCode());
    lexer::SourceLocation location = index.GetLocation(pos);

    if (etsWerror_) {
        throw Error(ErrorType::ETS_WARNING, ark::es2panda::util::BaseName(program_->SourceFilePath().Utf8()), message,
                    location.line, location.col);
    }

    std::cout << "ETS Warning: " << message << "."
              << " [" << ark::es2panda::util::BaseName(program_->SourceFilePath().Utf8()) << ":" << location.line << ":"
              << location.col << "]" << std::endl;
}

}  // namespace ark::es2panda::checker
