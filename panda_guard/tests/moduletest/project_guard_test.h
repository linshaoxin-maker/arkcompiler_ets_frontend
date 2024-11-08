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

#ifndef PANDA_GUARD_MODULE_TEST_PROJECT_GUARD_TEST_H
#define PANDA_GUARD_MODULE_TEST_PROJECT_GUARD_TEST_H

#include <gtest/gtest.h>
#include "abc2program_compiler.h"
#include "obfuscate/program.h"

namespace panda::guard {
    class ProjectGuardTest : public testing::Test {
    public:
        [[nodiscard]] virtual std::string GetProjectName() const = 0;

        virtual std::string GetConfigName();

        void SetUp() override;

        void TearDown() override;

        void DoObfuscation() const;

        void ValidateNameCache();

        void ValidatePa();

    protected:
        void DumpAndRun();

        std::shared_ptr<Program> guardProgram_ = nullptr;

        pandasm::Program asmProgram_{};

        bool hasDump = true;

        bool hasRun = true;
    private:
        std::unique_ptr<abc2program::Abc2ProgramCompiler> compiler_ = nullptr;
    };
}

#define DECLARE_MODULE_TEST_CLASS(class_name, projectName, configName)\
class class_name##_##projectName##_##configName : public panda::guard::ProjectGuardTest {\
public:\
    [[nodiscard]] std::string GetProjectName() const override\
    {\
        return #projectName;\
    }\
\
    std::string GetConfigName() override\
    {\
        return #configName;\
    }\
};

#define DECLARE_VALIDATE_PA_AND_NAME_CACHE_MODULE_TEST(projectName, configName)\
DECLARE_MODULE_TEST_CLASS(ValidatePaAndNameCache, projectName, configName)\
HWTEST_F(ValidatePaAndNameCache_##projectName##_##configName, should_success_when_validate_pa_and_name_cache_with_##configName, TestSize.Level4)\
{\
    DoObfuscation();\
    ValidateNameCache();\
    DumpAndRun();\
    ValidatePa();\
}

#define DECLARE_VALIDATE_PA_AND_NAME_CACHE_NOT_RUN_ABC_MODULE_TEST(projectName, configName)\
DECLARE_MODULE_TEST_CLASS(ValidatePaAndNameCache, projectName, configName)\
HWTEST_F(ValidatePaAndNameCache_##projectName##_##configName, should_success_when_validate_pa_and_name_cache_with_##configName, TestSize.Level4)\
{\
    hasRun = false;\
    DoObfuscation();\
    ValidateNameCache();\
    DumpAndRun();\
    ValidatePa();\
}

#endif // PANDA_GUARD_MODULE_TEST_PROJECT_GUARD_TEST_H
