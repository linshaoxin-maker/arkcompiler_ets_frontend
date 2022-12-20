/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <assembly-program.h>
#include <assembly-emitter.h>
#include <bytecode_optimizer/optimize_bytecode.h>
#include <es2panda.h>
#include <mem/arena_allocator.h>
#include <mem/pool_manager.h>
#include <options.h>
#include <protobufSnapshotGenerator.h>
#include <util/dumper.h>
#include <util/moduleHelpers.h>
#include <util/programCache.h>

#include <iostream>

namespace panda::es2panda::aot {
using mem::MemConfig;
class MemManager {
public:
    explicit MemManager()
    {
        constexpr auto COMPILER_SIZE = 2048_MB;

        MemConfig::Initialize(0, 0, COMPILER_SIZE, 0);
        PoolManager::Initialize(PoolType::MMAP);
    }

    NO_COPY_SEMANTIC(MemManager);
    NO_MOVE_SEMANTIC(MemManager);

    ~MemManager()
    {
        PoolManager::Finalize();
        MemConfig::Finalize();
    }
};

static void GenerateBase64Output(panda::pandasm::Program *prog)
{
    auto pandaFile = panda::pandasm::AsmEmitter::Emit(*prog, nullptr);
    const uint8_t *buffer = pandaFile->GetBase();
    size_t size = pandaFile->GetPtr().GetSize();
    std::string content(reinterpret_cast<const char*>(buffer), size);
    std::string base64Output = util::Base64Encode(content);
    std::cout << base64Output << std::endl;
}

static bool EmitProgramWithOpt(const std::string &output, panda::pandasm::Program *prog,
    const std::unique_ptr<panda::es2panda::aot::Options> &options)
{
    const bool emitDebuginfo = options->CompilerOptions().isDebug;
    bool isOpt = !emitDebuginfo && options->OptLevel() != 0 && options->ScriptKind() != parser::ScriptKind::COMMONJS;
    std::map<std::string, size_t> stat;
    std::map<std::string, size_t> *statp = options->SizeStat() ? &stat : nullptr;
    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps{};
    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp = isOpt ? &maps : nullptr;

    if (isOpt) {
        if (!panda::pandasm::AsmEmitter::Emit(output, *prog, statp, mapsp, emitDebuginfo)) {
            std::cerr << "Failed to emit unoptimized single abc file: " << output << std::endl;
            return false;
        }
        bytecodeopt::OptimizeBytecode(prog, mapsp, output, true);  // true: has memory pool
    }
    if (!panda::pandasm::AsmEmitter::Emit(output, *prog, statp, mapsp, emitDebuginfo)) {
        std::cerr << "Failed to emit single abc file: " << output << std::endl;
        return false;
    }
    return true;
}

static bool EmitProgramsWithOpt(const std::string &output, const std::vector<panda::pandasm::Program *> &progs,
    const std::unique_ptr<panda::es2panda::aot::Options> &options)
{
    const bool emitDebuginfo = options->CompilerOptions().isDebug;
    bool isOpt = !emitDebuginfo && options->OptLevel() != 0 && options->ScriptKind() != parser::ScriptKind::COMMONJS;
    std::map<std::string, size_t> stat;
    std::map<std::string, size_t> *statp = options->SizeStat() ? &stat : nullptr;
    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps{};
    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp = isOpt ? &maps : nullptr;

    if (isOpt) {
        if (!panda::pandasm::AsmEmitter::EmitPrograms(output, progs, statp, mapsp, emitDebuginfo)) {
            std::cerr << "Failed to emit unoptimized abc file: " << output << std::endl;
            return false;
        }
        bytecodeopt::OptimizeBytecodeWithPrograms(progs, mapsp, output, true);  // true: has memory pool
    }
    if (!panda::pandasm::AsmEmitter::EmitPrograms(output, progs, statp, mapsp, emitDebuginfo)) {
        std::cerr << "Failed to emit abc file: " << output << std::endl;
        return false;
    }
    return true;
}

static bool GenerateMultiProgram(const std::unordered_map<panda::pandasm::Program*, std::string> &programs,
    const std::unique_ptr<panda::es2panda::aot::Options> &options)
{
    if (options->CompilerOptions().mergeAbc) {
        std::vector<panda::pandasm::Program*> progs;
        for (auto &prog: programs) {
            progs.push_back(prog.first);
        }

        auto output = programs.begin()->second;
        return EmitProgramsWithOpt(output, progs, options);
    }
    for (auto &prog: programs) {
        if (!EmitProgramWithOpt(prog.second, prog.first, options)) {
            return false;
        }
    }
    return true;
}

static void TryDumpAsmAndLiteralBuffer(const std::unordered_map<panda::pandasm::Program*, std::string> &programs,
    const std::unique_ptr<panda::es2panda::aot::Options> &options)
{
    const auto &compilerOptions = options->CompilerOptions();
    if (!compilerOptions.dumpAsm && !compilerOptions.dumpLiteralBuffer) {
        return;
    }
    for (auto &prog : programs) {
        if (compilerOptions.dumpAsm) {
            es2panda::Compiler::DumpAsm(prog.first);
        }

        if (compilerOptions.dumpLiteralBuffer) {
            panda::es2panda::util::Dumper::DumpLiterals(prog.first->literalarray_table);
        }
    }
}

static bool GenerateProgram(const std::unordered_map<panda::pandasm::Program*, std::string> &programs,
    const std::unique_ptr<panda::es2panda::aot::Options> &options)
{
    TryDumpAsmAndLiteralBuffer(programs, options);
    if (programs.size() > 1) {
        return GenerateMultiProgram(programs, options);
    }
    auto *prog = programs.begin()->first;
    const auto &output = programs.begin()->second;
    if (output.empty()) {
        GenerateBase64Output(prog);
        return true;
    }
    return EmitProgramWithOpt(output, prog, options);
}

static bool GenerateAbcFiles(const std::map<std::string, panda::es2panda::util::ProgramCache*> &programsInfo,
    const std::unique_ptr<panda::es2panda::aot::Options> &options, size_t expectedProgsCount)
{
    std::unordered_map<panda::pandasm::Program*, std::string> programs;
    for (auto &info : programsInfo) {
        auto outputFileName = options->OutputFiles().empty() ? options->CompilerOutput() :
            options->OutputFiles().at(info.first);
        programs.insert({info.second->program, outputFileName});
    }

    if (programs.size() != expectedProgsCount) {
        std::cerr << "the size of programs is expected to be " << expectedProgsCount
                  << ", but is " << programs.size() << std::endl;
        return false;
    }

    if (!GenerateProgram(programs, options)) {
        std::cerr << "GenerateProgram Failed!" << std::endl;
        return false;
    }

    return true;
}

int Run(int argc, const char **argv)
{
    auto options = std::make_unique<Options>();
    if (!options->Parse(argc, argv)) {
        std::cerr << options->ErrorMsg() << std::endl;
        return 1;
    }

    if (options->CompilerOptions().bcVersion || options->CompilerOptions().bcMinVersion) {
        std::string version = options->CompilerOptions().bcVersion ?
            panda::panda_file::GetVersion(panda::panda_file::version) :
            panda::panda_file::GetVersion(panda::panda_file::minVersion);
        std::cout << version << std::endl;
        return 0;
    }

    std::map<std::string, panda::es2panda::util::ProgramCache*> programsInfo;
    size_t expectedProgsCount = options->CompilerOptions().sourceFiles.size();
    panda::ArenaAllocator allocator(panda::SpaceType::SPACE_TYPE_COMPILER, nullptr, true);
    std::map<std::string, panda::es2panda::util::ProgramCache*> *cachePrograms = nullptr;

    if (!options->CacheFile().empty()) {
        cachePrograms = proto::ProtobufSnapshotGenerator::GetCacheContext(options->CacheFile(),
            options->CompilerOptions().isDebug, &allocator);
    }

    int ret = Compiler::CompileFiles(options->CompilerOptions(), cachePrograms, programsInfo, &allocator);
    if (options->ParseOnly()) {
        return ret;
    }

    if (!options->NpmModuleEntryList().empty()) {
        es2panda::util::ModuleHelpers::CompileNpmModuleEntryList(options->NpmModuleEntryList(), cachePrograms,
            programsInfo, &allocator);
        expectedProgsCount++;
    }

    if (!GenerateAbcFiles(programsInfo, options, expectedProgsCount)) {
        return 1;
    }

    if (!options->CacheFile().empty()) {
        proto::ProtobufSnapshotGenerator::UpdateCacheFile(programsInfo, options->CompilerOptions().isDebug,
            options->CacheFile());
    }

    return 0;
}
}  // namespace panda::es2panda::aot

int main(int argc, const char **argv)
{
    panda::es2panda::aot::MemManager mm;
    return panda::es2panda::aot::Run(argc, argv);
}
