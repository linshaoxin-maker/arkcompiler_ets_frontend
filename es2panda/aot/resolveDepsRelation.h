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

#ifndef ES2PANDA_AOT_DEPEND_RELATION_H
#define ES2PANDA_AOT_DEPEND_RELATION_H
#include <aot/options.h>
#include <es2panda.h>
#include <string>
#include <queue>
#include <util/workerQueue.h>
#include <map>
#include <set>
#include <vector>

namespace panda::es2panda::aot {

class DepsRelationResolver {
public:
    explicit DepsRelationResolver(const std::map<std::string, panda::es2panda::util::ProgramCache *> &progsInfo,
                                  const std::unique_ptr<panda::es2panda::aot::Options> &options,
                                  std::map<std::string, std::set<std::string>> &resolveDepsRelation,
                                  std::set<std::string> &generatedRecords)
        : progsInfo_(progsInfo), resolveDepsRelation_(resolveDepsRelation),
        compileContextInfo_(options->CompilerOptions().compileContextInfo), generatedRecords_(generatedRecords)
    {
    }

    ~DepsRelationResolver() = default;
    void CollectStaticImportDepsRelation(const panda::pandasm::Program &program);
    void CollectDynamicImportDepsRelation(const panda::pandasm::Program &program, const std::string &recordName);
    bool Resolve();

private:
    void FillRecord2ProgramMap(std::unordered_map<std::string, std::string> &record2ProgramMap);
    bool AddValidRecord(const std::string &recordName,
                        const std::unordered_map<std::string, std::string> &record2ProgramMap);
    bool CheckShouldCollectDepsLiteralValue(std::string ohmurl);
    void CollectStaticImportDepsRelationWithLiteral(panda::pandasm::LiteralArray::Literal literal);

    const std::map<std::string, panda::es2panda::util::ProgramCache *> &progsInfo_;
    std::map<std::string, std::set<std::string>> &resolveDepsRelation_;
    CompileContextInfo &compileContextInfo_;
    std::queue<std::string> unresolvedDeps_ {};
    std::set<std::string> resolvedDeps_ {};
    std::set<std::string> &generatedRecords_;
};
} // namespace panda::es2panda::aot

#endif

