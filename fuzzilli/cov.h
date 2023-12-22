/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef FUZZI_COV_H_
#define FUZZI_COV_H_

// This file is defining functions to handle coverage which are needed for
// fuzzilli fuzzer It communicates coverage bitmap with fuzzilli through shared
// memory

#include <cstdint>
#include <vector>
extern "C" void SanitizerCoverTracePcGuardInit(uint32_t *start, uint32_t *stop);
void SanitizerCoverResetEdgeguards();
uint32_t SanitizerCoverCountDiscoveredEdges();
void CoverInitBuiltinsEdges(uint32_t num_edges);
void CoverUpdateBuiltinsBasicBlockCoverage(const std::vector<bool> &cov_map);

#endif // FUZZI_COV_H_
