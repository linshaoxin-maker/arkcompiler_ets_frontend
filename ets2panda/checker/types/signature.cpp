/**
 * Copyright (c) 2021 - 2023 Huawei Device Co., Ltd.
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

#include "signature.h"

#include "varbinder/scope.h"
#include "ir/base/scriptFunction.h"
#include "ir/ts/tsTypeParameter.h"
#include "checker/ETSchecker.h"

namespace ark::es2panda::checker {

util::StringView Signature::InternalName() const
{
    return internalName_.Empty() ? func_->Scope()->InternalName() : internalName_;
}

Signature *Signature::Substitute(TypeRelation *relation, const Substitution *substitution)
{
    if (substitution == nullptr || substitution->empty()) {
        return this;
    }
    auto *checker = relation->GetChecker()->AsETSChecker();
    auto *allocator = checker->Allocator();
    bool anyChange = false;
    SignatureInfo *newSigInfo = allocator->New<SignatureInfo>(allocator);
    const Substitution *newSubstitution = substitution;

    if (!signatureInfo_->typeParams.empty()) {
        auto *newSubstitutionSeed = checker->CopySubstitution(substitution);
        for (auto *tparam : signatureInfo_->typeParams) {
            auto *newTparam = tparam->Substitute(relation, newSubstitutionSeed);
            newSigInfo->typeParams.push_back(newTparam);
            anyChange |= (newTparam != tparam);
            if (newTparam != tparam && tparam->IsETSTypeParameter()) {
                newSubstitutionSeed->insert({tparam->AsETSTypeParameter(), newTparam});
            }
        }
        newSubstitution = newSubstitutionSeed;
    }
    newSigInfo->minArgCount = signatureInfo_->minArgCount;

    for (auto *param : signatureInfo_->params) {
        auto *newParam = param;
        auto *newParamType = param->TsType()->Substitute(relation, newSubstitution);
        if (newParamType != param->TsType()) {
            anyChange = true;
            newParam = param->Copy(allocator, param->Declaration());
            newParam->SetTsType(newParamType);
        }
        newSigInfo->params.push_back(newParam);
    }

    if (signatureInfo_->restVar != nullptr) {
        auto *newRestType = signatureInfo_->restVar->TsType()->Substitute(relation, newSubstitution);
        if (newRestType != signatureInfo_->restVar->TsType()) {
            anyChange = true;
            newSigInfo->restVar = signatureInfo_->restVar->Copy(allocator, signatureInfo_->restVar->Declaration());
            newSigInfo->restVar->SetTsType(newRestType);
        }
    }

    if (!anyChange) {
        newSigInfo = signatureInfo_;
    }

    auto *newReturnType = returnType_->Substitute(relation, newSubstitution);
    if (newReturnType != returnType_) {
        anyChange = true;
    }
    if (!anyChange) {
        return this;
    }
    auto *result = allocator->New<Signature>(newSigInfo, newReturnType);
    result->func_ = func_;
    result->flags_ = flags_;
    result->internalName_ = internalName_;
    result->ownerObj_ = ownerObj_;
    result->ownerVar_ = ownerVar_;

    return result;
}

Signature *Signature::Copy(ArenaAllocator *allocator, TypeRelation *relation, GlobalTypesHolder *globalTypes)
{
    SignatureInfo *copiedInfo = allocator->New<SignatureInfo>(signatureInfo_, allocator);

    for (size_t idx = 0; idx < signatureInfo_->params.size(); idx++) {
        auto *const paramType = signatureInfo_->params[idx]->TsType();
        if (paramType->HasTypeFlag(TypeFlag::GENERIC) && paramType->IsETSObjectType()) {
            copiedInfo->params[idx]->SetTsType(paramType->Instantiate(allocator, relation, globalTypes));
            auto originalTypeArgs = paramType->AsETSObjectType()->GetOriginalBaseType()->TypeArguments();
            copiedInfo->params[idx]->TsType()->AsETSObjectType()->SetTypeArguments(std::move(originalTypeArgs));
        } else {
            copiedInfo->params[idx]->SetTsType(
                ETSChecker::TryToInstantiate(paramType, allocator, relation, globalTypes));
        }
    }

    auto *const copiedSignature = allocator->New<Signature>(copiedInfo, returnType_, func_);
    copiedSignature->flags_ = flags_;
    copiedSignature->internalName_ = internalName_;
    copiedSignature->ownerObj_ = ownerObj_;
    copiedSignature->ownerVar_ = ownerVar_;

    return copiedSignature;
}

void Signature::ToString(std::stringstream &ss, const varbinder::Variable *variable, bool printAsMethod) const
{
    if (!signatureInfo_->typeParams.empty()) {
        ss << "<";
        for (auto it = signatureInfo_->typeParams.begin(); it != signatureInfo_->typeParams.end(); ++it) {
            (*it)->ToString(ss);
            if (std::next(it) != signatureInfo_->typeParams.end()) {
                ss << ", ";
            }
        }
        ss << ">";
    }

    ss << "(";

    for (auto it = signatureInfo_->params.begin(); it != signatureInfo_->params.end(); it++) {
        ss << (*it)->Name();

        if ((*it)->HasFlag(varbinder::VariableFlags::OPTIONAL)) {
            ss << "?";
        }

        ss << ": ";

        (*it)->TsType()->ToString(ss);

        if (std::next(it) != signatureInfo_->params.end()) {
            ss << ", ";
        }
    }

    if (signatureInfo_->restVar != nullptr) {
        if (!signatureInfo_->params.empty()) {
            ss << ", ";
        }

        ss << "...";
        ss << signatureInfo_->restVar->Name();
        ss << ": ";
        signatureInfo_->restVar->TsType()->ToString(ss);
        ss << "[]";
    }

    ss << ")";

    if (printAsMethod || (variable != nullptr && variable->HasFlag(varbinder::VariableFlags::METHOD))) {
        ss << ": ";
    } else {
        ss << " => ";
    }

    returnType_->ToString(ss);
}

namespace {
std::size_t GetToCheckParamCount(Signature *signature, bool isEts)
{
    auto paramNumber = static_cast<ssize_t>(signature->Params().size());
    if (!isEts || signature->Function() == nullptr) {
        return paramNumber;
    }
    for (auto i = paramNumber - 1; i >= 0; i--) {
        if (!signature->Function()->Params()[i]->AsETSParameterExpression()->IsDefault()) {
            return static_cast<std::size_t>(i + 1);
        }
    }
    return 0;
}
}  // namespace

bool Signature::IdenticalParameter(TypeRelation *relation, Type *type1, Type *type2)
{
    if (!CheckFunctionalInterfaces(relation, type1, type2)) {
        relation->IsIdenticalTo(type1, type2);
    }
    return relation->IsTrue();
}

void Signature::Identical(TypeRelation *relation, Signature *other)
{
    bool isEts = relation->GetChecker()->IsETSChecker();
    auto const thisToCheckParametersNumber = GetToCheckParamCount(this, isEts);
    auto const otherToCheckParametersNumber = GetToCheckParamCount(other, isEts);
    if ((thisToCheckParametersNumber != otherToCheckParametersNumber || this->MinArgCount() != other->MinArgCount()) &&
        this->RestVar() == nullptr && other->RestVar() == nullptr) {
        // skip check for ets cases only when all parameters are mandatory
        if (!isEts || (thisToCheckParametersNumber == this->Params().size() &&
                       otherToCheckParametersNumber == other->Params().size())) {
            relation->Result(false);
            return;
        }
    }

    if (relation->NoReturnTypeCheck()) {
        relation->Result(true);
    } else {
        relation->IsIdenticalTo(this->ReturnType(), other->ReturnType());
    }

    if (relation->IsTrue()) {
        /* In ETS, the functions "foo(a: int)" and "foo(a: int, b: int = 1)" should be considered as having an
           equivalent signature. Hence, we only need to check if the mandatory parameters of the signature with
           more mandatory parameters can match the parameters of the other signature (including the optional
           parameter or rest parameters) here.

           XXX_to_check_parameters_number is calculated beforehand by counting mandatory parameters.
           Signature::params() stores all parameters (mandatory and optional), excluding the rest parameter.
           Signature::restVar() stores the rest parameters of the function.

           For example:
           foo(a: int): params().size: 1, to_check_param_number: 1, restVar: nullptr
           foo(a: int, b: int = 0): params().size: 2, to_check_param_number: 1, restVar: nullptr
           foo(a: int, ...b: int[]): params().size: 1, to_check_param_number: 1, restVar: ...b: int[]

           Note that optional parameters always come after mandatory parameters, and signatures containing both
           optional and rest parameters are not allowed.

           "to_check_parameters_number" is the number of parameters that need to be checked to ensure identical.
           "parameters_number" is the number of parameters that can be checked in Signature::params().
        */
        auto const toCheckParametersNumber = std::max(thisToCheckParametersNumber, otherToCheckParametersNumber);
        auto const parametersNumber =
            std::min({this->Params().size(), other->Params().size(), toCheckParametersNumber});

        std::size_t i = 0U;
        for (; i < parametersNumber; ++i) {
            if (!IdenticalParameter(relation, this->Params()[i]->TsType(), other->Params()[i]->TsType())) {
                return;
            }
        }

        /* "i" could be one of the following three cases:
            1. == to_check_parameters_number, we have finished the checking and can directly return.
            2. == other->Params().size(), must be < this_to_check_parameters_number in this case since
            xxx->Params().size() always >= xxx_to_check_parameters_number. We need to check the remaining
            mandatory parameters of "this" against ths RestVar of "other".
            3. == this->Params().size(), must be < other_to_check_parameters_number as described in 2, and
            we need to check the remaining mandatory parameters of "other" against the RestVar of "this".
        */
        if (i == toCheckParametersNumber) {
            return;
        }
        bool isOtherMandatoryParamsMatched = i < thisToCheckParametersNumber;
        ArenaVector<varbinder::LocalVariable *> const &parameters =
            isOtherMandatoryParamsMatched ? this->Params() : other->Params();
        varbinder::LocalVariable const *restParameter =
            isOtherMandatoryParamsMatched ? other->RestVar() : this->RestVar();
        if (restParameter == nullptr) {
            relation->Result(false);
            return;
        }
        auto *const restParameterType = restParameter->TsType()->AsETSArrayType()->ElementType();
        for (; i < toCheckParametersNumber; ++i) {
            if (!IdenticalParameter(relation, parameters[i]->TsType(), restParameterType)) {
                return;
            }
        }
    }
}

bool Signature::CheckFunctionalInterfaces(TypeRelation *relation, Type *source, Type *target)
{
    if (!source->IsETSObjectType() || !source->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::FUNCTIONAL)) {
        return false;
    }

    if (!target->IsETSObjectType() || !target->AsETSObjectType()->HasObjectFlag(ETSObjectFlags::FUNCTIONAL)) {
        return false;
    }

    auto sourceInvokeFunc = source->AsETSObjectType()
                                ->GetProperty(util::StringView("invoke"), PropertySearchFlags::SEARCH_INSTANCE_METHOD)
                                ->TsType()
                                ->AsETSFunctionType()
                                ->CallSignatures()[0];

    auto targetInvokeFunc = target->AsETSObjectType()
                                ->GetProperty(util::StringView("invoke"), PropertySearchFlags::SEARCH_INSTANCE_METHOD)
                                ->TsType()
                                ->AsETSFunctionType()
                                ->CallSignatures()[0];

    relation->IsIdenticalTo(sourceInvokeFunc, targetInvokeFunc);
    return true;
}

void Signature::AssignmentTarget(TypeRelation *relation, Signature *source)
{
    if (signatureInfo_->restVar == nullptr &&
        (source->Params().size() - source->OptionalArgCount()) > signatureInfo_->params.size()) {
        relation->Result(false);
        return;
    }

    for (size_t i = 0; i < source->Params().size(); i++) {
        if (signatureInfo_->restVar == nullptr && i >= Params().size()) {
            break;
        }

        if (signatureInfo_->restVar != nullptr) {
            relation->IsAssignableTo(source->Params()[i]->TsType(), signatureInfo_->restVar->TsType());

            if (!relation->IsTrue()) {
                return;
            }

            continue;
        }

        relation->IsAssignableTo(source->Params()[i]->TsType(), Params()[i]->TsType());

        if (!relation->IsTrue()) {
            return;
        }
    }

    relation->IsAssignableTo(source->ReturnType(), returnType_);

    if (relation->IsTrue() && signatureInfo_->restVar != nullptr && source->RestVar() != nullptr) {
        relation->IsAssignableTo(source->RestVar()->TsType(), signatureInfo_->restVar->TsType());
    }
}
}  // namespace ark::es2panda::checker
