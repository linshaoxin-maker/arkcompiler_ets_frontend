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

#ifndef PANDA_GUARD_OBFUSCATE_FUNCTION_H
#define PANDA_GUARD_OBFUSCATE_FUNCTION_H

#include <functional>
#include "compiler/optimizer/ir/graph.h"
#include "mem/arena_allocator.h"
#include "entity.h"
#include "property.h"

namespace panda::guard {
    class Node;

    using InsTraver = void(InstructionInfo &functionInfo);

    enum class FunctionType {
        NONE,
        INSTANCE_FUNCTION, // '>'
        STATIC_FUNCTION, // '<'
        CONSTRUCTOR_FUNCTION, // '='
        NORMAL_FUNCTION, // '*'
        ENUM_FUNCTION, // '%'
        NAMESPACE_FUNCTION, // '&'
    };

    class Function : public Entity, public IExtractNames {
    public:
        Function(Program *program, std::string idx, bool useScope = true)
            : Entity(program), idx_(std::move(idx)), useScope_(useScope)
        {
            this->obfIdx_ = this->idx_; // obfIdx default equal to idx
        }

        /**
         * Function Init
         * 1. split idx get record, scope, name
         * 2. InitBaseInfo from origin Function
         */
        void Init();

        void Build() override;

        void WriteNameCache(const std::string &filePath) override;

        void ExtractNames(std::set<std::string> &strings) const override;

        /**
         * 遍历函数所有指令
         * @param callback 指令回调
         */
        void ForEachIns(const std::function<InsTraver> &callback);

        /**
         * Update all reference instruction
         */
        void UpdateReference();

        /**
         * 删除ConsoleLog日志，需在完成其他指令更新后调用（删除日志会改变指令相对顺序）
         */
        void RemoveConsoleLog();

        /**
         * 填充为指定序号的指令
         * @param index 指令序号
         * @param instInfo 待填充指令
         */
        void FillInstInfo(size_t index, InstructionInfo &instInfo);

        /**
         * 获取函数对应的Graph结构, 用于关联指令计算
         */
        void GetGraph(compiler::Graph *&outGraph);

    protected:
        void RefreshNeedUpdate() override;

        void Update() override;

        void WriteFileCache(const std::string &filePath) override;

        void WritePropertyCache() override;

        /**
         * 修改函数定义，默认为定义函数处指令（definefunc）
         */
        virtual void UpdateDefine() const;

        virtual void InitNameCacheScope();

        [[nodiscard]] std::string GetLines() const;

        virtual bool IsWhiteListOrAnonymousFunction(const std::string &functionIdx) const;

    private:
        pandasm::Function &GetOriginFunction();

        void InitBaseInfo();

        void SetFunctionType(char functionTypeCode);

        void CreateProperty(const InstructionInfo &info);

        static bool IsPropertyIns(const InstructionInfo &info);

        void GetPropertyNameInfo(const InstructionInfo &info, InstructionInfo &nameInfo) const;

        void UpdateName(const Node &node);

        void UpdateFunctionTable(Node &node);

        void BuildPcInsMap(const compiler::Graph *graph);

    public:
        std::string idx_;
        std::string obfIdx_;
        std::string recordName_;
        std::string rawName_;
        std::string scopeTypeStr_;
        FunctionType type_ = FunctionType::NONE;
        size_t regsNum = 0; // 函数内申请的寄存器数量
        size_t startLine_ = 0;
        size_t endLine_ = 0;
        uint32_t methodPtr_ = 0;
        std::vector<Property> properties_{}; // own property, 构造函数关联的属性
        std::vector<Property> variableProperties_{}; // 函数中遍历或参数中存在的属性
        std::unordered_map<size_t, size_t> pcInstMap_{}; // 指令PC 与 指令索引 的映射表
        bool useScope_ = true;
        bool nameNeedUpdate_ = true; // 函数名称 需要更新
        bool contentNeedUpdate_ = true; // 函数内容(属性等) 需要更新

    private:
        bool anonymous = false; // is anonymous function
        compiler::Graph *graph_ = nullptr;
        std::shared_ptr<ArenaAllocator> allocator_ = nullptr;
        std::shared_ptr<ArenaAllocator> local_allocator_ = nullptr;
        std::shared_ptr<compiler::RuntimeInterface> runtimeInterface_ = nullptr;
    };
}

#endif // PANDA_GUARD_OBFUSCATE_FUNCTION_H
