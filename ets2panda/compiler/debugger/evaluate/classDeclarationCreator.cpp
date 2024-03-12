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

#include "classDeclarationCreator.h"
#include "libpandafile/include/type.h"
#include "checker/ETSchecker.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "libpandafile/debug_data_accessor-inl.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "compiler/debugger/debugInfoLookup.h"

namespace ark::es2panda {

void ClassDeclarationCreator::CreateClassDeclaration(const util::StringView &identName, panda_file::ClassDataAccessor *cda)
{
    std::cout << cda << std::endl;
    ASSERT(cda != nullptr);

    checker::ETSChecker::ClassBuilder classBuilder = [&](varbinder::ClassScope *scope,
                                                         ArenaVector<ir::AstNode *> *classBody) {
        CreateClassBody(scope, classBody, cda);
    };
    checker_->BuildClass(identName, classBuilder);
}

void ClassDeclarationCreator::CreateClassBody(varbinder::ClassScope *scope, ArenaVector<ir::AstNode *> *classBody,
                                              panda_file::ClassDataAccessor *cda)
{
    ASSERT(scope != nullptr);
    ASSERT(classBody != nullptr);
    ASSERT(cda != nullptr);

    auto *classType = scope->Node()->AsClassDeclaration()->Definition()->TsType()->AsETSObjectType();
    CreateFieldsProperties(scope, classBody, classType, cda);
    CreateFunctionProperties(scope, classBody, classType, cda);
}

void ClassDeclarationCreator::CreateFieldsProperties(varbinder::ClassScope *scope,
                                                     ArenaVector<ir::AstNode *> *classBody,
                                                     checker::ETSObjectType *classType,
                                                     panda_file::ClassDataAccessor *cda)
{
    const auto &pf = cda->GetPandaFile();

    cda->EnumerateFields([&](panda_file::FieldDataAccessor &fda) -> void {
        const char *name = utf::Mutf8AsCString(pf.GetStringData(fda.GetNameId()).data);
        std::cout << name << std::endl;
        checker::Type *checkerType = nullptr;

std::cout << "++++++++++++++++++++++++++" << std::endl;
        auto pandaType = panda_file::Type::GetTypeFromFieldEncoding(fda.GetType());
        if (pandaType.IsReference()) {
            auto typeId = panda_file::FieldDataAccessor::GetTypeId(pf, fda.GetFieldId());
            std::string refName = utf::Mutf8AsCString(pf.GetStringData(typeId).data);
            // Remove L and ; symbols
            refName.erase(refName.begin());
            refName.erase(refName.size() - 1, 1);
            checkerType = ResolveReferenceType(refName);
        } else {
            checkerType = ToCheckerType(pandaType);
        }
std::cout << "=========================" << std::endl;
        auto *fieldIdent = checker_->AllocNode<ir::Identifier>(name, allocator_);
        auto *field = checker_->AllocNode<ir::ClassProperty>(
            fieldIdent, nullptr, nullptr, ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC, allocator_, false);
        field->SetTsType(checkerType);
std::cout << "----------------------------" << std::endl;
        auto *decl = allocator_->New<varbinder::LetDecl>(fieldIdent->Name());
        decl->BindNode(field);

        auto *var = scope->AddDecl(allocator_, decl, checker_->VarBinder()->Extension());
        var->AddFlag(varbinder::VariableFlags::PROPERTY);
        var->SetTsType(checkerType);
        fieldIdent->SetVariable(var);

        bool isStatic = fda.IsStatic();
        if (isStatic) {
            classType->AddProperty<checker::PropertyType::STATIC_FIELD>(var->AsLocalVariable());
        } else {
            classType->AddProperty<checker::PropertyType::INSTANCE_FIELD>(var->AsLocalVariable());
        }
        classBody->push_back(field);
    });
}

std::vector<checker::Type *> ClassDeclarationCreator::GetFunctionParameters(panda_file::MethodDataAccessor &mda)
{
    const auto &pf = mda.GetPandaFile();

    std::vector<checker::Type *> parameters;
 
    // auto debugInfoId = mda.GetDebugInfoId();
    // if (!debugInfoId) {
    //     return parameters;
    // }
    
    // panda_file::DebugInfoDataAccessor dda(pf, debugInfoId.value());
    // panda_file::ProtoDataAccessor pda(pf, mda.GetProtoId());

    // size_t idxRef = 0;

    // dda.EnumerateParameters([&](panda_file::File::EntityId &paramId) {
    //     if (paramId.IsValid()) {
    //         auto refType = pda.GetReferenceType(idxRef++);
    //         info.signature = utf::Mutf8AsCString(pf.GetStringData(refType).data);
    //         paramNames.push_back(utf::Mutf8AsCString(pf.GetStringData(paramId).data));
    //     } else {
    //         std::cout << "dda.EnumerateParameters error" << std::endl;
    //     }
    // });

    mda.EnumerateTypesInProto([&](panda_file::Type type, panda_file::File::EntityId classId) -> void {
        checker::Type *checkerType = nullptr;

        if (type.IsReference()) {
            checkerType = ResolveReferenceType(utf::Mutf8AsCString(pf.GetStringData(classId).data));
        } else {
            checkerType = ToCheckerType(type);
        }

        parameters.push_back(checkerType);
    },
    true); // true -- skip this parameter

    return parameters;
}

checker::Type *ClassDeclarationCreator::ResolveReferenceType(const std::string &refName)
{
    std::cout << "NAME = " << refName << std::endl;
    util::UString name(refName, allocator_);
    auto *ident = checker_->AllocNode<ir::Identifier>(name.View(), allocator_);
    return checker_->ResolveIdentifier(ident);
}

void ClassDeclarationCreator::CreateFunctionProperties(varbinder::ClassScope *classScope,
                                                       ArenaVector<ir::AstNode *> *classBody,
                                                       checker::ETSObjectType *classType,
                                                       panda_file::ClassDataAccessor *cda)
{
    cda->EnumerateMethods([&](panda_file::MethodDataAccessor &mda) -> void {
        std::string methodNameStr = mda.GetFullName();
        if (methodNameStr == "<ctor>" || methodNameStr == "<cctor>") {
            // todo support constructors
            return;
        }
        util::UString methodName(methodNameStr, allocator_);

        std::cout << methodNameStr << " " << mda.GetClassName() << std::endl;
        auto parameters = GetFunctionParameters(mda);
        bool isStatic = mda.IsStatic();

        auto flags = ir::ModifierFlags::PUBLIC;
        if (isStatic) {
            flags = flags | ir::ModifierFlags::STATIC;
        }
    
        auto methodBuilder = [&](varbinder::FunctionScope *scope, 
                                 ArenaVector<ir::Statement *> *stms,
                                 ArenaVector<ir::Expression *> *fparams, 
                                 checker::Type **rettype) -> void {
            *rettype = parameters[0];
            // NOTE
            // Dirty hack to make checker think that we return something nonvoid
            // But the bytecode will still generate return.void
            // Maybe this hack won't work in the future.
            auto *retStatement = checker_->AllocNode<ir::ReturnStatement>();
            stms->push_back(retStatement);
            
            auto *paramScope = scope->Parent()->AsFunctionParamScope();
            
            if (!isStatic) {
                util::UString thisParamName(std::string("this"), allocator_);
                auto *thisParam = checker_->AddParam(paramScope, thisParamName.View(), classType);
                fparams->push_back(thisParam);
            }

            for (size_t idx = 1; idx < parameters.size(); ++idx) {
                util::UString paramName(std::string("field") + std::to_string(idx), allocator_);
                auto *param = checker_->AddParam(paramScope, paramName.View(), parameters[idx]);
                fparams->push_back(param);
            }
        };

        auto *method = checker_->CreateClassMethod(isStatic, classScope, methodName.View(), flags, methodBuilder, false);
        classBody->push_back(method);

        varbinder::LocalScope *methodScope = nullptr;
        if (isStatic) {
            methodScope = classScope->StaticMethodScope();
        } else {
            methodScope = classScope->InstanceFieldScope();
        }

        auto *decl = allocator_->New<varbinder::FunctionDecl>(allocator_, methodName.View(), method);
        auto var = methodScope->AddDecl(allocator_, decl, ScriptExtension::ETS);
        var->AddFlag(varbinder::VariableFlags::METHOD);
    });
}

checker::Type *ClassDeclarationCreator::ToCheckerType(panda_file::Type pandaFileType)
{
    switch (pandaFileType.GetId()) {
        case panda_file::Type::TypeId::VOID:
            return checker_->GetGlobalTypesHolder()->GlobalETSVoidType();
        case panda_file::Type::TypeId::U1:
            return checker_->GetGlobalTypesHolder()->GlobalBooleanType();
        case panda_file::Type::TypeId::I8:
            return checker_->GetGlobalTypesHolder()->GlobalByteType();
        case panda_file::Type::TypeId::U8:
            return checker_->GetGlobalTypesHolder()->GlobalByteType();
        case panda_file::Type::TypeId::I16:
            return checker_->GetGlobalTypesHolder()->GlobalShortType();
        case panda_file::Type::TypeId::U16:
            return checker_->GetGlobalTypesHolder()->GlobalShortType();
        case panda_file::Type::TypeId::I32:
            return checker_->GetGlobalTypesHolder()->GlobalIntType();
        case panda_file::Type::TypeId::U32:
            return checker_->GetGlobalTypesHolder()->GlobalIntType();
        case panda_file::Type::TypeId::I64:
            return checker_->GetGlobalTypesHolder()->GlobalLongType();
        case panda_file::Type::TypeId::U64:
            return checker_->GetGlobalTypesHolder()->GlobalLongType();
        case panda_file::Type::TypeId::F32:
            return checker_->GetGlobalTypesHolder()->GlobalFloatType();
        case panda_file::Type::TypeId::F64:
            return checker_->GetGlobalTypesHolder()->GlobalDoubleType();
        case panda_file::Type::TypeId::REFERENCE:
            std::cout << "reference" << std::endl;		
            [[fallthrough]];
        case panda_file::Type::TypeId::TAGGED:
            std::cout << "tagged" << std::endl;
            [[fallthrough]];
        case panda_file::Type::TypeId::INVALID:
            std::cout << "invalid" << std::endl;
            [[fallthrough]];
        default:
            break;
    }
    UNREACHABLE();
}

}  // namespace ark::es2panda
