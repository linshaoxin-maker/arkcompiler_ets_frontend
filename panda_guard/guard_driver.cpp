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

#include "guard_driver.h"

#include "abc2program_compiler.h"
#include "program_dump.h"
#include "assembly-emitter.h"
#include "utils/logger.h"
#include "utils/time.h"
#include "util/assert_util.h"
#include "configs/guard_context.h"
#include "guard4program.h"

namespace {
    const std::string TAG = "[Guard_Driver]";

    void Dump(const panda::pandasm::Program &program, const std::string &paFilePath)
    {
        std::ofstream ofs;
        ofs.open(paFilePath, std::ios::trunc | std::ios::out);
        panda::abc2program::PandasmProgramDumper dumper;
        dumper.Dump(ofs, program);
        ofs.close();
    }
}

void panda::guard::GuardDriver::Run(int argc, const char **argv)
{
    uint64_t startTime = time::GetCurrentTimeInMillis();
    do {
        const auto context = GuardContext::GetInstance();
        context->Init(argc, argv);
        LOG(INFO, PANDAGUARD) << TAG << "guard context init success";

        auto options = context->GetGuardOptions();
        if (options->DisableObfuscation()) {
            LOG(ERROR, PANDAGUARD) << TAG << "no need obfuscation";
            break;
        }

        abc2program::Abc2ProgramCompiler compiler;
        bool bRet = compiler.OpenAbcFile(options->GetAbcFilePath());
        PANDA_GUARD_ASSERT_PRINT(!bRet, TAG << "abc 2 program, open abc file failed" << options->GetAbcFilePath());

        auto program = std::move(*compiler.CompileAbcFile());
        LOG(INFO, PANDAGUARD) << TAG << "abc 2 program success";

        context->CreateGraphContext(compiler.GetAbcFile());

        ProgramGuard::GuardProgram(program);
        LOG(INFO, PANDAGUARD) << TAG << "guard 4 program success";

        bRet = pandasm::AsmEmitter::Emit(options->GetObfAbcFilePath(), program, nullptr, nullptr, true, nullptr,
                                         options->GetTargetApiVersion(), options->GetTargetApiSubVersion());
        if (!bRet) {
            if (context->IsDebugMode() && !options->GetObfPaFilePath().empty()) {
                Dump(program, options->GetObfPaFilePath());
                LOG(INFO, PANDAGUARD) << TAG << "program 2 pa success";
            }
            PANDA_GUARD_ABORT_PRINT(TAG << "program 2 abc failed" << pandasm::AsmEmitter::GetLastError());
            break;
        }
        LOG(INFO, PANDAGUARD) << TAG << "program 2 abc success";

        if (context->IsDebugMode() && !options->GetObfPaFilePath().empty()) {
            abc2program::Abc2ProgramCompiler obfCompiler;
            obfCompiler.OpenAbcFile(options->GetObfAbcFilePath());
            program = std::move(*obfCompiler.CompileAbcFile());
            Dump(program, options->GetObfPaFilePath());
            LOG(INFO, PANDAGUARD) << TAG << "program 2 pa success";
        }

        context->Finalize();
    } while (false);

    uint64_t endTime = time::GetCurrentTimeInMillis();
    LOG(ERROR, PANDAGUARD) << TAG << "obf cost time:" << endTime - startTime;
}