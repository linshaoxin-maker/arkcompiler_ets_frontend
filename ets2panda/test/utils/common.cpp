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
#include <algorithm>
#include <gtest/gtest.h>

#include "common.h"
#include "ir/base/classDefinition.h"
#include "ir/expressions/identifier.h"
#include "ir/statement.h"
#include "ir/astNode.h"
#include "parser/ETSparser.h"
#include "varbinder/ETSBinder.h"
#include "varbinder/variableFlags.h"
#include "checker/ETSchecker.h"
#include "es2panda.h"
#include "util/options.h"

namespace test::utils {

ir_alias::Identifier *ScopeInitTest::BodyToFirstName(ir_alias::Statement *body)
{
    return body->AsBlockStatement()
        ->Statements()
        .front()
        ->AsVariableDeclaration()
        ->Declarators()
        .front()
        ->Id()
        ->AsIdentifier();
}

checker_alias::Type *CheckerTest::FindClassType(varbinder_alias::ETSBinder *varbinder, std::string_view className)
{
    auto classDefs = varbinder->AsETSBinder()->GetRecordTable()->ClassDefinitions();
    auto baseClass = std::find_if(classDefs.begin(), classDefs.end(), [className](ir_alias::ClassDefinition *cdef) {
        return cdef->Ident()->Name().Is(className);
    });
    if (baseClass == classDefs.end()) {
        return nullptr;
    }
    return (*baseClass)->TsType();
}

checker_alias::Type *CheckerTest::FindTypeAlias(checker_alias::ETSChecker *checker, std::string_view aliasName)
{
    auto *foundVar =
        checker->Scope()->FindLocal(aliasName, varbinder_alias::ResolveBindingOptions::TYPE_ALIASES)->AsLocalVariable();
    if (foundVar == nullptr) {
        return nullptr;
    }
    return foundVar->Declaration()->Node()->AsTSTypeAliasDeclaration()->TypeAnnotation()->TsType();
}

es2panda_Context *CheckerTest::CreateContextAndProceedToState(const es2panda_Impl *impl, es2panda_Config *config,
                                                              char const *source, char const *fileName,
                                                              es2panda_ContextState state)
{
    es2panda_Context *ctx = impl->CreateContextFromString(config, source, fileName);
    impl->ProceedToState(ctx, state);
    return ctx;
}

verifier_alias::Messages CheckerTest::VerifyCheck(verifier_alias::ASTVerifier &verifier, const ir_alias::AstNode *ast,
                                                  const std::string &check, verifier_alias::InvariantNameSet &checks)
{
    checks.insert(check);
    return verifier.Verify(ast, checks);
}

verifier_alias::Messages CheckerTest::VerifyCheck(verifier_alias::ASTVerifier &verifier, const ir_alias::AstNode *ast,
                                                  const std::string &check)
{
    verifier_alias::InvariantNameSet checks;
    checks.insert(check);
    return verifier.Verify(ast, checks);
}

ark::pandasm::Function *AsmTest::GetFunction(std::string_view functionName,
                                             const std::unique_ptr<ark::pandasm::Program> &program)
{
    auto it = program->functionTable.find(functionName.data());
    if (it == program->functionTable.end()) {
        return nullptr;
    }
    return &it->second;
}

void AsmTest::CompareActualWithExpected(const std::string &expectedValue, ark::pandasm::ScalarValue *scalarValue,
                                        const std::string &field)
{
    std::string actualValue = std::visit(
        [](const auto &value) -> std::string {
            using ValueType = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<ValueType, std::string>) {
                return value;
            } else if constexpr (std::is_arithmetic_v<ValueType>) {
                return std::to_string(value);
            } else {
                return "Unsupported type";
            }
        },
        scalarValue->GetValue());

    ASSERT_EQ(actualValue, expectedValue) << "Value mismatch for " + field;
}

void AsmTest::CheckAnnoDecl(ark::pandasm::Program *program, const std::string &annoName,
                            const std::vector<std::pair<std::string, std::string>> &expectedAnnotations)
{
    const auto &recordTable = program->recordTable;
    ASSERT_FALSE(recordTable.empty()) << "No records found in the program.";
    auto found = recordTable.find(annoName);
    ASSERT_NE(found, recordTable.end());

    for (size_t i = 0; i < expectedAnnotations.size(); i++) {
        auto scalarValue = found->second.fieldList[i].metadata->GetValue();
        if (scalarValue) {
            CompareActualWithExpected(expectedAnnotations[i].second, &*scalarValue, found->second.fieldList[i].name);
        }
    }
}

void AsmTest::CheckLiteralArrayTable(
    ark::pandasm::Program *program,
    const std::vector<std::pair<std::string, std::vector<AnnotationValueType>>> &expectedLiteralArrayTable)
{
    const auto &literalarrayTable = program->literalarrayTable;
    ASSERT_FALSE(literalarrayTable.empty()) << "literalarrayTable is empty!";
    for (const auto &literalArray : expectedLiteralArrayTable) {
        auto found = literalarrayTable.find(literalArray.first);
        ASSERT_NE(found, literalarrayTable.end());
        size_t i = 1;
        for (const auto &value : literalArray.second) {
            constexpr int STRIDE = 2;
            ASSERT_EQ(value, found->second.literals[i].value) << "Value mismatch for " + literalArray.first;
            i += STRIDE;
        }
    }
}

void AsmTest::CheckAnnotation(const std::vector<std::pair<std::string, std::string>> &expectedValues,
                              const ark::pandasm::AnnotationData &annotation)
{
    for (const auto &element : annotation.GetElements()) {
        auto it = std::find_if(expectedValues.begin(), expectedValues.end(),
                               [&element](const auto &pair) { return pair.first == element.GetName(); });
        if (it != expectedValues.end()) {
            CompareActualWithExpected(it->second, element.GetValue()->GetAsScalar(), element.GetName());
        }
    }
}

void AsmTest::CheckClassAnnotations(ark::pandasm::Program *program, const std::string &className,
                                    const AnnotationMap &expectedAnnotations)
{
    const auto &recordTable = program->recordTable;
    ASSERT_FALSE(recordTable.empty()) << "No records found in the program.";
    auto found = recordTable.find(className);
    ASSERT_NE(found, recordTable.end());

    for (const auto &anno : found->second.metadata->GetAnnotations()) {
        auto it = expectedAnnotations.find(anno.GetName());
        ASSERT_NE(it, expectedAnnotations.end()) << "Unexpected annotation: " << anno.GetName();

        // Check the fields for the matched annotation name
        CheckAnnotation(it->second, anno);
    }
}

void AsmTest::CheckFunctionAnnotations(ark::pandasm::Program *program, const std::string &functionName,
                                       const AnnotationMap &expectedAnnotations)
{
    const auto &functionTable = program->functionTable;
    auto found = functionTable.find(functionName);
    ASSERT_NE(found, functionTable.end());

    for (const auto &annotation : found->second.metadata->GetAnnotations()) {
        auto it = expectedAnnotations.find(annotation.GetName());
        ASSERT_NE(it, expectedAnnotations.end()) << "Unexpected annotation: " << annotation.GetName();

        // Check the fields for the matched annotation name
        CheckAnnotation(it->second, annotation);
    }
}

}  // namespace test::utils