/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "generateBin.h"
#include "bytecode_optimizer/bytecodeopt_options.h"
#include "bytecode_optimizer/optimize_bytecode.h"
#include "compiler/compiler_logger.h"
#include "compiler/compiler_options.h"

namespace panda::es2panda::util {

class ProgramGenerator {
public:
    ProgramGenerator(panda::pandasm::Program *prog, const util::Options *options, const ReporterFun &reporter)
        : prog_(prog), options_(options), reporter_(reporter)
    {
        statp_ = options_->OptLevel() != 0 ? &stat_ : nullptr;
        mapsp_ = options_->OptLevel() != 0 ? &maps_ : nullptr;
    }

#ifdef PANDA_WITH_BYTECODE_OPTIMIZER
    int OptimizeBytecode() const
    {
        if (options_->OptLevel() != 0) {
            panda::Logger::ComponentMask componentMask;
            componentMask.set(panda::Logger::Component::ASSEMBLER);
            componentMask.set(panda::Logger::Component::COMPILER);
            componentMask.set(panda::Logger::Component::BYTECODE_OPTIMIZER);

            panda::Logger::InitializeStdLogging(Logger::LevelFromString(options_->LogLevel()), componentMask);

            if (!panda::pandasm::AsmEmitter::Emit(options_->CompilerOutput(), *prog_, statp_, mapsp_, true)) {
                reporter_("Failed to emit binary data: " + panda::pandasm::AsmEmitter::GetLastError());
                return 1;
            }

            panda::bytecodeopt::g_options.SetOptLevel(options_->OptLevel());
            // Set default value instead of maximum set in panda::bytecodeopt::SetCompilerOptions()
            panda::compiler::CompilerLogger::Init({"all"});
            panda::compiler::g_options.SetCompilerMaxBytecodeSize(
                panda::compiler::g_options.GetCompilerMaxBytecodeSize());
            panda::bytecodeopt::OptimizeBytecode(prog_, mapsp_, options_->CompilerOutput(), options_->IsDynamic(),
                                                 true);
        }
        return 0;
    }
#endif

    int GenerateProgram() const
    {
#ifdef PANDA_WITH_BYTECODE_OPTIMIZER
        if (OptimizeBytecode() == 1) {
            return 1;
        }
#endif

        if (options_->CompilerOptions().dumpAsm) {
            es2panda::Compiler::DumpAsm(prog_);
        }

        if (!panda::pandasm::AsmEmitter::AssignProfileInfo(prog_)) {
            reporter_("AssignProfileInfo failed");
            return 1;
        }

        if (!panda::pandasm::AsmEmitter::Emit(options_->CompilerOutput(), *prog_, statp_, mapsp_, true)) {
            reporter_("Failed to emit binary data: " + panda::pandasm::AsmEmitter::GetLastError());
            return 1;
        }

        if (options_->SizeStat()) {
            size_t totalSize = 0;
            std::cout << "Panda file size statistic:" << std::endl;
            constexpr std::array<std::string_view, 2> INFO_STATS = {"instructions_number", "codesize"};

            for (const auto &[name, size] : stat_) {
                if (find(INFO_STATS.begin(), INFO_STATS.end(), name) != INFO_STATS.end()) {
                    continue;
                }
                std::cout << name << " section: " << size << std::endl;
                totalSize += size;
            }

            for (const auto &name : INFO_STATS) {
                std::cout << name << ": " << stat_.at(std::string(name)) << std::endl;
            }

            std::cout << "total: " << totalSize << std::endl;
        }

        return 0;
    }

private:
    panda::pandasm::Program *prog_;
    const util::Options *options_;
    const ReporterFun &reporter_;

    std::map<std::string, size_t> stat_;
    std::map<std::string, size_t> *statp_;
    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps_ {};
    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp_;
};

int GenerateProgram(panda::pandasm::Program *prog, const util::Options *options, const ReporterFun &reporter)
{
    const auto &generator = ProgramGenerator {prog, options, reporter};
    return generator.GenerateProgram();
}

}  // namespace panda::es2panda::util
