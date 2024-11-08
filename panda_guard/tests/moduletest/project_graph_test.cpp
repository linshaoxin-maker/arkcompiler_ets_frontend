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

#include "project_guard_test.h"
#include "obfuscate/graph_analyzer.h"

using namespace testing::ext;

namespace {
    constexpr std::string_view TAG = "[Test]";
}

class ProjectGraphTest : public panda::guard::ProjectGuardTest {
public:
    [[nodiscard]] std::string GetProjectName() const override
    {
        return "graph";
    }
};

HWTEST_F(ProjectGraphTest, should_success_when_use_graph, TestSize.Level4)
{
    EXPECT_NE(this->guardProgram_->node_table_.size(), 0);

    panda::guard::Node &file = this->guardProgram_->node_table_.at("main");
    auto it = std::find_if(
            file.functionTable_.begin(), file.functionTable_.end(),
            [](const std::pair<std::string, panda::guard::Function> &pair) -> bool {
                return pair.second.name_ == "graphTest";
            });
    EXPECT_NE(it, file.functionTable_.end());

    int numDynamicImport = 0;
    int numLdObjByValue = 0;
    int numStObjByValue = 0;
    it->second.ForEachIns(
        [&](const panda::guard::InstructionInfo &instInfo) {
            panda::pandasm::Ins *ins = instInfo.ins_;
            switch (ins->opcode) {
                case panda::pandasm::Opcode::DYNAMICIMPORT: {
                    numDynamicImport++;
                    panda::guard::InstructionInfo outInfo;
                    panda::guard::GraphAnalyzer::GetLdaStr(instInfo, outInfo);
                    LOG(INFO, PANDAGUARD) << TAG << "find DYNAMICIMPORT related to "
                                          << outInfo.ins_->ToString();
                    EXPECT_EQ(outInfo.ins_->ids[0], "./common/dyn");
                    break;
                }
                case panda::pandasm::Opcode::LDOBJBYVALUE: {
                    numLdObjByValue++;
                    panda::guard::InstructionInfo outInfo;
                    panda::guard::GraphAnalyzer::GetLdaStr(instInfo, outInfo);
                    LOG(INFO, PANDAGUARD) << TAG << "find LDOBJBYVALUE related to " << outInfo.ins_->ToString();
                    EXPECT_EQ(outInfo.ins_->ids[0], "memA");
                    break;
                }
                case panda::pandasm::Opcode::STOBJBYVALUE: {
                    numStObjByValue++;
                    panda::guard::InstructionInfo outInfo;
                    panda::guard::GraphAnalyzer::GetLdaStr(instInfo, outInfo);
                    if (outInfo.IsValid()) {
                        LOG(INFO, PANDAGUARD) << TAG << "find STOBJBYVALUE related to "
                                              << outInfo.ins_->ToString();
                        EXPECT_EQ(outInfo.ins_->ids[0], "memC");
                    } else {
                        panda::guard::GraphAnalyzer::GetLdaStr(instInfo, outInfo, 2);
                        LOG(INFO, PANDAGUARD) << TAG << "find STOBJBYVALUE(acc type) related to "
                                              << outInfo.ins_->ToString();
                        EXPECT_EQ(outInfo.ins_->ids[0], "accVal");
                    }
                    break;
                }
                default:
                    break;
            }
        });
    EXPECT_EQ(numDynamicImport, 1);
    EXPECT_EQ(numLdObjByValue, 1);
    EXPECT_EQ(numStObjByValue, 2);
}

HWTEST_F(ProjectGraphTest, should_success_when_use_graph_in_class, TestSize.Level4)
{
    EXPECT_NE(this->guardProgram_->node_table_.size(), 0);

    panda::guard::Node &file = this->guardProgram_->node_table_.at("common/dyn");
    auto it = std::find_if(
            file.functionTable_.begin(), file.functionTable_.end(),
            [](const std::pair<std::string, panda::guard::Function> &pair) -> bool {
                return pair.second.rawName_ == "func_main_0";
            });
    EXPECT_NE(it, file.functionTable_.end());

    int numDefineGetSet = 0;
    it->second.ForEachIns(
        [&](const panda::guard::InstructionInfo &instInfo) {
            panda::pandasm::Ins *ins = instInfo.ins_;
            switch (ins->opcode) {
                case panda::pandasm::Opcode::DEFINEGETTERSETTERBYVALUE: {
                    numDefineGetSet++;
                    panda::guard::InstructionInfo outInfo;
                    panda::guard::GraphAnalyzer::GetLdaStr(instInfo, outInfo);
                    LOG(INFO, PANDAGUARD) << TAG << "find DEFINEGETTERSETTERBYVALUE related to "
                                          << outInfo.ins_->ToString();
                    EXPECT_EQ(outInfo.ins_->ids[0], "memA");
                    break;
                }
                default:
                    break;
            }
        });
    EXPECT_EQ(numDefineGetSet, 1);
}