/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "mergeProgram.h"
#include "protobufSnapshotGenerator.h"
#include "arena_allocator.h"
#include "Options.h"
#include "assembler/assembly-function.h"
#include "libpandafile/literal_data_accessor.h"
#include <assembly-emitter.h>

#if defined(PANDA_TARGET_WINDOWS)
#include <io.h>
#else
#include <dirent.h>
#endif
#include <mem/pool_manager.h>

namespace panda::proto {

using mem::MemConfig;

class ProtoMemManager {
public:
    explicit ProtoMemManager()
    {
        constexpr auto COMPILER_SIZE = 512_MB;

        MemConfig::Initialize(0, 0, COMPILER_SIZE, 0);
        PoolManager::Initialize(PoolType::MMAP);
    }

    NO_COPY_SEMANTIC(ProtoMemManager);
    NO_MOVE_SEMANTIC(ProtoMemManager);

    ~ProtoMemManager()
    {
        PoolManager::Finalize();
        MemConfig::Finalize();
    }
};

int TraverseProtoBinPath(const std::string &protoBinPath, const std::string &protoBinSuffix, MergeProgram *mergeProgram,
                         panda::ArenaAllocator *allocator)
{
    panda::pandasm::Program program;

#if PANDA_TARGET_WINDOWS
    int handle = 0;
    struct _finddata_t fileInfo;
    std::string path;
    if ((handle = _findfirst(path.assign(protoBinPath).append("\\*").c_str(), &fileInfo)) == -1) {
        return 1;
    }
    do
    {
        if (fileInfo.attrib & _A_SUBDIR) {
            if((!strncmp(fileInfo.name, ".", 1)) || (!strncmp(fileInfo.name, "..", 2))) {
                continue;
            }
            if (TraverseProtoBinPath(path.assign(protoBinPath).append("\\").append(fileInfo.name), protoBinSuffix,
                                     mergeProgram, allocator) != 0) {
                _findclose(handle);
                return 1;
            }
        } else {
            std::string fileName(fileInfo.name);
            if (fileName.substr(fileName.find_last_of(".") + 1).compare(protoBinSuffix) == 0) {
                proto::ProtobufSnapshotGenerator::GenerateProgram(
                    path.assign(protoBinPath).append("\\").append(fileName), program, allocator);
                mergeProgram->Merge(&program);
            }
        }
    } while (_findnext(handle, &fileInfo) == 0);
    _findclose(handle);
#else
    DIR *protoBin = opendir(protoBinPath.c_str());
    if (protoBin == nullptr) {
        return 1;
    }
    dirent *dir = nullptr;
    std::string pathPrefix = protoBinPath + "/";
    while ((dir = readdir(protoBin)) != nullptr) {
        if((!strncmp(dir->d_name, ".", 1)) || (!strncmp(dir->d_name, "..", 2))) {
            continue;
        }
        if (dir->d_type == DT_DIR) {
            std::string subDirName = pathPrefix + dir->d_name;
            if (TraverseProtoBinPath(subDirName, protoBinSuffix, mergeProgram, allocator) != 0) {
                closedir(protoBin);
                return 1;
            }
        } else {
            std::string fileName = pathPrefix + dir->d_name;
            std::string suffixStr = fileName.substr(fileName.find_last_of(".") + 1);
            if (suffixStr.compare(protoBinSuffix) == 0) {
                proto::ProtobufSnapshotGenerator::GenerateProgram(fileName, program, allocator);
                mergeProgram->Merge(&program);
            }
        }
    }
    closedir(protoBin);
#endif
    return 0;
}

void MergeProgram::Merge(panda::pandasm::Program *src) {
    CorrectLiteraArrayId(src);

    bool hasTypeAnnoRecord = false;
    for (auto &iter : src->record_table) {
        auto &name = iter.first;
        bool isTypeAnnoRecord = name == std::string(TYPE_ANNOTATION_RECORD.data());
        if (hasTypeAnnoRecord && isTypeAnnoRecord) {
            continue;
        }
        ASSERT(prog_->record_table.find(name) == prog_->record_table.end());
        prog_->record_table.insert(std::move(iter));
        hasTypeAnnoRecord = hasTypeAnnoRecord || isTypeAnnoRecord;
    }

    for (auto &[name, func] : src->function_table) {
        ASSERT(prog_->function_table.find(name) == prog_->function_table.end());
        prog_->function_table.emplace(name, std::move(func));
    }

    ASSERT(src->function_synonyms.empty());

    const auto base = prog_->literalarray_table.size();
    size_t count = 0;
    for (auto &[id, litArray] : src->literalarray_table) {
        prog_->literalarray_table.emplace(std::to_string(base + count), std::move(litArray));
        count++;
    }

    for (const auto &str : src->strings) {
        prog_->strings.insert(str);
    }

    for (const auto &type: src->array_types) {
        prog_->array_types.insert(type);
    }
}

void MergeProgram::CorrectLiteraArrayId(panda::pandasm::Program *src)
{
    const auto base = prog_->literalarray_table.size();

    for (auto &[name, litArray] : src->literalarray_table) {
        for (auto &lit : litArray.literals_) {
            if (lit.tag_ == panda_file::LiteralTag::TYPEINDEX) {
                lit.value_ = std::get<uint32_t>(lit.value_) + base;
            }
        }
    }

    for (auto &[name, func] : src->function_table) {
        for (auto &insn : func.ins) {
            IncreaseInsLiteralArrayIdByBase(insn, base);
        }
    }

    for (auto &[name, record] : src->record_table) {
        for (auto &field : record.field_list) {
            if (field.type.GetId() != panda_file::Type::TypeId::U32) {
                continue;
            }
            auto addedVal = static_cast<uint32_t>(base) + field.metadata->GetValue().value().GetValue<uint32_t>();
            field.metadata->SetValue(panda::pandasm::ScalarValue::Create<panda::pandasm::Value::Type::U32>(addedVal));
        }
    }
}

// TODO: let it be auto-generated after isa-refactoring
void MergeProgram::IncreaseInsLiteralArrayIdByBase(panda::pandasm::Ins &insn, size_t base)
{
    switch (insn.opcode) {
        case panda::pandasm::Opcode::ECMA_CREATEARRAYWITHBUFFER:
        case panda::pandasm::Opcode::ECMA_CREATEOBJECTWITHBUFFER:
        case panda::pandasm::Opcode::ECMA_CREATEOBJECTHAVINGMETHOD:
        case panda::pandasm::Opcode::ECMA_DEFINECLASSWITHBUFFER:
            insn.imms[0] = std::get<int64_t>(insn.imms[0]) + static_cast<int64_t>(base);
            return;
        case panda::pandasm::Opcode::ECMA_NEWLEXENVWITHNAMEDYN:
            insn.imms[1] = std::get<int64_t>(insn.imms[1]) + static_cast<int64_t>(base);
            return;
        default:
            return;
    }
}

int Run(int argc, const char **argv)
{
    auto options = std::make_unique<Options>();
    if (!options->Parse(argc, argv)) {
        std::cerr << options->ErrorMsg() << std::endl;
        return 1;
    }

    std::string protoBinPath = options->protoBinPath();
    std::string protoBinSuffix = options->protoBinSuffix();

    panda::ArenaAllocator allocator(panda::SpaceType::SPACE_TYPE_COMPILER, nullptr, true);

    panda::pandasm::Program program;
    MergeProgram mergeProgram(&program);
    if (panda::proto::TraverseProtoBinPath(protoBinPath, protoBinSuffix, &mergeProgram, &allocator)) {
        return 1;
    }

    std::map<std::string, size_t> stat;
    std::map<std::string, size_t> *statp = nullptr;
    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps {};
    panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp = nullptr;

    std::string outputPandaFile = options->outputPandaFile();
#ifdef PANDA_TARGET_WINDOWS
    outputPandaFile = protoBinPath + "\\" + outputPandaFile;
#else
    outputPandaFile = protoBinPath + "/" + outputPandaFile;
#endif
    if (!panda::pandasm::AsmEmitter::Emit(outputPandaFile, *(mergeProgram.GetResult()), statp, mapsp, true)) {
        return 1;
    }

    return 0;
}
}

int main(int argc, const char **argv)
{
    panda::proto::ProtoMemManager mm;
    return panda::proto::Run(argc, argv);
}
