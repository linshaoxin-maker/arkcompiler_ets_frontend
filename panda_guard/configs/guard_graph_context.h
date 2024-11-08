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

#ifndef PANDA_GUARD_CONFIGS_GUARD_GRAPH_CONTEXT_H
#define PANDA_GUARD_CONFIGS_GUARD_GRAPH_CONTEXT_H

#include "libpandafile/file.h"
#include "obfuscate/function.h"

namespace panda::guard {
    class GraphContext {
    public:
        explicit GraphContext(const panda_file::File &file): file_(file)
        {
        };

        void Init();

        static void Finalize();

        [[nodiscard]] const panda_file::File &GetAbcFile() const;

        /**
         * 获取函数对应的 MethodPtr(函数在abc文件中的偏移值)
         */
        void FillMethodPtr(Function &func) const;

    private:
        [[nodiscard]] std::string GetStringById(const panda_file::File::EntityId &entity_id) const;

    private:
        const panda_file::File &file_;
        std::map<std::string, uint32_t> recordNameMap_;
    };
}

#endif // PANDA_GUARD_CONFIGS_GUARD_GRAPH_CONTEXT_H
