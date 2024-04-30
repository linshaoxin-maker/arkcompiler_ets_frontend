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

std::string DepsRelationResolver::RecordNameForliteralKey(std::string literalKey)
{
    size_t pos = literalKey.rfind('_');
    if (pos != std::string::npos) {
        return literalKey.substr(0, pos);
    } else {
        std::cerr << "The literalKey format is error!" << std::endl;
    }
    return literalKey;
}


bool DepsRelationResolver::CheckIsHspOHMUrl(std::string ohmUrl)
{
    size_t prev = 0;
    constexpr int pos = 3;
    for (int i = 0; i < pos; i++) {
        prev = ohmUrl.find('&', prev) + 1;
    }
    size_t behindPos = ohmUrl.find('&', prev);
    std::string normalizedPath =  ohmUrl.substr(prev, behindPos);
    
    size_t slashPos = normalizedPath.find('/', 0);
    if (normalizedPath[0] == '@') {
        slashPos = normalizedPath.find('/', slashPos + 1);
    }
    std::string pkgName =  normalizedPath.substr(0, slashPos);
    auto it = std::find(compileContextInfo_.hspPkgNames.begin(), compileContextInfo_.hspPkgNames.end(), pkgName);
    return it != compileContextInfo_.hspPkgNames.end();
}

bool DepsRelationResolver::CheckShouldCollectDepsLiteralValue(std::string literalValue)
{
    const std::string normalizedPrefix = "@normalized:N";
    if (literalValue.substr(0, normalizedPrefix.length()) == normalizedPrefix &&
        !CheckIsHspOHMUrl(literalValue)) {
        return true;
    }
    return false;
}

std::string DepsRelationResolver::TransformRecordName(std::string ohmUrl)
{
    size_t prev = 0;
    constexpr int pos = 2;
    for (int i = 0; i < pos; i++) {
        prev = ohmUrl.find('&', prev) + 1;
    }
    return ohmUrl.substr(prev, ohmUrl.length());
}

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

bool IsHspPackage(const std::string &ohmurl, const std::vector<std::string> &hspPkgNames)
{
    auto pkgName = GetPkgNameFromNormalizedOhmurl(ohmurl);
    if (std::find(hspPkgNames.begin(), hspPkgNames.end(), pkgName) != hspPkgNames.end()) {
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

            CollectStaticImportDepsRelation(progItr->second->program, depRecord);
            CollectDynamicImportDepsRelation(progItr->second->program, depRecord);
        }
    }
    return true;
}

void DepsRelationResolver::CollectStaticImportDepsRelation(const panda::pandasm::Program &program, const std::string &recordName)
{
    for (auto &literalarrayPair : program.literalarray_table) {
        std::cout << "literalarrayKey: " << literalarrayPair.first << std::endl;
        std::string literalKeyRecord = RecordNameForliteralKey(literalarrayPair.first);
        if (literalKeyRecord != recordName) {
            continue;
        }
        for (auto& literal : literalarrayPair.second.literals_) {
            std::visit([this](auto&& element) {
                // std::cout<< "literalValue: " << element <<std::endl;
                if constexpr (std::is_same_v<std::decay_t<decltype(element)>, std::string>) {
                    std::cout<< "literalValue: " << element <<std::endl;
                    if (this->CheckShouldCollectDepsLiteralValue(element)) {
                        auto collectRecord = this->TransformRecordName(element);
                        std::cout << "collectRecord: " << collectRecord << std::endl;
                        if (!resolvedDeps_.count(collectRecord)) {
                            unresolvedDeps_.push(collectRecord);
                            resolvedDeps_.insert(collectRecord);
                        }
                    }
                }
            }, literal.value_);
        }
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
                if (IsHspPackage(dynamicImportOhmurl, compileContextInfo_.hspPkgNames)) {
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
