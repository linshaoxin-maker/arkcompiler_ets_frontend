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

#include "inst_obf.h"
#include "program.h"
#include "graph_analyzer.h"
#include "configs/guard_context.h"

using namespace panda::guard;
namespace {
    using FunctionProcessor = void(InstructionInfo &inst);
    using ProcessorMap = std::map<panda::pandasm::Opcode, const std::function<FunctionProcessor>>;

    constexpr auto PROCESSOR_DEFAULT = [](InstructionInfo &inst) {
        inst.UpdateInsName();
    };
    constexpr auto PROCESSOR_STOBJBYNAME = [](InstructionInfo &inst) {
        if (!GuardContext::GetInstance()->GetGuardOptions()->EnableProperty()) {
            return;
        }
        if (!inst.IsInnerReg()) {
            return;
        }
        inst.UpdateInsName();
        inst.WriteNameCache();
    };
    constexpr auto PROCESSOR_DEFINEPROPERTYBYNAME = [](InstructionInfo &inst) {
        if (!inst.IsInnerReg()) {
            return;
        }
        inst.UpdateInsName();
    };
    constexpr auto PROCESSOR_STSUPERBYNAME = [](InstructionInfo &inst) {
        if (!GuardContext::GetInstance()->GetGuardOptions()->EnableProperty()) {
            return;
        }
        inst.UpdateInsName();
    };

    constexpr auto PROCESSOR_DEFAULT_LDA_STR = [](InstructionInfo &inst) {
        InstructionInfo outInfo;
        GraphAnalyzer::GetLdaStr(inst, outInfo);
        if (!outInfo.IsValid()) {
            return;
        }
        outInfo.UpdateInsName();
    };
    constexpr auto PROCESSOR_STOBJBYVALUE = [](InstructionInfo &inst) {
        if (!GuardContext::GetInstance()->GetGuardOptions()->EnableProperty()) {
            return;
        }
        if (!inst.IsInnerReg()) {
            return;
        }
        InstructionInfo outInfo;
        GraphAnalyzer::GetLdaStr(inst, outInfo);
        if (!outInfo.IsValid()) {
            return;
        }
        outInfo.UpdateInsName();
        outInfo.WriteNameCache();
    };
    constexpr auto PROCESSOR_STSUPERBYVALUE = [](InstructionInfo &inst) {
        if (!GuardContext::GetInstance()->GetGuardOptions()->EnableProperty()) {
            return;
        }
        InstructionInfo outInfo;
        GraphAnalyzer::GetLdaStr(inst, outInfo);
        if (!outInfo.IsValid()) {
            return;
        }
        outInfo.UpdateInsName();
    };

    const ProcessorMap INST_PROCESSOR_MAP = {
            {panda::pandasm::Opcode::LDOBJBYNAME,                   PROCESSOR_DEFAULT},
            {panda::pandasm::Opcode::THROW_UNDEFINEDIFHOLEWITHNAME, PROCESSOR_DEFAULT},
            {panda::pandasm::Opcode::LDSUPERBYNAME,                 PROCESSOR_DEFAULT},
            {panda::pandasm::Opcode::STOBJBYNAME,                   PROCESSOR_STOBJBYNAME},
            {panda::pandasm::Opcode::DEFINEPROPERTYBYNAME,          PROCESSOR_DEFINEPROPERTYBYNAME},
            {panda::pandasm::Opcode::STSUPERBYNAME,                 PROCESSOR_STSUPERBYNAME},
            {panda::pandasm::Opcode::LDOBJBYVALUE,                  PROCESSOR_DEFAULT_LDA_STR},
            {panda::pandasm::Opcode::STOBJBYVALUE,                  PROCESSOR_STOBJBYVALUE},
            {panda::pandasm::Opcode::DEFINEGETTERSETTERBYVALUE,     PROCESSOR_DEFAULT_LDA_STR},
            {panda::pandasm::Opcode::ISIN,                          PROCESSOR_DEFAULT_LDA_STR},
            {panda::pandasm::Opcode::LDSUPERBYVALUE,                PROCESSOR_DEFAULT_LDA_STR},
            {panda::pandasm::Opcode::STSUPERBYVALUE,                PROCESSOR_STSUPERBYVALUE},
            {panda::pandasm::Opcode::STOWNBYNAME,                   PROCESSOR_DEFAULT},
            {panda::pandasm::Opcode::STOWNBYVALUEWITHNAMESET,       PROCESSOR_DEFAULT_LDA_STR},
    };
}

void InstObf::UpdateInst(InstructionInfo &inst)
{
    auto it = INST_PROCESSOR_MAP.find(inst.ins_->opcode);
    if (it == INST_PROCESSOR_MAP.end()) {
        return;
    }
    it->second(inst);
}