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

#include "debugInfoLookup.h"
#include "disassembler/disassembler.h"

#include "varbinder/declaration.h"
#include "varbinder/variable.h"

#include "checker/ETSchecker.h"
#include "varbinder/recordTable.h"
#include "varbinder/ETSBinder.h"

#include <filesystem>

namespace ark::es2panda {

DebugInfoLookup::DebugInfoLookup(std::string_view pathToPfs, checker::ETSChecker *checker)
    : checker_(checker), allocator_(checker->Allocator()), classDeclCreator_(checker_, allocator_)
{
    for (auto &entry : std::filesystem::directory_iterator(pathToPfs)) {
        ExtractInfoFromFile(entry.path().string());
    }
}

void DebugInfoLookup::ExtractInfoFromFile(std::string_view abcFilePath)
{
    auto pf = panda_file::OpenPandaFile(abcFilePath);

    FillClassRecords(*pf);
    FillMethodDebugInfo(*pf);

    pandaFiles_.push_back(std::move(pf));
}

static std::string GetFullRecordName(const panda_file::File &pf, const panda_file::File::EntityId &classId)
{
    std::string name = reinterpret_cast<const char *>(pf.GetStringData(classId).data);

    auto type = pandasm::Type::FromDescriptor(name);
    type = pandasm::Type(type.GetComponentName(), type.GetRank());

    return type.GetPandasmName();
}

void DebugInfoLookup::FillClassRecords(const panda_file::File &pf)
{
    auto classes = pf.GetClasses();

    for (size_t i = 0; i < classes.Size(); i++) {
        panda_file::File::EntityId classId(classes[i]);
        recordNameSet_.insert(GetFullRecordName(pf, classId));

        if (pf.IsExternal(classId)) {
            // can't construct ClassDataAccessor for external classes
            continue;
        }

        panda_file::ClassDataAccessor cda(pf, classId);
        std::cout << "FillClassRecords: " << cda.DemangledName() << std::endl;
        classRecordInfo_.insert({cda.DemangledName(), std::move(cda)});
    }
}

void DebugInfoLookup::FillMethodDebugInfo(const panda_file::File &pf)
{
    auto debugInfoExtractor = panda_file::DebugInfoExtractor(&pf);
    auto methodIdList = debugInfoExtractor.GetMethodIdList();

    for (auto &methodId : methodIdList) {
        MethodDebugInfo methodDebugInfo = {debugInfoExtractor.GetSourceFile(methodId),
                                           debugInfoExtractor.GetSourceCode(methodId),
                                           methodId,
                                           debugInfoExtractor.GetLineNumberTable(methodId),
                                           debugInfoExtractor.GetLocalVariableTable(methodId),
                                           debugInfoExtractor.GetParameterInfo(methodId),
                                           debugInfoExtractor.GetColumnNumberTable(methodId)};
        methodDebugInfo_.push_back(methodDebugInfo);
    }
}

void DebugInfoLookup::LookupByName(const util::StringView &identName)
{
    std::string identNameStr = std::string(identName);
    std::cout << "LookupByName = " << identNameStr << std::endl;
    if (auto record = recordNameSet_.find(identNameStr); record != recordNameSet_.end()) {
        auto it = classRecordInfo_.find(identNameStr);
        if (it != classRecordInfo_.end()) {
            classDeclCreator_.CreateClassDeclaration(identName, &(it->second));
        }
    }
}

}  // namespace ark::es2panda
