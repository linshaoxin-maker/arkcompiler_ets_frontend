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

#include "guard_graph_context.h"

#include "mem/pool_manager.h"
#include "libpandafile/file-inl.h"
#include "libpandafile/class_data_accessor.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/method_data_accessor.h"
#include "util/assert_util.h"

namespace {
    const std::string TAG = "[Guard_Graph]";
}

void panda::guard::GraphContext::Init()
{
    PoolManager::Initialize(PoolType::MALLOC);

    auto records = file_.GetClasses();
    for (auto id: records) {
        panda_file::File::EntityId record_id{id};
        if (file_.IsExternal(record_id)) {
            continue;
        }
        std::string name = GetStringById(record_id);
        pandasm::Type type = pandasm::Type::FromDescriptor(name);
        std::string record_full_name = type.GetName();
        recordNameMap_.emplace(record_full_name, id);
    }

    LOG(INFO, PANDAGUARD) << TAG << "[graph context init success]";
}

void panda::guard::GraphContext::Finalize()
{
    PoolManager::Finalize();
    LOG(INFO, PANDAGUARD) << TAG << "[graph context finalize success]";
}

const panda::panda_file::File &panda::guard::GraphContext::GetAbcFile() const
{
    return file_;
}

void panda::guard::GraphContext::FillMethodPtr(Function &func) const
{
    auto it = recordNameMap_.find(func.recordName_);
    PANDA_GUARD_ASSERT_PRINT(it == recordNameMap_.end(), TAG << "can not find record: " << func.recordName_);

    uint32_t id = it->second;
    panda_file::File::EntityId record_id{id};
    panda_file::ClassDataAccessor cda{this->GetAbcFile(), record_id};
    cda.EnumerateMethods([this, &func](const panda_file::MethodDataAccessor &mda) {
        if (mda.IsExternal()) {
            return;
        }
        std::string method_name_raw = this->GetStringById(mda.GetNameId());
        if (method_name_raw == func.rawName_) {
            func.methodPtr_ = mda.GetMethodId().GetOffset();
        }
    });

    PANDA_GUARD_ASSERT_PRINT(func.methodPtr_ == 0, TAG << "can not find method ptr for: " << func.idx_);
}

std::string panda::guard::GraphContext::GetStringById(const panda_file::File::EntityId &entity_id) const
{
    panda_file::File::StringData sd = file_.GetStringData(entity_id);
    return (reinterpret_cast<const char *>(sd.data));
}