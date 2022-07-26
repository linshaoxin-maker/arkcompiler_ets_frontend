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

#include <assembly-function.h>
#include <assembly-ins.h>
#include <assembly-literals.h>
#include <assembly-program.h>
#include <binder/scope.h>
#include <libpandabase/utils/arena_containers.h>

#include <mutex>

#ifndef ES2PANDA_UTIL_HOTFIX_H
#define ES2PANDA_UTIL_HOTFIX_H

namespace panda::es2panda::compiler {
class PandaGen;
}  // namespace panda::es2panda::compiler

namespace panda::es2panda::util {

class Hotfix {
    struct OriginFunctionInfo {
        std::string funcName;
        std::string funcInternalName;
        std::string funcHash;
        ArenaMap<std::string, std::pair<uint32_t, int>> lexenv;
        ArenaMap<std::string, std::string> classHash;

        OriginFunctionInfo(ArenaAllocator *allocator) : lexenv(allocator->Adapter()),
                                                        classHash(allocator->Adapter()) {}
    };
    using FunctionInfos = ArenaUnorderedMap<std::string, OriginFunctionInfo>;
    using LiteralBuffers = ArenaVector<std::pair<int32_t, std::vector<panda::pandasm::LiteralArray::Literal>>>;

public:
    static Hotfix* GetInstance() {
        if (instance_ == nullptr) {
            instance_ = new Hotfix();
        }
        return instance_;
    }

    bool Initialize(const std::string &mapFile, const std::string &dumpMapFile);
    void Finalize(panda::pandasm::Program **prog);
    uint32_t SetLexicalForPatch(binder::VariableScope *scope, const std::string &variableName);
    bool NeedSetLexicalForPatch(binder::VariableScope *scope);
    uint32_t GetPatchLexicalIdx(const std::string &variableName);
    void ProcessFunction(const compiler::PandaGen *pg, panda::pandasm::Function *func, LiteralBuffers &literalBuffers);
    void ProcessModule(const std::string &recordName, std::vector<panda::pandasm::LiteralArray::Literal> &moduleBuffer);

private:
    std::vector<std::string> GetStringItems(std::string &input, const std::string &delimiter);
    bool ReadMapFile(const std::string &mapFile);
    void WriteMapFile(const std::string &content);
    void DumpFunctionInfo(const compiler::PandaGen *pg, panda::pandasm::Function *func, LiteralBuffers &literalBuffers);
    void MarkFunctionForPatch(const compiler::PandaGen *pg, panda::pandasm::Function *func,
        LiteralBuffers &literalBuffers);
    std::vector<std::pair<std::string, size_t>> GenerateFunctionAndClassHash(panda::pandasm::Function *func,
        LiteralBuffers literalBuffers);
    void DumpModuleInfo(const std::string &recordName, std::vector<panda::pandasm::LiteralArray::Literal> &moduleBuffer);
    void MarkModuleForPatch(const std::string &recordName,
        std::vector<panda::pandasm::LiteralArray::Literal> &moduleBuffer);

    std::string ExpandLiteral(int64_t bufferIdx, LiteralBuffers &literalBuffers);
    std::string ConverLiteralToString(std::vector<panda::pandasm::LiteralArray::Literal> &literalBuffer);
    void CollectFuncDefineIns(panda::pandasm::Function *func);
    void AddInsForNewFunction(panda::pandasm::Program *prog, std::vector<panda::pandasm::Ins> &ins);
    bool IsAnonymousOrDuplicateNameFunction(const std::string &funcName);
    void Destroy();

    Hotfix() : allocator_(SpaceType::SPACE_TYPE_COMPILER, nullptr, true),
               originFunctionInfo_(allocator_.Adapter()),
               originModuleInfo_(allocator_.Adapter()),
               topScopeLexEnvs_(allocator_.Adapter()),
               patchFuncNames_(allocator_.Adapter()),
               newFuncNames_(allocator_.Adapter()),
               funcDefineIns_(allocator_.Adapter()) {}

    static Hotfix *instance_;
    ArenaAllocator allocator_;
    FunctionInfos originFunctionInfo_;
    ArenaUnorderedMap<std::string, std::string> originModuleInfo_;
    std::mutex m_;
    std::string mapFile_;
    std::string dumpMapFile_;
    uint32_t topScopeIdx_;
    ArenaUnorderedMap<std::string, uint32_t> topScopeLexEnvs_;
    ArenaSet<std::string> patchFuncNames_;
    ArenaSet<std::string> newFuncNames_;
    ArenaVector<panda::pandasm::Ins> funcDefineIns_;
    bool patchError_ {false};
};

} // namespace panda::es2panda::util
#endif // ES2PANDA_UTIL_HOTFIX_H
