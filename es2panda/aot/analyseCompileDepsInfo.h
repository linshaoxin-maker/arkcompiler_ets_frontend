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

#ifndef ES2PANDA_AOT_ANALYSECOMPILEDEPSINFO_H
#define ES2PANDA_AOT_ANALYSECOMPILEDEPSINFO_H
#include <options.h>
#include <util/programCache.h>

namespace panda::es2panda::aot {

bool AnalyseCompileDepsInfo(const std::map<std::string, panda::es2panda::util::ProgramCache *> &programsInfo,
                            const std::unique_ptr<panda::es2panda::aot::Options> &options,
                            std::unordered_map<std::string, std::unordered_set<std::string>> &compileDepsInfo,
                            std::unordered_set<std::string> &generatedRecords);

} // namespace panda::es2panda::aot

#endif