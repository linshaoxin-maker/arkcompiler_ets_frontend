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

#include "analyseCompileDepsInfo.h"

#include <cstddef>
#include <cstdint>
#include <regex>

namespace panda::es2panda::aot {

static void fillRecord2ProgramMap(const std::map<std::string, panda::es2panda::util::ProgramCache *> &programsInfo,
                                  std::unordered_map<std::string, std::unordered_set<std::string>> &compileDepsInfo,
                                  std::unordered_map<std::string, std::string> &record2ProgramMap,
                                  std::unordered_set<std::string> &generatedRecords)
{
    for (const auto &progInfo : programsInfo) {
        if (progInfo.first.find("npmEntries.txt") != std::string::npos) {
            for (const auto &record : progInfo.second->program.record_table) {
                if (record.second.field_list.empty()) {
                    generatedRecords.insert(record.second.name);
                    continue;
                }
                compileDepsInfo[progInfo.first].insert(record.second.name);
            }
            continue;
        }
        for (const auto &record : progInfo.second->program.record_table) {
            if (record.second.field_list.empty()) {
                generatedRecords.insert(record.second.name);
                continue;
            }
            record2ProgramMap[record.second.name] = progInfo.first;
        }
    }
}

static bool AddValidRecord(const std::string &recordName,
                          const std::map<std::string, panda::es2panda::util::ProgramCache *> &programsInfo,
                          const std::unordered_map<std::string, std::string> &record2ProgramMap,
                          std::unordered_map<std::string, std::unordered_set<std::string>> &compileDepsInfo)
{
    const auto progkeyItr = record2ProgramMap.find(recordName);
    if (progkeyItr == record2ProgramMap.end()) {
        std::cerr << "Failed to find record: " << recordName << std::endl;
        return false;
    }

    const auto progItr = programsInfo.find(progkeyItr->second);
    if (progItr == programsInfo.end()) {
        std::cerr << "Failed to find program for file: " << progkeyItr->second << std::endl;
        return false;
    }

    compileDepsInfo[progkeyItr->second].insert(recordName);
    return true;
}

static std::vector<std::string> SplitNormalizedOhmurl(std::string ohmurl) {
    // format of ohmurl: "@normalized:N&<moduleName>&<bundleName>&normalizedImport&<version>"
    static constexpr char NORMALIZED_OHMURL_SEPARATOR = '&';
    std::string normalizedImport{};
    std::string pkgName{};
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

static std::string GetPkgNameFromNormalizedOhmurl(std::string ohmurl) {
    static constexpr char SLASH_TAG = '/';
    static constexpr size_t NORMALIZED_IMPORT_POS = 3U;

    std::string normalizedImport{};
    std::string pkgName{};
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

static std::string GetRecordNameFromNormalizedOhmurl(std::string ohmurl) {
    // format of recordName: "<bundleName>&normalizedImport&<version>"
    static constexpr size_t BUNDLE_NAME_POS = 2U;
    static constexpr size_t NORMALIZED_IMPORT_POS = 3U;
    static constexpr size_t VERSION_POS = 4U;
    std::string recordName{};
    auto items = SplitNormalizedOhmurl(ohmurl);

    recordName += items[BUNDLE_NAME_POS] + '&' + items[NORMALIZED_IMPORT_POS] + '&' + items[VERSION_POS];
    return recordName;
}

static bool IsHspPackage(std::string ohmurl, const std::vector<std::string> &hspPkgNames)
{
    auto pkgName = GetPkgNameFromNormalizedOhmurl(ohmurl);
    if (std::find(hspPkgNames.begin(), hspPkgNames.end(), pkgName) != hspPkgNames.end()) {
        return true;
    }
    return false;
}

static bool CollectDynamicImportDepsRelation(const std::map<std::string, panda::es2panda::util::ProgramCache *> &programsInfo,
                                             const std::unique_ptr<panda::es2panda::aot::Options> &options,
                                             std::unordered_map<std::string, std::unordered_set<std::string>> &compileDepsInfo,
                                             std::unordered_set<std::string> &generatedRecords)
{
    auto compileEntries = options->CompilerOptions().compileContextInfo.compileEntries;
    auto hspPkgNames = options->CompilerOptions().compileContextInfo.hspPkgNames;

    std::queue<std::string> unresolvedDeps{};
    std::unordered_set<std::string> resolvedDeps{};
    std::unordered_map<std::string, std::string> record2ProgramMap{};

    fillRecord2ProgramMap(programsInfo, compileDepsInfo, record2ProgramMap, generatedRecords);

    for (const std::string &entryRecord : compileEntries) {
        if (!AddValidRecord(entryRecord, programsInfo, record2ProgramMap, compileDepsInfo)) {
            return false;
        }
        unresolvedDeps.push(entryRecord);
        resolvedDeps.insert(entryRecord);

        while (!unresolvedDeps.empty()) {
            auto depRecord = unresolvedDeps.front();
            unresolvedDeps.pop();
            const auto progkeyItr = record2ProgramMap.find(depRecord);
            if (progkeyItr == record2ProgramMap.end()) {
                std::cerr << "Failed to find compile entry record: " << depRecord << std::endl;
                return false;
            }
            const auto progItr = programsInfo.find(progkeyItr->second);
            if (progItr == programsInfo.end()) {
                std::cerr << "Failed to find program for file: " << progkeyItr->second << std::endl;
                return false;
            }

            compileDepsInfo[progkeyItr->second].insert(depRecord);
            for (const auto &func: progItr->second->program.function_table) {
                size_t regs_num = func.second.regs_num;

                if (func.second.name.find(depRecord) == std::string::npos) {
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
                        if (IsHspPackage(dynamicImportOhmurl, hspPkgNames)) {
                            continue;
                        }
                        auto dynamicImportRecord = GetRecordNameFromNormalizedOhmurl(dynamicImportOhmurl);
                        if (!resolvedDeps.count(dynamicImportRecord)) {
                            unresolvedDeps.push(dynamicImportRecord);
                            resolvedDeps.insert(dynamicImportRecord);
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool AnalyseCompileDepsInfo(const std::map<std::string, panda::es2panda::util::ProgramCache *> &programsInfo,
                            const std::unique_ptr<panda::es2panda::aot::Options> &options,
                            std::unordered_map<std::string, std::unordered_set<std::string>> &compileDepsInfo,
                            std::unordered_set<std::string> &generatedRecords)
{
    if (!CollectDynamicImportDepsRelation(programsInfo, options, compileDepsInfo, generatedRecords)) {
        std::cerr << "CollectDynamicImportDepsRelation Failed!" << std::endl;
        return false;
    }
    return true;
}

} // namespace panda::es2panda::aot