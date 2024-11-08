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

#include "function.h"

#include "libpandafile/method_data_accessor-inl.h" // inline function, do not delete
#include "bytecode_optimizer/runtime_adapter.h"
#include "mem/arena_allocator.h"
#include "compiler/optimizer/ir_builder/ir_builder.h"
#include "program.h"
#include "utils/logger.h"
#include "util/string_util.h"
#include "util/assert_util.h"
#include "configs/guard_context.h"
#include "graph_analyzer.h"
#include "inst_obf.h"

namespace {
    using OpcodeList = std::vector<panda::pandasm::Opcode>;

    constexpr std::string_view TAG = "[Function]";
    constexpr std::string_view LINE_DELIMITER = ":";
    constexpr std::string_view RECORD_DELIMITER = ".";
    constexpr std::string_view SCOPE_DELIMITER = "#";
    constexpr size_t SCOPE_AND_NAME_LEN = 2;
    constexpr size_t MAX_LINE_NUMBER = 100000;
    constexpr std::string_view ENTRY_FUNC_TAG = ".func_main_0";
    constexpr std::string_view STATIC_INITIALIZER_TAG = ">#static_initializer";
    constexpr std::string_view INSTANCE_INITIALIZER_TAG = ">#instance_initializer";
    constexpr std::string_view CONSOLE_INS_VAR = "console";
    constexpr std::string_view ANONYMOUS_FUNCTION_NAME = "^0"; // first anonymous function name

    const std::map<char, panda::guard::FunctionType> FUNCTION_TYPE_MAP = {
            {'>', panda::guard::FunctionType::INSTANCE_FUNCTION},
            {'<', panda::guard::FunctionType::STATIC_FUNCTION},
            {'=', panda::guard::FunctionType::CONSTRUCTOR_FUNCTION},
            {'*', panda::guard::FunctionType::NORMAL_FUNCTION},
            {'%', panda::guard::FunctionType::ENUM_FUNCTION},
            {'&', panda::guard::FunctionType::NAMESPACE_FUNCTION},
    };
    const OpcodeList PROPERTY_TYPE_LIST_NORMAL = {
            panda::pandasm::Opcode::STOBJBYNAME,
            panda::pandasm::Opcode::DEFINEPROPERTYBYNAME,
            panda::pandasm::Opcode::STOBJBYVALUE,
    };

    /**
     * 是否为入口函数(func_main_0)
     * @param functionIdx 函数Idx
     * @return 是否为入口函数
     */
    bool IsEntryMethod(const std::string &functionIdx)
    {
        return functionIdx.find(ENTRY_FUNC_TAG) != std::string::npos;
    }

    /**
     * 是否为隐式函数（字节码中动态生成，不在源码中体现）
     * @param functionIdx 函数Idx
     * @return 是否为隐式函数
     */
    bool IsImplicitMethod(const std::string &functionIdx)
    {
        return functionIdx.find(INSTANCE_INITIALIZER_TAG) != std::string::npos ||
               functionIdx.find(STATIC_INITIALIZER_TAG) != std::string::npos;
    }

    /**
     * 在白名单的函数不进行名称混淆（内部指令和关联属性需进行混淆）
     * @param functionIdx 函数Idx
     * @return 是否在白名单内
     */
    bool IsWhiteListFunction(const std::string &functionIdx)
    {
        return IsEntryMethod(functionIdx) || IsImplicitMethod(functionIdx);
    }

    bool GetConsoleLogInfoForStart(const std::vector<panda::pandasm::Ins> &insList, size_t &start, uint16_t &reg)
    {
        size_t i = 0;
        while (i < insList.size()) {
            auto &ins = insList[i];
            if (ins.opcode == panda::pandasm::Opcode::TRYLDGLOBALBYNAME &&
                ins.ids[0] == CONSOLE_INS_VAR) {
                size_t nextInsIndex = i + 1;
                PANDA_GUARD_ASSERT_PRINT(
                        nextInsIndex >= insList.size(),
                        TAG << "get console next ins get bad index:" << nextInsIndex);

                auto &nextIns = insList[nextInsIndex];
                PANDA_GUARD_ASSERT_PRINT(
                        nextIns.opcode != panda::pandasm::Opcode::STA,
                        TAG << "get console next ins get bad ins type");

                reg = nextIns.regs[0];
                start = i;
                return true;
            }
            i++;
        }

        return false;
    }

    bool HasMovInstForRange(const std::vector<panda::pandasm::Ins> &insList, size_t start, size_t end, uint16_t oriReg,
                            uint16_t rangeReg)
    {
        size_t i = end - 1;
        while (i > start) {
            auto &ins = insList[i];
            if ((ins.opcode == panda::pandasm::Opcode::MOV) && (ins.regs[0] == rangeReg) &&
                (ins.regs[1] == oriReg)) {
                return true;
            }
            i--;
        }
        return false;
    }

    int GetConsoleLogInfoForEnd(
            const std::vector<panda::pandasm::Ins> &insList, size_t start, uint16_t reg, size_t &end)
    {
        size_t i = start + 1;
        while (i < insList.size()) {
            auto &ins = insList[i];
            if ((ins.opcode == panda::pandasm::Opcode::CALLTHIS1
                 || ins.opcode == panda::pandasm::Opcode::CALLTHIS2
                 || ins.opcode == panda::pandasm::Opcode::CALLTHIS3) && ins.regs[0] == reg) {
                end = i + 1;
                return true;
            }

            if ((ins.opcode == panda::pandasm::Opcode::CALLTHISRANGE) &&
                HasMovInstForRange(insList, start, i, reg, ins.regs[0])) {
                end = i + 1;
                return true;
            }

            i++;
        }

        return false;
    }

    /**
     * 获取指令列表中的console.xxx相关指令信息 [start, end)
     * @param insList 指令列表
     * @param start console指令开始索引
     * @param end console指令结束索引（不包括）
     * @return result code
     */
    bool GetConsoleLogInfo(const std::vector<panda::pandasm::Ins> &insList, size_t &start, size_t &end)
    {
        size_t startIndex = 0;
        uint16_t reg = 0; // console variable stored reg
        size_t endIndex = 0;
        if (!GetConsoleLogInfoForStart(insList, startIndex, reg)) {
            return false;
        }

        if (!GetConsoleLogInfoForEnd(insList, startIndex, reg, endIndex)) {
            return false;
        }

        start = startIndex;
        end = endIndex;

        return true;
    }

    void ReplaceJumpInsLabel(
            std::vector<panda::pandasm::Ins> &insList, const std::string &oldLabel, const std::string &newLabel)
    {
        size_t i = 0;
        while (i < insList.size()) {
            auto &ins = insList[i];
            if (!ins.ids.empty() && ins.ids[0] == oldLabel) { // TODO 识别为跳转指令才修改，防止修改用户字符串
                ins.ids[0] = newLabel;
                LOG(INFO, PANDAGUARD) << TAG << "replace label at index:" << i << " " << oldLabel << "-->"
                                      << newLabel;
            }
            i++;
        }
    }
}

void panda::guard::Function::Init()
{
    LOG(INFO, PANDAGUARD) << TAG << "Function:" << this->idx_;
    this->obfIdx_ = this->idx_;
    const auto &[recordName, rawName] = StringUtil::RSplitOnce(this->idx_, RECORD_DELIMITER.data());
    PANDA_GUARD_ASSERT_PRINT(recordName.empty() || rawName.empty(), TAG << "split record and name get bad value");

    this->recordName_ = recordName;
    this->rawName_ = rawName;

    this->InitBaseInfo();

    LOG(INFO, PANDAGUARD) << TAG << "idx:" << this->idx_;
    LOG(INFO, PANDAGUARD) << TAG << "recordName:" << this->recordName_;
    LOG(INFO, PANDAGUARD) << TAG << "rawName:" << this->rawName_;
    LOG(INFO, PANDAGUARD) << TAG << "regsNum:" << this->regsNum;
    LOG(INFO, PANDAGUARD) << TAG << "startLine:" << this->startLine_;
    LOG(INFO, PANDAGUARD) << TAG << "endLine:" << this->endLine_;

    if (!this->useScope_) {
        return;
    }

    std::vector<std::string> scopeAndName = StringUtil::Split(rawName, SCOPE_DELIMITER.data());
    PANDA_GUARD_ASSERT_PRINT(
            scopeAndName.empty() || scopeAndName.size() > SCOPE_AND_NAME_LEN,
            TAG << "split scope and name get bad len");

    this->scopeTypeStr_ = scopeAndName[0];
    this->SetFunctionType(scopeAndName[0].back());
    this->name_ = scopeAndName.size() > 1 ? scopeAndName[1] : ANONYMOUS_FUNCTION_NAME;

    if (StringUtil::IsAnonymousFunctionName(this->name_)) {
        this->anonymous = true;
    }

    this->obfName_ = this->name_;

    LOG(INFO, PANDAGUARD) << TAG << "scopeTypeStr:" << this->scopeTypeStr_;
    LOG(INFO, PANDAGUARD) << TAG << "type:" << (int) this->type_;
    LOG(INFO, PANDAGUARD) << TAG << "name:" << this->name_;
    LOG(INFO, PANDAGUARD) << TAG << "anonymous:" << (this->anonymous ? "true" : "false");
}

panda::pandasm::Function &panda::guard::Function::GetOriginFunction()
{
    return this->program_->prog_->function_table.at(this->obfIdx_);
}

void panda::guard::Function::InitBaseInfo()
{
    const auto &func = this->GetOriginFunction();
    this->name_ = this->idx_;
    this->obfName_ = this->name_;
    this->regsNum = func.regs_num;

    size_t startLineIndex = 0;
    while (startLineIndex < func.ins.size()) {
        const size_t lineNumber = func.ins[startLineIndex].ins_debug.line_number;
        if (lineNumber < MAX_LINE_NUMBER) {
            this->startLine_ = lineNumber;
            break;
        }
        startLineIndex++;
    }

    size_t endLineIndex = func.ins.size() - 1;
    while (endLineIndex >= startLineIndex) {
        const size_t lineNumber = func.ins[endLineIndex].ins_debug.line_number;
        if (lineNumber < MAX_LINE_NUMBER) {
            this->endLine_ = lineNumber + 1;
            break;
        }
        endLineIndex--;
    }
}

void panda::guard::Function::SetFunctionType(char functionTypeCode)
{
    PANDA_GUARD_ASSERT_PRINT(
            FUNCTION_TYPE_MAP.find(functionTypeCode) == FUNCTION_TYPE_MAP.end(),
            TAG << "unsupported function type code:" << functionTypeCode);

    this->type_ = FUNCTION_TYPE_MAP.at(functionTypeCode);
}

void panda::guard::Function::InitNameCacheScope()
{
    this->SetNameCacheScope(this->name_);
}

void panda::guard::Function::Build()
{
    if (IsEntryMethod(this->idx_)) {
        return;
    }

    LOG(INFO, PANDAGUARD) << TAG << "function build for " << this->idx_ << " start";

    this->InitNameCacheScope();

    this->ForEachIns([&](const InstructionInfo &info) -> void {
        CreateProperty(info);
    });

    LOG(INFO, PANDAGUARD) << TAG << "scope:" << (this->scope_ == TOP_LEVEL ? "TOP_LEVEL" : "Function");
    LOG(INFO, PANDAGUARD) << TAG << "nameCacheScope:" << this->GetNameCacheScope();
    LOG(INFO, PANDAGUARD) << TAG << "export:" << (this->export_ ? "true" : "false");
    LOG(INFO, PANDAGUARD) << TAG << "propertiesSize:" << this->properties_.size();

    LOG(INFO, PANDAGUARD) << TAG << "function build for " << this->idx_ << " end";
}

bool panda::guard::Function::IsWhiteListOrAnonymousFunction(const std::string &functionIdx) const
{
    return IsWhiteListFunction(functionIdx) || this->anonymous;
}

void panda::guard::Function::RefreshNeedUpdate()
{
    this->needUpdate = true;
    this->contentNeedUpdate_ = true;
    this->nameNeedUpdate_ = TopLevelOptionEntity::NeedUpdate(*this) && !IsWhiteListOrAnonymousFunction(this->idx_);
    LOG(INFO, PANDAGUARD) << TAG << "Function nameNeedUpdate: " << (this->nameNeedUpdate_ ? "true" : "false");
}

void panda::guard::Function::ForEachIns(const std::function<InsTraver> &callback)
{
    auto &func = this->GetOriginFunction();
    for (size_t i = 0; i < func.ins.size(); i++) {
        InstructionInfo info(this, &func.ins[i], i);
        callback(info);
    }
}

void panda::guard::Function::UpdateReference()
{
    if (IsImplicitMethod(this->idx_)) {
        LOG(INFO, PANDAGUARD) << TAG << "skip update reference for:" << this->idx_;
        return;
    }

    LOG(INFO, PANDAGUARD) << TAG << "update reference start:" << this->idx_;
    this->ForEachIns([&](InstructionInfo &info) -> void {
        InstObf::UpdateInst(info);
    });
    LOG(INFO, PANDAGUARD) << TAG << "update reference end:" << this->idx_;
}

void panda::guard::Function::RemoveConsoleLog()
{
    auto &insList = this->GetOriginFunction().ins;
    size_t start = 0;
    size_t end = 0;
    while (GetConsoleLogInfo(insList, start, end)) {
        LOG(INFO, PANDAGUARD) << TAG << "remove console log for:" << this->idx_;
        LOG(INFO, PANDAGUARD) << TAG << "found console ins range:[" << start << ", " << end << ")";
        PANDA_GUARD_ASSERT_PRINT(end >= insList.size(), TAG << "bad end ins index for console:" << end);
        if (insList[start].set_label) {
            if (insList[end].set_label) {
                // replace jump ins label
                ReplaceJumpInsLabel(insList, insList[start].label, insList[end].label);
            } else {
                // add label to end ins
                insList[end].set_label = true;
                insList[end].label = insList[start].label;
            }
        }
        insList.erase(insList.begin() + start, insList.begin() + end);
    }
}

void panda::guard::Function::FillInstInfo(size_t index, InstructionInfo &instInfo)
{
    auto &func = this->GetOriginFunction();
    PANDA_GUARD_ASSERT_PRINT(index >= func.ins.size(), TAG << "out of range index: " << index);

    instInfo.index_ = index;
    instInfo.ins_ = &func.ins[index];
    instInfo.function_ = this;
}

void panda::guard::Function::ExtractNames(std::set<std::string> &strings) const
{
    strings.emplace(this->name_);
    for (const auto &property: this->properties_) {
        property.ExtractNames(strings);
    }
}

std::string panda::guard::Function::GetLines() const
{
    return LINE_DELIMITER.data() + std::to_string(this->startLine_) + LINE_DELIMITER.data() + std::to_string(
            this->endLine_);
}

void panda::guard::Function::CreateProperty(const InstructionInfo &info)
{
    if (!IsValidProperty(info)) {
        return;
    }

    InstructionInfo nameInfo;
    GetPropertyNameInfo(info, nameInfo);
    if(!nameInfo.IsValid()) {
        LOG(INFO, PANDAGUARD) << TAG << "invalid nameInfo:" << info.index_ << " " << info.ins_->ToString();
        return;
    }

    std::string name = StringUtil::UnicodeEscape(nameInfo.ins_->ids[0]);
    Property property(this->program_, name);
    property.insInfo_ = info;
    property.nameInfo_ = nameInfo;
    property.scope_ = this->scope_;
    property.export_ = this->export_;

    property.Create();

    LOG(INFO, PANDAGUARD) << TAG << "find property:" << property.name_;

    this->properties_.push_back(property);
}

bool panda::guard::Function::IsValidProperty(const InstructionInfo &info)
{
    bool found = std::any_of(
            PROPERTY_TYPE_LIST_NORMAL.begin(), PROPERTY_TYPE_LIST_NORMAL.end(),
            [&](panda::pandasm::Opcode opcode) -> bool {
                return info.ins_->opcode == opcode;
            });
    return found && !info.IsInnerReg(); // 寄存器小于函数申请寄存器数量则为函数内寄存器, 这种情况下不关联属性
}

void panda::guard::Function::GetPropertyNameInfo(const InstructionInfo &info, InstructionInfo &nameInfo) const
{
    if (info.ins_->opcode != pandasm::Opcode::STOBJBYVALUE) {
        nameInfo = info;
        return;
    }

    // this.['property']
    GraphAnalyzer::GetLdaStr(info, nameInfo);
    if (nameInfo.IsValid() || this->type_ != FunctionType::ENUM_FUNCTION) {
        return;
    }

    // e.g. this[0] = 'property'
    LOG(INFO, PANDAGUARD) << TAG << "try to find property in acc";
    GraphAnalyzer::GetLdaStr(info, nameInfo, 2);
}

void panda::guard::Function::UpdateName(const Node &node)
{
    std::string obfRawName;
    if (node.contentNeedUpdate_ && this->nameNeedUpdate_) {
        this->obfName_ = GuardContext::GetInstance()->GetNameMapping()->GetName(this->name_);
        obfRawName = obfRawName +
                     SCOPE_DELIMITER.data() + this->scopeTypeStr_ + SCOPE_DELIMITER.data() + this->obfName_;

    } else {
        obfRawName = this->rawName_;
    }

    this->obfIdx_ = (node.fileNameNeedUpdate_ ? node.obfName_ : node.name_) +
                    RECORD_DELIMITER.data() + obfRawName;
}

void panda::guard::Function::UpdateDefine() const
{
    if (!this->insInfo_.IsValid()) {
        LOG(INFO, PANDAGUARD) << TAG << "no bind define ins, ignore update define";
        return;
    }

    // definefunc, definemethod for function, defineclasswithbuffer, callruntime.definesendableclass for constructor
    PANDA_GUARD_ASSERT_PRINT(
            this->insInfo_.ins_->opcode != pandasm::Opcode::DEFINEFUNC &&
            this->insInfo_.ins_->opcode != pandasm::Opcode::DEFINECLASSWITHBUFFER &&
            this->insInfo_.ins_->opcode != pandasm::Opcode::CALLRUNTIME_DEFINESENDABLECLASS &&
            this->insInfo_.ins_->opcode != pandasm::Opcode::DEFINEMETHOD,
            TAG << "get bad ins type");

    this->insInfo_.ins_->ids[0] = this->obfIdx_;
    this->program_->prog_->strings.emplace(this->obfIdx_);
}

void panda::guard::Function::UpdateFunctionTable()
{
    if (this->idx_ == this->obfIdx_) {
        return;
    }
    auto entry = this->program_->prog_->function_table.extract(this->idx_);
    entry.key() = this->obfIdx_;
    entry.mapped().name = this->obfIdx_;
    this->program_->prog_->function_table.insert(std::move(entry));
}

void panda::guard::Function::GetGraph(compiler::Graph *&outGraph)
{
    // the existence of graph_ depends on GraphContext
    auto context = GuardContext::GetInstance()->GetGraphContext();
    PANDA_GUARD_ASSERT_PRINT(context == nullptr, TAG << "graph context not inited");

    if (this->graph_ != nullptr) {
        outGraph = this->graph_;
        return;
    }

    context->FillMethodPtr(*this);

    auto method_ptr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(this->methodPtr_);
    this->allocator_ = std::make_shared<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER);
    this->local_allocator_ = std::make_shared<ArenaAllocator>(SpaceType::SPACE_TYPE_COMPILER, nullptr, true);
    this->runtimeInterface_ = std::make_shared<panda::BytecodeOptimizerRuntimeAdapter>(context->GetAbcFile());
    auto graph = this->allocator_->New<compiler::Graph>(this->allocator_.get(), this->local_allocator_.get(),
                                                        Arch::NONE, method_ptr, this->runtimeInterface_.get(),
                                                        false, nullptr, true, true);
    PANDA_GUARD_ASSERT_PRINT((graph == nullptr) || !graph->RunPass<panda::compiler::IrBuilder>(),
                             TAG << "Graph " << this->idx_ << ": IR builder failed!");

    this->BuildPcInsMap(graph);
    this->graph_ = graph;
    LOG(INFO, PANDAGUARD) << TAG << "create graph for " << this->idx_ << " end";

    outGraph = graph;
}

void panda::guard::Function::BuildPcInsMap(const compiler::Graph *graph)
{
    auto method_ptr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(this->methodPtr_);

    size_t funcSize = this->GetOriginFunction().ins.size();
    this->pcInstMap_.reserve(funcSize);

    auto instructions_buf = graph->GetRuntime()->GetMethodCode(method_ptr);
    compiler::BytecodeInstructions instructions(instructions_buf, graph->GetRuntime()->GetMethodCodeSize(method_ptr));
    size_t idx = 0;
    for (auto inst: instructions) {
        this->pcInstMap_.emplace(instructions.GetPc(inst), idx);
        idx++;
        if (idx >= funcSize) {
            break;
        }
    }
}

void panda::guard::Function::Update()
{
    LOG(INFO, PANDAGUARD) << TAG << "function update for " << this->idx_ << " start";

    auto it = this->program_->node_table_.find(this->recordName_);
    PANDA_GUARD_ASSERT_PRINT(it == this->program_->node_table_.end(), TAG << "not find node: " + this->recordName_);

    this->UpdateName(it->second);
    this->UpdateDefine();
    this->UpdateFunctionTable();

    if (it->second.contentNeedUpdate_ && this->contentNeedUpdate_) {
        for (auto &property: this->properties_) {
            property.Obfuscate();
        }
    }

    LOG(INFO, PANDAGUARD) << TAG << "function update for " << this->idx_ << " end";
}

void panda::guard::Function::WriteNameCache(const std::string &filePath)
{
    if (!IsWhiteListOrAnonymousFunction(this->idx_)) {
        this->WriteFileCache(filePath);
        this->WritePropertyCache();
    }

    for (auto &property: this->properties_) {
        property.WriteNameCache(filePath);
    }
}

void panda::guard::Function::WriteFileCache(const std::string &filePath)
{
    if (this->type_ != panda::guard::FunctionType::CONSTRUCTOR_FUNCTION) {
        std::string name = this->GetNameCacheScope();
        if (this->type_ != panda::guard::FunctionType::ENUM_FUNCTION) {
            name += this->GetLines();
        }
        GuardContext::GetInstance()->GetNameCache()->AddObfIdentifierName(filePath, name, this->obfName_);
    } else {
        std::string name = this->name_ + this->GetLines();
        GuardContext::GetInstance()->GetNameCache()->AddObfMemberMethodName(filePath, name, this->obfName_);
    }
}

void panda::guard::Function::WritePropertyCache()
{
    TopLevelOptionEntity::WritePropertyCache(*this);
}
