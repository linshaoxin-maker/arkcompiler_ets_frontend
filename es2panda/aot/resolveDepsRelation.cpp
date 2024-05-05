/*
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

#include "resolveDepsRelation.h"

namespace panda::es2panda::aot {

void DepsRelationResolver::FillRecord2ProgramMap(std::unordered_map<std::string, std::string> &record2ProgramMap)
{
    for (const auto &progInfo : progsInfo_) {
        if (progInfo.first.find("npmEntries.txt") != std::string::npos) {
            for (const auto &record : progInfo.second->program.record_table) {
                if (record.second.field_list.empty()) {
                    generatedRecords_.insert(record.second.name);
                    continue;
                }
                resolveDepsRelation_[progInfo.first].insert(record.second.name);
            }
            continue;
        }
        for (const auto &record : progInfo.second->program.record_table) {
            if (record.second.field_list.empty()) {
                generatedRecords_.insert(record.second.name);
                continue;
            }
            record2ProgramMap[record.second.name] = progInfo.first;
        }
    }
}

bool DepsRelationResolver::AddValidRecord(const std::string &recordName,
                                          const std::unordered_map<std::string, std::string> &record2ProgramMap)
{
    const auto progkeyItr = record2ProgramMap.find(recordName);
    if (progkeyItr == record2ProgramMap.end()) {
        std::cerr << "Failed to find record: " << recordName << std::endl;
        return false;
    }

    const auto progItr = progsInfo_.find(progkeyItr->second);
    if (progItr == progsInfo_.end()) {
        std::cerr << "Failed to find program for file: " << progkeyItr->second << std::endl;
        return false;
    }

    resolveDepsRelation_[progkeyItr->second].insert(recordName);
    return true;
}

std::vector<std::string> SplitNormalizedOhmurl(const std::string &ohmurl) {
    // format of ohmurl: "@normalized:N&<moduleName>&<bundleName>&normalizedImport&<version>"
    static constexpr char NORMALIZED_OHMURL_SEPARATOR = '&';
    std::string normalizedImport {};
    std::string pkgName {};
    std::vector<std::string> items;

    size_t start = 0;
    size_t pos = ohmurl.find(NORMALIZED_OHMURL_SEPARATOR);
    while (pos != std::string::npos) {
        std::string item = ohmurl.substr(start, pos - start);
        items.emplace_back(item);
        start = pos + 1;
        pos = ohmurl.find(NORMALIZED_OHMURL_SEPARATOR, start);
    }
    std::string tail = ohmurl.substr(start);
    items.emplace_back(tail);

    return items;
}

std::string GetPkgNameFromNormalizedOhmurl(const std::string &ohmurl) {
    static constexpr char SLASH_TAG = '/';
    static constexpr size_t NORMALIZED_IMPORT_POS = 3U;

    std::string normalizedImport {};
    std::string pkgName {};
    auto items = SplitNormalizedOhmurl(ohmurl);

    normalizedImport = items[NORMALIZED_IMPORT_POS];
    size_t pos = normalizedImport.find(SLASH_TAG);
    if (pos != std::string::npos) {
        pkgName = normalizedImport.substr(0, pos);
    }
    if (normalizedImport[0] == '@') {
        pos = normalizedImport.find(SLASH_TAG, pos + 1);
        if (pos != std::string::npos) {
            pkgName = normalizedImport.substr(0, pos);
        }
    }
    return pkgName;
}

std::string GetRecordNameFromNormalizedOhmurl(const std::string &ohmurl) {
    // format of recordName: "<bundleName>&normalizedImport&<version>"
    static constexpr size_t BUNDLE_NAME_POS = 2U;
    static constexpr size_t NORMALIZED_IMPORT_POS = 3U;
    static constexpr size_t VERSION_POS = 4U;
    std::string recordName {};
    auto items = SplitNormalizedOhmurl(ohmurl);

    recordName += items[BUNDLE_NAME_POS] + '&' + items[NORMALIZED_IMPORT_POS] + '&' + items[VERSION_POS];
    return recordName;
}

bool IsHspPackage(const std::string &ohmurl, const std::set<std::string> &hspPkgNames)
{
    auto pkgName = GetPkgNameFromNormalizedOhmurl(ohmurl);
    if (std::find(hspPkgNames.begin(), hspPkgNames.end(), pkgName) != hspPkgNames.end()) {
        return true;
    }
    return false;
}
bool DepsRelationResolver::CheckShouldCollectDepsLiteralValue(std::string ohmurl)
{
    if (ohmurl.find("@normalized:N") != std::string::npos && !IsHspPackage(ohmurl, compileContextInfo_.externalPkgNames)) {
        return true;
    }
    return false;
}

bool DepsRelationResolver::Resolve()
{
    std::unordered_map<std::string, std::string> record2ProgramMap{};
    FillRecord2ProgramMap(record2ProgramMap);

    for (auto &entryRecord : compileContextInfo_.compileEntries) {
        if (!AddValidRecord(entryRecord, record2ProgramMap)) {
            return false;
        }
        unresolvedDeps_.push(entryRecord);
        resolvedDeps_.insert(entryRecord);

        while (!unresolvedDeps_.empty()) {
            auto depRecord = unresolvedDeps_.front();
            unresolvedDeps_.pop();
            const auto progkeyItr = record2ProgramMap.find(depRecord);
            if (progkeyItr == record2ProgramMap.end()) {
                std::cerr << "Failed to find compile record: " << depRecord << std::endl;
                return false;
            }
            const auto progItr = progsInfo_.find(progkeyItr->second);
            if (progItr == progsInfo_.end()) {
                std::cerr << "Failed to find program for file: " << progkeyItr->second << std::endl;
                return false;
            }
            resolveDepsRelation_[progkeyItr->second].insert(depRecord);

            CollectStaticImportDepsRelation(progItr->second->program);
            CollectDynamicImportDepsRelation(progItr->second->program, depRecord);
        }
    }
    return true;
}

void DepsRelationResolver::CollectStaticImportDepsRelationWithLiteral(panda::pandasm::LiteralArray::Literal literal)
{
    std::visit([this](auto&& element) {
        if constexpr (std::is_same_v<std::decay_t<decltype(element)>, std::string>) {
            if (this->CheckShouldCollectDepsLiteralValue(element)) {
                auto collectRecord = GetRecordNameFromNormalizedOhmurl(element);
                if (!this->resolvedDeps_.count(collectRecord)) {
                    this->unresolvedDeps_.push(collectRecord);
                    this->resolvedDeps_.insert(collectRecord);
                }
            }
        }
    }, literal.value_);
}

void DepsRelationResolver::CollectStaticImportDepsRelation(const panda::pandasm::Program &program)
{
    auto &recordTable = program.record_table;
    std::string literal_array_key;
    for (auto &pair : recordTable) {
        for (auto &field : pair.second.field_list) {
            if (field.name == "moduleRecordIdx") {
                literal_array_key = field.metadata->GetValue().value().GetValue<std::string>();
                goto find_literal_array_key;
            }
        }
    }
    find_literal_array_key:
    auto itr = program.literalarray_table.find(literal_array_key);
    for (auto& literal : itr->second.literals_) {
        CollectStaticImportDepsRelationWithLiteral(literal);
    }
}

void DepsRelationResolver::CollectDynamicImportDepsRelation(const panda::pandasm::Program &program, const std::string &recordName)
{
    for (const auto &func: program.function_table) {
        size_t regs_num = func.second.regs_num;
        if (func.second.name.find(recordName) == std::string::npos) {
            continue;
        }
        for (uint32_t i = 0; i < func.second.ins.size(); i++) {
            const auto inst = func.second.ins[i];
            if (inst.opcode == pandasm::Opcode::DYNAMICIMPORT) {
                std::string dynamicImportOhmurl;
                for (uint32_t j = i; j >= 0; j--) {
                    if (func.second.ins[j].opcode == pandasm::Opcode::LDA_STR) {
                        dynamicImportOhmurl = func.second.ins[j].ToString("", true, regs_num);
                        // std::string dynamicImportOhmurl1 = func.second.ins[j].OperandsToString(true, regs_num);
                        break;
                    }
                }
                // skipping variable dynamicImport
                if (dynamicImportOhmurl.find("@normalized:") == std::string::npos) {
                    continue;
                }
                // skipping HSP package
                if (IsHspPackage(dynamicImportOhmurl, compileContextInfo_.externalPkgNames)) {
                    continue;
                }
                auto dynamicImportRecord = GetRecordNameFromNormalizedOhmurl(dynamicImportOhmurl);
                if (!resolvedDeps_.count(dynamicImportRecord)) {
                    unresolvedDeps_.push(dynamicImportRecord);
                    resolvedDeps_.insert(dynamicImportRecord);
                }
            }
        }
    }
}

}
