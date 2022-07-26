/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "hotfix.h"
#include <binder/binder.h>
#include <binder/scope.h>
#include <binder/variable.h>
#include <compiler/core/pandagen.h>
#include <ir/expressions/literal.h>

#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>

namespace panda::es2panda::util {

Hotfix* Hotfix::instance_ = nullptr;

const std::string ANONYMOUS_DUPLICATE_SPECIFIER = "#";
const std::string SEPERATOR = "&&";
const std::string TYPE_SEPERATOR = "|";
const std::string FUNC_MAIN = "func_main_0";
const std::string PATCH_MAIN_NEW = "patch_main_0";
const std::string PATCH_MAIN_MODIFIED = "patch_main_1";
const size_t MAP_FUNCTION_ITEM_NUMBER = 3;
const size_t MAP_MODULE_ITEM_NUMBER = 1;
const uint32_t PATCH_ENV_VREG = 0;  // reuse first param vreg

bool Hotfix::Initialize(const std::string &mapFile, const std::string &dumpMapFile)
{
    if (!mapFile.empty() && !ReadMapFile(mapFile)) {
        std::cerr << "Failed to read map file: " << mapFile << ". Stop generating patch" << std::endl;
        return false;
    }
    mapFile_ = mapFile;

    if (!dumpMapFile.empty()) {
        std::fstream fs;
        fs.open(dumpMapFile, std::ios_base::out | std::ios_base::trunc);
        if (!fs.is_open()) {
            std::cerr << "Failed to create output map file: " << dumpMapFile << std::endl;
            return false;
        }
        fs.close();
    }

    dumpMapFile_ = dumpMapFile;
    return true;
}

void Hotfix::Destroy()
{
    if (instance_ != nullptr) {
        delete instance_;
        instance_ = nullptr;
    }
}

void Hotfix::ProcessFunction(const compiler::PandaGen *pg, panda::pandasm::Function *func,
    LiteralBuffers &literalBuffers)
{
    if (!dumpMapFile_.empty()) {
        DumpFunctionInfo(pg, func, literalBuffers);
        return;
    }

    if (!mapFile_.empty()) {
        MarkFunctionForPatch(pg, func, literalBuffers);
        return;
    }
}

void Hotfix::ProcessModule(const std::string &recordName,
    std::vector<panda::pandasm::LiteralArray::Literal> &moduleBuffer)
{
    if (!dumpMapFile_.empty()) {
        DumpModuleInfo(recordName, moduleBuffer);
        return;
    }

    if (!mapFile_.empty()) {
        MarkModuleForPatch(recordName, moduleBuffer);
        return;
    }
}

void Hotfix::DumpModuleInfo(const std::string &recordName,
    std::vector<panda::pandasm::LiteralArray::Literal> &moduleBuffer)
{
    std::stringstream ss;
    ss << recordName << SEPERATOR;
    auto hash = std::hash<std::string>{}(ConverLiteralToString(moduleBuffer));
    ss << hash << std::endl;
    WriteMapFile(ss.str());
}

void Hotfix::MarkModuleForPatch(const std::string &recordName,
    std::vector<panda::pandasm::LiteralArray::Literal> &moduleBuffer)
{
    auto it = originModuleInfo_.find(recordName);
    if (it == originModuleInfo_.end()) {
        std::cerr << "Found new import/export expression, not supported!" << std::endl;
        patchError_ = true;
        return;
    }

    // auto hash = GetHash32String(reinterpret_cast<const uint8_t *>(ConverLiteralToString(moduleBuffer).c_str()));
    auto hash = std::hash<std::string>{}(ConverLiteralToString(moduleBuffer));
    if (std::to_string(hash) != it->second) {
        std::cerr << "Found import/export expression changed in " << recordName << ", not supported!" << std::endl;
        patchError_ = true;
        return;
    }
}

bool Hotfix::IsAnonymousOrDuplicateNameFunction(const std::string &funcName)
{
    return funcName.find(ANONYMOUS_DUPLICATE_SPECIFIER) != std::string::npos;
}

std::vector<std::string> Hotfix::GetStringItems(std::string &input, const std::string &delimiter)
{
    std::vector<std::string> items;
    size_t pos = 0;
    std::string token;
    while ((pos = input.find(delimiter)) != std::string::npos) {
        token = input.substr(0, pos);
        if (!token.empty()) {
            items.push_back(token);
        }
        input.erase(0, pos + delimiter.length());
    }
    if (!input.empty()) {
        items.push_back(input);
    }
    return items;
}

std::vector<std::pair<std::string, size_t>> Hotfix::GenerateFunctionAndClassHash(panda::pandasm::Function *func,
    LiteralBuffers literalBuffers)
{
    std::stringstream ss;
    std::vector<std::pair<std::string, size_t>> hashList;

    ss << ".function any " << func->name << '(';

    for (uint32_t i = 0; i < func->GetParamsNum(); i++) {
        ss << "any a" << std::to_string(i);
        if (i != func->GetParamsNum() - 1) {
            ss << ", ";
        }
    }
    ss << ") {" << std::endl;

    for (const auto &ins : func->ins) {
        ss << (ins.set_label ? "" : "\t") << ins.ToString("", true, func->GetTotalRegs()) << " ";
        if (ins.opcode == panda::pandasm::Opcode::ECMA_CREATEARRAYWITHBUFFER ||
            ins.opcode == panda::pandasm::Opcode::ECMA_CREATEOBJECTWITHBUFFER) {
            int64_t bufferIdx = std::get<int64_t>(ins.imms[0]);
            ss << ExpandLiteral(bufferIdx, literalBuffers) << " ";
        } else if (ins.opcode == panda::pandasm::Opcode::ECMA_DEFINECLASSWITHBUFFER) {
            int64_t bufferIdx = std::get<int64_t>(ins.imms[1]);
            std::string literalStr = ExpandLiteral(bufferIdx, literalBuffers);
            auto classHash = std::hash<std::string>{}(literalStr);
            hashList.push_back(std::pair<std::string, size_t>(ins.ids[0], classHash));
        }
        ss << " ";
    }

    ss << "}" << std::endl;

    for (const auto &ct : func->catch_blocks) {
        ss << ".catchall " << ct.try_begin_label << ", " << ct.try_end_label << ", " << ct.catch_begin_label
            << std::endl;
    }

    std::string functionStr = ss.str();
    std::string funcName = func->name;
    // auto funcHash = GetHash32String(reinterpret_cast<const uint8_t *>(functionStr.c_str()));
    auto funcHash = std::hash<std::string>{}(functionStr);
    hashList.push_back(std::pair<std::string, size_t>(funcName, funcHash));
    return hashList;
}

std::string Hotfix::ConverLiteralToString(std::vector<panda::pandasm::LiteralArray::Literal> &literalBuffer)
{
    std::stringstream ss;
    int count = 0;
    for (auto literal : literalBuffer) {
        ss << "{" << "index: " << count++ << " ";
        ss << "tag: " << static_cast<std::underlying_type<panda::es2panda::ir::LiteralTag>::type>(literal.tag_);
        ss << " ";
        std::string val;
        std::visit([&val](auto&& element) {
            val += "val: ";
            val += element;
            val += " ";
        }, literal.value_ );
        ss << val;
        ss << "},";

    }

    return ss.str();
}

std::string Hotfix::ExpandLiteral(int64_t bufferIdx, Hotfix::LiteralBuffers &literalBuffers)
{
    for (auto &litPair : literalBuffers) {
        if (litPair.first == bufferIdx) {
            return ConverLiteralToString(litPair.second);
        }
    }

    return "";
}

bool Hotfix::NeedSetLexicalForPatch(binder::VariableScope *scope)
{
    if (mapFile_.empty()) {
        return false;
    }

    if (!scope->IsFunctionVariableScope()) {
        return false;
    }

    auto funcName = scope->AsFunctionVariableScope()->InternalName();
    if (std::string(funcName) != FUNC_MAIN) {
        return false;
    }
    return true;
}

uint32_t Hotfix::SetLexicalForPatch(binder::VariableScope *scope, const std::string &variableName)
{
    if (originFunctionInfo_.count(FUNC_MAIN) && originFunctionInfo_.at(FUNC_MAIN).lexenv.count(variableName)) {
        return originFunctionInfo_.at(FUNC_MAIN).lexenv[variableName].first;
    }

    if (!topScopeLexEnvs_.count(variableName)) {
        topScopeLexEnvs_[variableName] = topScopeIdx_++;
    }

    return UINT32_MAX;
}

uint32_t Hotfix::GetPatchLexicalIdx(const std::string &variableName)
{
    ASSERT(topScopeLexEnvs_.count(variableName));
    return topScopeLexEnvs_[variableName];
}

bool IsFunctionOrClassDefineIns(panda::pandasm::Ins &ins)
{
    if (ins.opcode == panda::pandasm::Opcode::ECMA_DEFINEGENERATORFUNC ||
        ins.opcode == panda::pandasm::Opcode::ECMA_DEFINEMETHOD ||
        ins.opcode == panda::pandasm::Opcode::ECMA_DEFINEFUNCDYN ||
        ins.opcode == panda::pandasm::Opcode::ECMA_DEFINEASYNCFUNC ||
        ins.opcode == panda::pandasm::Opcode::ECMA_DEFINECLASSWITHBUFFER) {
        return true;
    }
    return false;
}

void Hotfix::CollectFuncDefineIns(panda::pandasm::Function *func)
{
    for (size_t i = 0; i < func->ins.size(); ++i) {
        if (IsFunctionOrClassDefineIns(func->ins[i])) {
            funcDefineIns_.push_back(func->ins[i]);  // push define ins
            funcDefineIns_.push_back(func->ins[i + 1]);  // push store ins
        }
    }
}

void Hotfix::AddInsForNewFunction(panda::pandasm::Program *prog, std::vector<panda::pandasm::Ins> &ins)
{
    panda::pandasm::Ins returnUndefine;
    returnUndefine.opcode = pandasm::Opcode::ECMA_RETURNUNDEFINED;

    if (ins.size() == 0) {
        ins.push_back(returnUndefine);
        return;
    }

    panda::pandasm::Ins newLexenv;
    newLexenv.opcode = pandasm::Opcode::ECMA_NEWLEXENVDYN;
    newLexenv.imms.reserve(1);
    auto newFuncNum = long(ins.size() / 2);  // each new function has 2 ins: define and store
    newLexenv.imms.emplace_back(newFuncNum);

    panda::pandasm::Ins stLexenv;
    stLexenv.opcode = pandasm::Opcode::STA_DYN;
    stLexenv.regs.reserve(1);
    stLexenv.regs.emplace_back(PATCH_ENV_VREG);

    ins.insert(ins.begin(), stLexenv);
    ins.insert(ins.begin(), newLexenv);
    ins.push_back(returnUndefine);
}

void Hotfix::Finalize(panda::pandasm::Program **prog)
{
    if (mapFile_.empty()) {
        return;
    }

    if (patchError_) {
        *prog = nullptr;
        std::cerr << "Found unsupported change in file, will not generate patch!" << std::endl;
        return;
    }

    // add patch main for new function and modified function
    std::vector<panda::pandasm::Ins> newFuncDefineIns_;
    std::vector<panda::pandasm::Ins> patchFuncDefineIns_;

    for (size_t i = 0; i < funcDefineIns_.size(); ++i) {
        if (IsFunctionOrClassDefineIns(funcDefineIns_[i])) {
            auto name = funcDefineIns_[i].ids[0];
            if (newFuncNames_.count(name)) {
                funcDefineIns_[i].regs[0] = PATCH_ENV_VREG;
                newFuncDefineIns_.push_back(funcDefineIns_[i]);
                newFuncDefineIns_.push_back(funcDefineIns_[i + 1]);
                continue;
            }
            if (patchFuncNames_.count(name)) {
                patchFuncDefineIns_.push_back(funcDefineIns_[i]);
                continue;
            }
        }
    }

    AddInsForNewFunction(*prog, newFuncDefineIns_);

    auto srcLang = panda::panda_file::SourceLang::ECMASCRIPT;
    panda::pandasm::Function funcNew(PATCH_MAIN_NEW, srcLang);
    panda::pandasm::Function funcPatch(PATCH_MAIN_MODIFIED, srcLang);

    size_t defaultParamCount = 3;
    funcNew.params.reserve(defaultParamCount);
    funcPatch.params.reserve(defaultParamCount);
    for (uint32_t i = 0; i < defaultParamCount; ++i) {
        funcNew.params.emplace_back(panda::pandasm::Type("any", 0), srcLang);
        funcPatch.params.emplace_back(panda::pandasm::Type("any", 0), srcLang);
    }

    funcNew.ins = newFuncDefineIns_;
    funcPatch.ins = patchFuncDefineIns_;

    (*prog)->function_table.emplace(funcNew.name, std::move(funcNew));
    (*prog)->function_table.emplace(funcPatch.name, std::move(funcPatch));

    // As this is the last step of patch generating, clear hotfix object
    Destroy();
}

void Hotfix::MarkFunctionForPatch(const compiler::PandaGen *pg, panda::pandasm::Function *func,
    LiteralBuffers &literalBuffers)
{
    std::string funcName = func->name;
    if (!originFunctionInfo_.count(funcName)) {
        if (IsAnonymousOrDuplicateNameFunction(funcName)) {
            std::cerr << "Found new anonymous or duplicate name function " << funcName << " not supported!" << std::endl;
            patchError_ = true;
            return;
        }
        newFuncNames_.insert(funcName);
        CollectFuncDefineIns(func);
        return;
    }

    auto bytecodeInfo = originFunctionInfo_.at(funcName);

    // compare lexenv
    auto &lexicalVarNames = pg->TopScope()->GetLexicalVarNames();
    auto &lexicalVarTypes = pg->TopScope()->GetLexicalVarTypes();
    auto lexenv = bytecodeInfo.lexenv;
    if (funcName != FUNC_MAIN) {
        if (lexenv.size() != lexicalVarNames.size()) {
            std::cerr << "Found lexenv size changed, not supported!" << std::endl;
            patchError_ = true;
            return;
        }
        for (auto &variable: lexicalVarNames) {
            auto varName = std::string(variable.second);
            if (!lexenv.count(varName)) {
                std::cerr << "Found new lex env added, not supported!" << std::endl;
                patchError_ = true;
                return;
            }
            auto lexInfo = lexenv[varName];
            if (variable.first != lexInfo.first || lexicalVarTypes[variable.first] != lexInfo.second) {
                std::cerr << "Found new lex env changed(slot or type), not supported!" << std::endl;
                patchError_ = true;
                return;
            }
        }
    }

    // compare function hash
    auto hashList = GenerateFunctionAndClassHash(func, literalBuffers);
    auto funcHash = std::to_string(hashList.back().second);
    if (funcHash == bytecodeInfo.funcHash) {
        func->metadata->SetAttribute("external");
    } else {
        patchFuncNames_.insert(funcName);
    }

    CollectFuncDefineIns(func);

    // compare class
    auto classInfo = bytecodeInfo.classHash;
    for (size_t i = 0; i < hashList.size() - 1; ++i) {
        auto className = hashList[i].first;
        if (classInfo.count(className)) {
            if (classInfo[className] != std::to_string(hashList[i].second)) {
                std::cerr << "Found class " << hashList[i].first << " changed, not supported!" << std::endl;
                patchError_ = true;
                return;
            }
        }
    }
}

void Hotfix::DumpFunctionInfo(const compiler::PandaGen *pg, panda::pandasm::Function *func,
    Hotfix::LiteralBuffers &literalBuffers)
{
    std::stringstream ss;

    ss << pg->InternalName();
    ss << SEPERATOR << pg->InternalName() << SEPERATOR;

    std::vector<std::pair<std::string, size_t>> hashList = GenerateFunctionAndClassHash(func, literalBuffers);
    ss << hashList.back().second << SEPERATOR;

    ss << TYPE_SEPERATOR;
    for (size_t i = 0; i < hashList.size() - 1; ++i) {
        ss << hashList[i].first << SEPERATOR << hashList[i].second << SEPERATOR;
    }
    ss << SEPERATOR << TYPE_SEPERATOR;

    for (auto &variable: pg->TopScope()->GetLexicalVarNames()) {
        ss << variable.second << SEPERATOR
           << variable.first << SEPERATOR
           << pg->TopScope()->GetLexicalVarTypes()[variable.first] << SEPERATOR;
    }
    ss << SEPERATOR << std::endl;

    WriteMapFile(ss.str());
}

bool Hotfix::ReadMapFile(const std::string &mapFile)
{
    std::ifstream ifs;
    std::string line;
    ifs.open(mapFile.c_str());
    if (!ifs.is_open()) {
        std::cerr << "Failed to open map file: " << mapFile << std::endl;
        return false;
    }

    // read func infos
    while (std::getline(ifs, line)) {
        std::vector<std::string> itemList = GetStringItems(line, TYPE_SEPERATOR);

        if (itemList.size() == MAP_FUNCTION_ITEM_NUMBER) {
            struct OriginFunctionInfo info(&allocator_);
            std::vector<std::string> funcItems = GetStringItems(itemList[0], SEPERATOR);
            std::vector<std::string> classItems = GetStringItems(itemList[1], SEPERATOR);
            std::vector<std::string> lexItems = GetStringItems(itemList[2], SEPERATOR);

            info.funcName = funcItems[0];
            info.funcInternalName = funcItems[1];
            info.funcHash = funcItems[2];
            for (size_t i = 0; i < classItems.size(); i = i + 2) {
                info.classHash.insert(std::pair<std::string, std::string>(classItems[i], classItems[i + 1]));
            }
            for (size_t i = 0; i < lexItems.size(); i = i + 3) {
                std::pair<int, int> slotAndType(std::atoi(lexItems[i + 1].c_str()), std::atoi(lexItems[i + 2].c_str()));
                info.lexenv.insert(std::pair<std::string, std::pair<int, int>>(lexItems[i], slotAndType));
            }

            originFunctionInfo_.insert(std::pair<std::string, OriginFunctionInfo>(info.funcInternalName, info));
        } else if (itemList.size() == MAP_MODULE_ITEM_NUMBER) {
            std::vector<std::string> moduleItems = GetStringItems(itemList[0], SEPERATOR);
            originModuleInfo_.insert({moduleItems[0], moduleItems[1]});
        } else {
            std::cerr << "Failed to read map file: Error map file format" << std::endl;
        }
    }
    return true;
}

void Hotfix::WriteMapFile(const std::string &content)
{
    std::lock_guard<std::mutex> lock(m_);
    std::fstream fs;
    fs.open(dumpMapFile_, std::ios_base::app | std::ios_base::in);
    if (fs.is_open()) {
        fs << content;
        fs.close();
    }
}

} // namespace panda::es2panda::util
