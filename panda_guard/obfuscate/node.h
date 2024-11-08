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

#ifndef PANDA_GUARD_OBFUSCATE_NODE_H
#define PANDA_GUARD_OBFUSCATE_NODE_H

#include "entity.h"
#include "function.h"
#include "class.h"
#include "object.h"
#include "module_record.h"

namespace panda::guard {
    struct FilePath {
        std::string prePart;
        std::string name;
        std::string obfName;
        std::string postPart;
    };

    class Node final : public Entity {
    public:
        Node(Program *program, const std::string &name, std::string pkgName)
                : Entity(program, name), moduleRecord_(program, name), pkgName_(std::move(pkgName))
        {}

        void Build() override;

        /**
         * For Each Function In Node
         * 1. Functions
         * 2. For Each Function In Classes
         */
        void ForEachFunction(const std::function<FunctionTraver> &callback);

        /**
         * Update file name references in this node
         */
        void UpdateFileNameReferences();

    protected:
        void RefreshNeedUpdate() override;

        void Update() override;

    private:
        void CreateFunction(const InstructionInfo &info, Scope scope);

        void CreateClass(const InstructionInfo &info, Scope scope);

        void CreateOuterMethod(const InstructionInfo &info);

        void CreateOuterMethodWithIns(const InstructionInfo &methodInsInfo, const InstructionInfo &defineInsInfo);

        void CreateObject(const InstructionInfo &info, Scope scope);

        void CreateObjectOuterProperty(const InstructionInfo &info);

        void CreateFilePath();

        void CreateFilePathForDefaultMode();

        void CreateFilePathForNormalizedMode();

        void ExtractNames();

        void WriteFileCache(const std::string &filePath) override;

        void UpdateRecordTable();

        void UpdateFileNameDefine();

    public:
        ModuleRecord moduleRecord_;
        FilePath filepath_;
        std::unordered_map<std::string, Function> functionTable_{}; // key: Function idx
        std::unordered_map<std::string, Class> classTable_{}; // key: class constructor scope without type
        std::unordered_map<std::string, Object> objectTable_{};
        std::set<std::string> outerProperties_{};
        std::set<std::string> strings_{};
        std::string pkgName_;
        bool fileNameNeedUpdate_ = true;
        bool contentNeedUpdate_ = true;
        bool isNormalizedOhmUrl_ = false;
    };
}

#endif // PANDA_GUARD_OBFUSCATE_NODE_H
