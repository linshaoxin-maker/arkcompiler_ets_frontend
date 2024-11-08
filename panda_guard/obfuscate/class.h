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

#ifndef PANDA_GUARD_OBFUSCATE_CLASS_H
#define PANDA_GUARD_OBFUSCATE_CLASS_H

#include "entity.h"
#include "module_record.h"
#include "method.h"

namespace panda::guard {
    class Class final : public Entity, public IExtractNames {
    public:
        Class(Program *program, const std::string &constructorIdx)
                : Entity(program), constructor_(program, constructorIdx)
        {
        }

        void Build() override;

        void WriteNameCache(const std::string &filePath) override;

        [[nodiscard]] size_t GetMethodCnt() const;

        [[nodiscard]] size_t GetPropertyCnt() const;

        /**
         * 遍历所有方法指令
         * @param callback 指令回调
         */
        void ForEachMethodIns(const std::function<InsTraver> &callback);

        /**
         * For Each Function In Class
         * 1. Class.constructor
         * 2. Class.methods(LiteralArray)
         * 3. Class.outerMethods(defined by definemethod)
         */
        void ForEachFunction(const std::function<FunctionTraver> &callback);

        void ExtractNames(std::set<std::string> &strings) const override;

    protected:
        void Update() override;

        void WriteFileCache(const std::string &filePath) override;

        void WritePropertyCache() override;

    private:
        void CreateMethods(const pandasm::LiteralArray &literalArray);

        void CreateMethod(const pandasm::LiteralArray &literalArray, size_t index, bool isStatic);

    public:
        ModuleRecord *moduleRecord_ = nullptr;
        Function constructor_;
        std::string literalArrayIdx_;
        std::vector<Method> methods_{}; // defineclasswithbuffer 指令关联literalArray中的方法
        std::vector<OuterMethod> outerMethods_{}; // definemethod 指令定义的外部方法
        bool callRunTimeInst_ = false;
    };
}

#endif // PANDA_GUARD_OBFUSCATE_CLASS_H