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

#ifndef ES2PANDA_COMPILER_CHECKER_ETS_ETS_WARNING_ANALYZER_H
#define ES2PANDA_COMPILER_CHECKER_ETS_ETS_WARNING_ANALYZER_H

#include "checker/ETSchecker.h"

namespace ark::es2panda::checker {
class ETSWarningAnalyzer {
public:
    ETSWarningAnalyzer(const ir::AstNode *node, parser::Program *program, const ETSWarnings warning, bool etsWerror,
                       bool etsAutoFix)
        : program_(program), etsWerror_(etsWerror), etsAutoFix_(etsAutoFix)
    {
        if (node == nullptr) {
            return;
        }

        switch (warning) {
            case ETSWarnings::SUGGEST_FINAL:
                ETSWarningSuggestFinal(node);
                break;
            case ETSWarnings::WRAP_TOP_LEVEL_STATEMENTS:
                ETSWarningWrapTopLevelStatements(node);
                break;
            case ETSWarnings::BOOST_EQUALITY_EXPRESSION:
                ETSWarningBoostEqualityExpression(node);
                break;
            case ETSWarnings::REMOVE_ASYNC_FUNCTIONS:
                ETSWarningRemoveAsync(node);
                break;
            case ETSWarnings::REMOVE_LAMBDA:
                ETSWarningRemoveLambda(node);
                break;
            case ETSWarnings::IMPLICIT_BOXING_UNBOXING:
                ETSWarningImplicitBoxingUnboxing(node);
                break;
            case ETSWarnings::REMOVE_REST_PARAMETERS:
                ETSWarningRemoveRestParameters(node);
                break;
            default:
                break;
        }
    }

private:
    void ETSThrowWarning(const std::string &message, const lexer::SourcePosition &position);

    void AutoFixSourceFile(const util::StringView &sourcePath, const std::string &tmpPath, size_t whereLine, const ETSWarnings warning, [[maybe_unused]] ir::AstNode *node);
    bool AnalyzeLocalClasssesForFinalModifierInMethodDef(const ir::AstNode *body, const ir::ClassDefinition *classDef);
    void AnalyzeClassDefForFinalModifier(const ir::ClassDefinition *classDef);
    void AnalyzeClassMethodForFinalModifier(const ir::MethodDefinition *methodDef, const ir::ClassDefinition *classDef);
    void CheckTypeOfBoxing(const ir::AstNode *node);
    void CheckTypeOfUnboxing(const ir::AstNode *node);
    void CheckTopLevelExpressions(const ir::Expression *expression);
    void CheckProhibitedTopLevelStatements(const ir::Statement *statement);
    std::string GetBoxingUnboxingType(const ir::AstNode *node);
    void CheckTypeOfBoxingUnboxing(const ir::AstNode *node);

    void ETSWarningSuggestFinal(const ir::AstNode *node);
    void ETSWarningWrapTopLevelStatements(const ir::AstNode *node);
    void ETSWarningBoostEqualityExpression(const ir::AstNode *node);
    void ETSWarningRemoveAsync(const ir::AstNode *node);
    void ETSWarningRemoveLambda(const ir::AstNode *node);
    void ETSWarningImplicitBoxingUnboxing(const ir::AstNode *node);
    void ETSWarningRemoveRestParameters(const ir::AstNode *node);

    parser::Program *program_;
    bool etsWerror_;
    bool etsAutoFix_;
};
}  // namespace ark::es2panda::checker

#endif
