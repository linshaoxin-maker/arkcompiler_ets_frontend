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

#include "graph_analyzer.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "function.h"
#include "util/assert_util.h"

using namespace panda::guard;

namespace {
    constexpr std::string_view TAG = "[Graph]";

    using IntrinsicId = panda::compiler::RuntimeInterface::IntrinsicId;
    using InstIdFilterList = std::vector<IntrinsicId>;
    struct InstParam {
    public:
        unsigned index; // 目标指令 在输入指令寄存器列表中的索引
        InstIdFilterList filterList; // 输入指令 类型列表
    };

    const InstIdFilterList INST_ID_LIST_DYNAMICIMPORT = {
            IntrinsicId::DYNAMICIMPORT,
    };
    const InstIdFilterList INST_ID_LIST_LDOBJBYVALUE = {
            IntrinsicId::LDOBJBYVALUE_IMM8_V8,
            IntrinsicId::LDOBJBYVALUE_IMM16_V8,
    };
    const InstIdFilterList INST_ID_LIST_STOBJBYVALUE = {
            IntrinsicId::STOBJBYVALUE_IMM8_V8_V8,
            IntrinsicId::STOBJBYVALUE_IMM16_V8_V8,
    };
    const InstIdFilterList INST_ID_LIST_DEFINEGETTERSETTERBYVALUE = {
            IntrinsicId::DEFINEGETTERSETTERBYVALUE_V8_V8_V8_V8,
    };
    const InstIdFilterList INST_ID_LIST_ISIN = {
            IntrinsicId::ISIN_IMM8_V8,
    };
    const InstIdFilterList INST_ID_LIST_LDSUPERBYVALUE = {
            IntrinsicId::LDSUPERBYVALUE_IMM16_V8,
            IntrinsicId::LDSUPERBYVALUE_IMM8_V8,
    };
    const InstIdFilterList INST_ID_LIST_STSUPERBYVALUE = {
            IntrinsicId::STSUPERBYVALUE_IMM16_V8_V8,
            IntrinsicId::STSUPERBYVALUE_IMM8_V8_V8,
    };
    const InstIdFilterList INST_ID_LIST_STOWNBYVALUEWITHNAMESET = {
            IntrinsicId::STOWNBYVALUEWITHNAMESET_IMM16_V8_V8,
            IntrinsicId::STOWNBYVALUEWITHNAMESET_IMM8_V8_V8,
    };
    const InstIdFilterList INST_ID_LIST_DEFINEMETHOD = {
            IntrinsicId::DEFINEMETHOD_IMM16_ID16_IMM8,
            IntrinsicId::DEFINEMETHOD_IMM8_ID16_IMM8,
    };
    const InstIdFilterList INST_ID_LIST_CREATEOBJECTWITHBUFFER = {
            IntrinsicId::CREATEOBJECTWITHBUFFER_IMM16_ID16,
            IntrinsicId::CREATEOBJECTWITHBUFFER_IMM8_ID16,
    };
    const InstIdFilterList INST_ID_LIST_DEFINECLASSWITHBUFFER = {
            IntrinsicId::DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8,
            IntrinsicId::DEFINECLASSWITHBUFFER_IMM8_ID16_ID16_IMM16_V8,
    };
    const InstIdFilterList INST_ID_LIST_LDOBJBYNAME = {
            IntrinsicId::LDOBJBYNAME_IMM16_ID16,
            IntrinsicId::LDOBJBYNAME_IMM8_ID16,
    };
    const InstIdFilterList INST_ID_LIST_CALLRUNTIME_TOPROPERTYKEY = {
            IntrinsicId::CALLRUNTIME_TOPROPERTYKEY_PREF_NONE,
    };

    const std::map<panda::pandasm::Opcode, InstParam> LDA_STA_INST_PARAM_MAP = {
            {
                    panda::pandasm::Opcode::DYNAMICIMPORT,
                    {0, INST_ID_LIST_DYNAMICIMPORT}
            },
            {
                    panda::pandasm::Opcode::LDOBJBYVALUE,
                    {1, INST_ID_LIST_LDOBJBYVALUE}
            },
            {
                    panda::pandasm::Opcode::STOBJBYVALUE,
                    {1, INST_ID_LIST_STOBJBYVALUE}
            },
            {
                    panda::pandasm::Opcode::DEFINEGETTERSETTERBYVALUE,
                    {1, INST_ID_LIST_DEFINEGETTERSETTERBYVALUE}
            },
            {
                    panda::pandasm::Opcode::ISIN,
                    {0, INST_ID_LIST_ISIN}
            },
            {
                    panda::pandasm::Opcode::LDSUPERBYVALUE,
                    {1, INST_ID_LIST_LDSUPERBYVALUE}
            },
            {
                    panda::pandasm::Opcode::STSUPERBYVALUE,
                    {1, INST_ID_LIST_STSUPERBYVALUE}
            },
            {
                    panda::pandasm::Opcode::STOWNBYVALUEWITHNAMESET,
                    {1, INST_ID_LIST_STOWNBYVALUEWITHNAMESET}
            }
    };

    const std::vector<InstIdFilterList> DEFINE_INST_PASS_THROUGH_LIST = {
            INST_ID_LIST_DEFINEGETTERSETTERBYVALUE,
            INST_ID_LIST_DEFINEMETHOD,
    };
    const std::vector<InstIdFilterList> DEFINE_INST_ID_LIST = {
            INST_ID_LIST_DEFINECLASSWITHBUFFER,
            INST_ID_LIST_CREATEOBJECTWITHBUFFER,
            INST_ID_LIST_LDOBJBYNAME,
    };

    /**
     * 指令类型是否在列表中
     */
    bool IsInstIdMatched(const panda::compiler::Inst *inst, const InstIdFilterList &list)
    {
        if (!inst->IsIntrinsic()) {
            return false;
        }

        IntrinsicId instId = inst->CastToIntrinsic()->GetIntrinsicId();
        return std::any_of(
                list.begin(), list.end(),
                [&instId](IntrinsicId filter) -> bool {
                    return instId == filter;
                });
    }
    bool IsInstIdMatched(const panda::compiler::Inst *inst, const std::vector<InstIdFilterList> &list)
    {
        return std::any_of(
                list.begin(), list.end(),
                [&](const InstIdFilterList &filter) -> bool {
                    return IsInstIdMatched(inst, filter);
                });
    }

    /**
     * 当前指令是否匹配要求: 同 InstructionInfo 一致
     * @param inst 待检查的Graph指令
     * @param list 应匹配的指令类型
     * @param targetIns 应匹配的InstructionInfo
     */
    bool IsInstMatched(const panda::compiler::Inst *inst, const InstructionInfo &targetIns,
                       const InstIdFilterList &list)
    {
        if (!IsInstIdMatched(inst, list)) {
            return false;
        }

        uint32_t pcVal = inst->GetPc();
        auto &pcInstMap = targetIns.function_->pcInstMap_;
        if (pcInstMap.find(pcVal) == pcInstMap.end() ||
            pcInstMap.at(pcVal) != targetIns.index_) {
            return false;
        }

        return true;
    }

    /**
     * 遍历Graph, 找出匹配的指令: InstructionInfo 转换为 Graph指令
     * @param inIns 需找到的指令
     * @param list 指令类型
     * @param outInst graph中的对应指令
     */
    void VisitGraph(const InstructionInfo &inIns, const InstIdFilterList &list,
                    const panda::compiler::Inst *&outInst)
    {
        Function *func = inIns.function_;
        panda::compiler::Graph *graph;
        func->GetGraph(graph);

        for (const panda::compiler::BasicBlock *bb: graph->GetBlocksLinearOrder()) {
            for (const panda::compiler::Inst *inst: bb->AllInsts()) {
                if (!IsInstMatched(inst, inIns, list)) {
                    continue;
                }
                outInst = inst;
                return;
            }
        }

        PANDA_GUARD_ABORT_PRINT(TAG << "not find inst: " << inIns.index_ << " in " << inIns.function_->idx_);
    }

    /**
     * 将 graph指令 填充进 InstructionInfo: Graph指令 转换为 InstructionInfo
     * @param inIns 用于获取InstructionInfo背景信息
     * @param target graph指令
     * @param outIns 用于填充的InstructionInfo
     */
    void FillInstInfo(const InstructionInfo &inIns, const panda::compiler::Inst *target, InstructionInfo &outIns)
    {
        uint32_t pcVal = target->GetPc();
        Function *func = inIns.function_;
        PANDA_GUARD_ASSERT_PRINT(
                func->pcInstMap_.find(pcVal) == func->pcInstMap_.end(),
                TAG << "no valid target ins: " << inIns.function_->idx_ << ", " << inIns.index_);
        func->FillInstInfo(func->pcInstMap_.at(pcVal), outIns);
    }

    /**
     * 打印Graph指令内容
     */
    std::string PrintInst(const panda::compiler::Inst *ins)
    {
        std::stringstream stream;
        ins->Dump(&stream, false);
        return stream.str();
    }

    /**
     * 找出输入指令对应的 lda.str指令
     */
    void FindLdaStr(const panda::compiler::Inst *prevInst, const panda::compiler::Inst *&outIns)
    {
        if (IsInstIdMatched(prevInst, INST_ID_LIST_CALLRUNTIME_TOPROPERTYKEY)) {
            prevInst = prevInst->GetInput(0).GetInst();
        }

        if (prevInst->GetOpcode() != panda::compiler::Opcode::CastValueToAnyType) {
            return;
        }
        const panda::compiler::Inst *targetInst = prevInst->GetInput(0).GetInst();
        if (targetInst->GetOpcode() != panda::compiler::Opcode::LoadString) {
            return;
        }

        outIns = targetInst;
    }

    /**
     * 找出输入指令对应的 对象定义指令
     */
    void FindDefineInst(const panda::compiler::Inst *prevInst, const panda::compiler::Inst *&outIns)
    {
        if (IsInstIdMatched(prevInst,
                            {INST_ID_LIST_CREATEOBJECTWITHBUFFER, INST_ID_LIST_DEFINECLASSWITHBUFFER})) {
            outIns = prevInst;
            return;
        }

        // Try to find class define in prototype instance
        if (!IsInstIdMatched(prevInst, INST_ID_LIST_LDOBJBYNAME)) {
            return;
        }

        const panda::compiler::Inst *targetIns = prevInst->GetInput(0).GetInst();
        if (!IsInstIdMatched(targetIns, INST_ID_LIST_DEFINECLASSWITHBUFFER)) {
            return;
        }
        outIns = targetIns;
    }
}

void GraphAnalyzer::GetLdaStr(const InstructionInfo &inIns, InstructionInfo &outIns, int index /* = -1 */)
{
    auto it = LDA_STA_INST_PARAM_MAP.find(inIns.ins_->opcode);
    PANDA_GUARD_ASSERT_PRINT(it == LDA_STA_INST_PARAM_MAP.end(),
                             TAG << "unsupported lda.str scene: " << inIns.ins_->ToString());

    const panda::compiler::Inst *inst;
    VisitGraph(inIns, it->second.filterList, inst);

    unsigned targetIndex = index >= 0 ? index : it->second.index;
    const panda::compiler::Inst *prevInst = inst->GetInput(targetIndex).GetInst();
    const panda::compiler::Inst *targetInst = nullptr;
    FindLdaStr(prevInst, targetInst);

    if (targetInst == nullptr) {
        LOG(INFO, PANDAGUARD) << TAG << "GetLdaStr failed: "
                              << inIns.function_->idx_ << ", " << inIns.index_ << ";" << PrintInst(prevInst);
        return;
    }

    FillInstInfo(inIns, targetInst, outIns);
}

void GraphAnalyzer::HandleDefineMethod(const InstructionInfo &inIns, InstructionInfo &defineIns)
{
    const panda::compiler::Inst *inst;
    VisitGraph(inIns, INST_ID_LIST_DEFINEMETHOD, inst);

    do {
        inst = inst->GetInput(0).GetInst();
    } while (IsInstIdMatched(inst, DEFINE_INST_PASS_THROUGH_LIST));
    PANDA_GUARD_ASSERT_PRINT(!IsInstIdMatched(inst, DEFINE_INST_ID_LIST),
                             TAG << "unexpect defineMethod scene: "
                                     << inIns.function_->idx_ << ", " << inIns.index_ << ";" << PrintInst(inst));

    const panda::compiler::Inst *targetInst = nullptr;
    FindDefineInst(inst, targetInst);
    PANDA_GUARD_ASSERT_PRINT(targetInst == nullptr,
                             TAG << "HandleDefineMethod failed: "
                                     << inIns.function_->idx_ << ", " << inIns.index_ << ";" << PrintInst(inst));

    FillInstInfo(inIns, targetInst, defineIns);
}