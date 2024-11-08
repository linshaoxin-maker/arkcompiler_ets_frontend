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

#include "json_util.h"

panda::JsonObject *panda::guard::JsonUtil::GetJsonObject(const JsonObject *obj, const std::string &key)
{
    auto value = obj->GetValue<JsonObject::JsonObjPointer>(key);
    if (!value) {
        return nullptr;
    }
    return value->get();
}

std::string panda::guard::JsonUtil::GetStringValue(const JsonObject *obj, const std::string &key)
{
    auto value = obj->GetValue<JsonObject::StringT>(key);
    return (value != nullptr) ? *value : "";
}

double panda::guard::JsonUtil::GetDoubleValue(const JsonObject *obj, const std::string &key)
{
    auto value = obj->GetValue<JsonObject::NumT>(key);
    return (value != nullptr) ? *value : 0;
}

bool panda::guard::JsonUtil::GetBoolValue(const JsonObject *obj, const std::string &key)
{
    auto value = obj->GetValue<JsonObject::BoolT>(key);
    return (value != nullptr) && *value;
}

std::vector<std::string> panda::guard::JsonUtil::GetArrayStringValue(const JsonObject *obj, const std::string &key)
{
    std::vector<std::string> res;
    auto arrValues = obj->GetValue<JsonObject::ArrayT>(key);
    if (!arrValues) {
        return res;
    }
    res.reserve(arrValues->size());
    for (auto &value: *arrValues) {
        res.emplace_back(*(value.Get<JsonObject::StringT>()));
    }
    return res;
}

std::map<std::string, std::string> panda::guard::JsonUtil::GetMapStringValue(
    const JsonObject *obj, const std::string &key)
{
    std::map<std::string, std::string> res;
    auto mapObj = GetJsonObject(obj, key);
    if (!mapObj) {
        return res;
    }
    for (size_t idx = 0; idx < mapObj->GetSize(); idx++) {
        auto mapKey = mapObj->GetKeyByIndex(idx);
        auto mapValue = GetStringValue(mapObj, mapKey);
        res.emplace(mapKey, mapValue);
    }
    return res;
}
