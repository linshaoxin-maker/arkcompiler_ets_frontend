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

#include "project_guard_test.h"

#include <fstream>
#include <iostream>

#include <assembly-emitter.h>
#include <program_dump.h>

#include "configs/guard_context.h"
#include "util/file_util.h"

#include "util/test_util.h"

namespace {
    const std::string TEST_PROJECT_OUT_DIR = PANDA_GUARD_PROJECT_OUT_DIR;
    const std::string TEST_EXPECT_DIR = PANDA_GUARD_EXPECT_DIR;
    const std::string ABC_FILE_NAME = "/modules.abc";
    const std::string PA_FILE_SUFFIX = ".pa";
    const std::string ABC_FILE_SUFFIX = ".abc";
    const std::string JSON_FILE_SUFFIX = ".json";
    const std::string CACHE_FILE_SUFFIX = ".cache.json";
    constexpr size_t ARGV_INDEX_2 = 2;

    void Dump(const std::string &abcFilePath, const std::string &paFilePath)
    {
        if (paFilePath.empty()) {
            return;
        }
        panda::abc2program::Abc2ProgramCompiler obfCompiler;
        obfCompiler.OpenAbcFile(abcFilePath);
        auto program = std::move(*obfCompiler.CompileAbcFile());

        std::ofstream ofs;
        ofs.open(paFilePath, std::ios::trunc | std::ios::out);
        panda::abc2program::PandasmProgramDumper dumper;
        dumper.Dump(ofs, program);
        ofs.close();
    }

    void RunAbcCmd(const std::string &projectName, const std::string &configName)
    {
        std::string rootPath = TEST_PROJECT_OUT_DIR;
        size_t pos = rootPath.find("/out");
        EXPECT_NE(pos, std::string::npos);
        rootPath = rootPath.substr(0, pos);

        std::string etsRuntimePath = rootPath + "/out/rk3568/clang_x64/arkcompiler/ets_runtime";
        std::string prebuiltsPath = rootPath + "/prebuilts/clang/ohos/linux-x86_64/llvm/lib";
        std::string zlibPath = rootPath + "/out/rk3568/clang_x64/thirdparty/zlib";
        std::string icuPath = rootPath + "/out/rk3568/clang_x64/thirdparty/icu";
        std::string exportCmd = "export LD_LIBRARY_PATH=" + etsRuntimePath;
        exportCmd.append(":").append(prebuiltsPath);
        exportCmd.append(":").append(zlibPath);
        exportCmd.append(":").append(icuPath);
        exportCmd.append("\n");

        std::string arkJsvmPath = rootPath + "/out/rk3568/clang_x64/arkcompiler/ets_runtime/ark_js_vm";

        std::string abcPath = TEST_PROJECT_OUT_DIR + projectName + ABC_FILE_NAME;
        std::string abcRunCmd = exportCmd + arkJsvmPath + " --entry-point=main " + abcPath;
        EXPECT_EQ(system(abcRunCmd.c_str()), 0);

        std::string obfAbcPath = TEST_PROJECT_OUT_DIR + projectName + "/obf/" + configName + ABC_FILE_SUFFIX;
        std::string obfAbcRunCmd = exportCmd + arkJsvmPath + " --entry-point=main " + obfAbcPath;
        EXPECT_EQ(system(obfAbcRunCmd.c_str()), 0);
    }
}

void panda::guard::ProjectGuardTest::SetUp()
{
    std::string configPath =
            TEST_PROJECT_OUT_DIR + this->GetProjectName() + "/" + this->GetConfigName() + JSON_FILE_SUFFIX;
    int argc = 3;
    char *argv[3];
    argv[0] = const_cast<char *>("xxx");
    argv[1] = const_cast<char *>("--debug");
    argv[ARGV_INDEX_2] = const_cast<char *>(configPath.c_str());
    GuardContext::GetInstance()->Init(argc, const_cast<const char **>(argv));

    this->compiler_ = std::make_unique<abc2program::Abc2ProgramCompiler>();
    bool bRet = this->compiler_->OpenAbcFile(GuardContext::GetInstance()->GetGuardOptions()->GetAbcFilePath());
    EXPECT_TRUE(bRet);

    GuardContext::GetInstance()->CreateGraphContext(this->compiler_->GetAbcFile());

    this->asmProgram_ = std::move(*this->compiler_->CompileAbcFile());
    this->guardProgram_ = std::make_shared<Program>(&asmProgram_);
    this->guardProgram_->Create();
}

void panda::guard::ProjectGuardTest::DumpAndRun()
{
    auto options = GuardContext::GetInstance()->GetGuardOptions();
    bool bRet = pandasm::AsmEmitter::Emit(options->GetObfAbcFilePath(), this->asmProgram_);
    EXPECT_TRUE(bRet);

    if (hasDump) {
        Dump(options->GetObfAbcFilePath(), options->GetObfPaFilePath());
    }

    if (hasRun) {
        RunAbcCmd(this->GetProjectName(), this->GetConfigName());
    }
}

void panda::guard::ProjectGuardTest::TearDown()
{
    GuardContext::GetInstance()->Finalize();
}

void panda::guard::ProjectGuardTest::DoObfuscation() const
{
    guardProgram_->Obfuscate();
}

std::string panda::guard::ProjectGuardTest::GetConfigName()
{
    return "config";
}

void panda::guard::ProjectGuardTest::ValidateNameCache()
{
    auto context = GuardContext::GetInstance();
    context->GetNameCache()->WriteCache();

    std::string nameCachePath = context->GetGuardOptions()->GetDefaultNameCachePath();
    std::string expectNameCachePath = TEST_EXPECT_DIR + "/" + this->GetProjectName() + "/" +
        this->GetConfigName() + CACHE_FILE_SUFFIX;
    TestUtil::ValidateData(nameCachePath, expectNameCachePath);
}

void panda::guard::ProjectGuardTest::ValidatePa()
{
    std::string paPath = TEST_PROJECT_OUT_DIR + this->GetProjectName() + "/obf/" +
        this->GetConfigName() + PA_FILE_SUFFIX;
    std::string expectPaPath = TEST_EXPECT_DIR + "/" + this->GetProjectName() + "/" +
        this->GetConfigName() + PA_FILE_SUFFIX;
    TestUtil::ValidateData(paPath, expectPaPath);
}