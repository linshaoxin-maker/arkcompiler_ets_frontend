/*
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

#include "etsFunctionType.h"

#include "varbinder/scope.h"
#include "checker/TSchecker.h"
#include "checker/ETSchecker.h"
#include "checker/types/signature.h"
#include "ir/astDump.h"
#include "ir/base/spreadElement.h"
#include "ir/base/methodDefinition.h"
#include "ir/base/scriptFunction.h"
#include "ir/expressions/identifier.h"
#include "ir/ts/tsTypeParameter.h"
#include "ir/ts/tsInterfaceDeclaration.h"
#include "ir/ts/tsInterfaceBody.h"
#include "ir/ts/tsTypeParameterDeclaration.h"
#include "ir/ets/etsParameterExpression.h"

namespace panda::es2panda::ir {
void ETSFunctionType::TransformChildren(const NodeTransformer &cb)
{
    if (type_params_ != nullptr) {
        type_params_ = cb(type_params_)->AsTSTypeParameterDeclaration();
    }

    for (auto *&it : params_) {
        it = cb(it)->AsExpression();
    }

    if (return_type_ != nullptr) {
        return_type_ = static_cast<TypeNode *>(cb(return_type_));
    }
}

void ETSFunctionType::Iterate(const NodeTraverser &cb) const
{
    if (type_params_ != nullptr) {
        cb(type_params_);
    }

    for (auto *it : params_) {
        cb(it);
    }

    if (return_type_ != nullptr) {
        cb(return_type_);
    }
}

void ETSFunctionType::Dump(ir::AstDumper *dumper) const
{
    dumper->Add({{"type", "ETSFunctionType"},
                 {"params", params_},
                 {"typeParameters", AstDumper::Optional(type_params_)},
                 {"returnType", return_type_}});

    if (IsThrowing()) {
        dumper->Add({"throwMarker", "throws"});
    }
}

void ETSFunctionType::Compile([[maybe_unused]] compiler::PandaGen *pg) const {}

checker::Type *ETSFunctionType::Check([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *ETSFunctionType::GetType([[maybe_unused]] checker::TSChecker *checker)
{
    return nullptr;
}

checker::Type *ETSFunctionType::Check(checker::ETSChecker *checker)
{
    auto *generic_interface_type = checker->GlobalBuiltinFunctionType(params_.size());
    functional_interface_ = generic_interface_type->GetDeclNode()->AsTSInterfaceDeclaration();

    ts_type_ = checker->GetFunctionalInterface(this);
    if (ts_type_ != nullptr) {
        return ts_type_;
    }

    auto *invoke_func = functional_interface_->Body()->Body()[0]->AsMethodDefinition()->Function();

    auto *substitution = checker->NewSubstitution();

    auto N = checker->GlobalBuiltinFunctionTypeVariadicThreshold();

    size_t i = 0;
    if (params_.size() < N) {
        for (; i < params_.size(); i++) {
            auto *param_type =
                checker->GetTypeFromTypeAnnotation(params_[i]->AsETSParameterExpression()->TypeAnnotation());
            if (param_type->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE)) {
                checker->Relation()->SetNode(params_[i]);
                auto *const boxed_type_arg = checker->PrimitiveTypeAsETSBuiltinType(param_type);
                ASSERT(boxed_type_arg);
                param_type = boxed_type_arg->Instantiate(checker->Allocator(), checker->Relation(),
                                                         checker->GetGlobalTypesHolder());
            }

            substitution->emplace(generic_interface_type->TypeArguments()[i], param_type);
        }
    }

    auto *return_type = return_type_->GetType(checker);
    if (return_type->HasTypeFlag(checker::TypeFlag::ETS_PRIMITIVE)) {
        checker->Relation()->SetNode(return_type_);
        auto *const boxed_type_ret = checker->PrimitiveTypeAsETSBuiltinType(return_type);
        return_type =
            boxed_type_ret->Instantiate(checker->Allocator(), checker->Relation(), checker->GetGlobalTypesHolder());
    }

    substitution->emplace(generic_interface_type->TypeArguments()[i], return_type);

    auto *interface_type =
        generic_interface_type->Substitute(checker->Relation(), substitution, false)->AsETSObjectType();

    util::StringView invoke_name = "invoke";
    auto *invoke_variable = interface_type->GetOwnProperty<checker::PropertyType::INSTANCE_METHOD>(invoke_name);
    ASSERT(invoke_variable == nullptr);

    auto *decl = checker->Allocator()->New<varbinder::FunctionDecl>(checker->Allocator(), invoke_name,
                                                                    interface_type->GetDeclNode());
    invoke_variable = checker->Allocator()->New<varbinder::LocalVariable>(decl, varbinder::VariableFlags::SYNTHETIC |
                                                                                    varbinder::VariableFlags::METHOD);

    auto *signature_info = checker->Allocator()->New<checker::SignatureInfo>(checker->Allocator());

    for (auto *p : params_) {
        auto *const param = p->AsETSParameterExpression();
        if (param->IsRestParameter()) {
            auto *rest_ident = param->Ident();

            ASSERT(rest_ident->Variable());
            signature_info->rest_var = rest_ident->Variable()->AsLocalVariable();

            ASSERT(param->TypeAnnotation());
            signature_info->rest_var->SetTsType(checker->GetTypeFromTypeAnnotation(param->TypeAnnotation()));

            auto array_type = signature_info->rest_var->TsType()->AsETSArrayType();
            checker->CreateBuiltinArraySignature(array_type, array_type->Rank());
        } else {
            auto *param_ident = param->Ident();

            ASSERT(param_ident->Variable());
            varbinder::Variable *param_var = param_ident->Variable();

            ASSERT(param->TypeAnnotation());
            param_var->SetTsType(checker->GetTypeFromTypeAnnotation(param->TypeAnnotation()));
            signature_info->params.push_back(param_var->AsLocalVariable());
            ++signature_info->min_arg_count;
        }
    }

    auto *signature =
        checker->Allocator()->New<checker::Signature>(signature_info, return_type_->GetType(checker), invoke_func);

    signature->SetOwnerVar(invoke_variable);
    signature->AddSignatureFlag(checker::SignatureFlags::FUNCTIONAL_INTERFACE_SIGNATURE);
    signature->SetOwner(interface_type);

    if (IsThrowing()) {
        signature->AddSignatureFlag(checker::SignatureFlags::THROWS);
    }

    if (IsRethrowing()) {
        signature->AddSignatureFlag(checker::SignatureFlags::RETHROWS);
    }

    auto *func_type = checker->CreateETSFunctionType(signature, invoke_name);
    func_type->AddTypeFlag(checker::TypeFlag::SYNTHETIC);
    invoke_variable->SetTsType(func_type);
    interface_type->AddProperty<checker::PropertyType::INSTANCE_METHOD>(invoke_variable);

    checker->CacheFunctionalInterface(this, interface_type);

    ts_type_ = interface_type;
    return interface_type;
}

checker::Type *ETSFunctionType::GetType(checker::ETSChecker *checker)
{
    return Check(checker);
}

}  // namespace panda::es2panda::ir
