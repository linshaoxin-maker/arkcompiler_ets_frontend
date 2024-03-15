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

#ifndef ES2PANDA_COMPILER_DEBUGGER_DEBUG_INFO_LOOKUP_H
#define ES2PANDA_COMPILER_DEBUGGER_DEBUG_INFO_LOOKUP_H

#include "compiler/debugger/evaluate/classDeclarationCreator.h"

#include "libpandafile/file.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/debug_info_extractor.h"
#include "libpandabase/mem/arena_allocator.h"

#include "ir/expressions/identifier.h"

#include <unordered_map>
#include <unordered_set>

namespace ark::es2panda {

namespace checker {
class ETSChecker;
}

class DebugInfoLookup {
public:
    DEFAULT_COPY_SEMANTIC(DebugInfoLookup);
    DEFAULT_MOVE_SEMANTIC(DebugInfoLookup);

    explicit DebugInfoLookup(std::string_view pathToPfs, checker::ETSChecker *checker);
    ~DebugInfoLookup() = default;

    void LookupByName(const util::StringView &identName);

private:
    void ExtractInfoFromFile(std::string_view abcFilePath);
    void FillClassRecords(const panda_file::File &pf);
    void FillMethodDebugInfo(const panda_file::File &pf);

    struct MethodDebugInfo {
        std::string sourceFile;
        std::string sourceCode;
        panda_file::File::EntityId &methodId;
        const panda_file::LineNumberTable &lineNumberTable;
        const panda_file::LocalVariableTable &localVariableTable;
        const std::vector<panda_file::DebugInfoExtractor::ParamInfo> &paramInfo;
        const panda_file::ColumnNumberTable &columnNumberTable;
    };

    checker::ETSChecker *checker_ {nullptr};
    ArenaAllocator *allocator_ {nullptr};

    std::vector<std::unique_ptr<const panda_file::File>> pandaFiles_;

    std::vector<MethodDebugInfo> methodDebugInfo_;
    std::unordered_map<std::string, panda_file::ClassDataAccessor> classRecordInfo_;
    std::unordered_set<std::string> recordNameSet_;

    ClassDeclarationCreator classDeclCreator_;
};

}  // namespace ark::es2panda

#endif  // ES2PANDA_COMPILER_DEBUGGER_DEBUG_INFO_LOOKUP_H
