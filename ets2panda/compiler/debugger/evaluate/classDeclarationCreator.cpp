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

namespace ark::es2panda {

void ClassDeclarationCreator::CreateClassDeclaration(const util::StringView &identName, panda_file::ClassDataAccessor *cda)
{
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
    cda->EnumerateFields([&](panda_file::FieldDataAccessor &fda) -> void {
        const char *name = reinterpret_cast<const char *>(cda->GetPandaFile().GetStringData(fda.GetNameId()).data);
        checker::Type *type = ToCheckerType(panda_file::Type::GetTypeFromFieldEncoding(fda.GetType()));

        auto *fieldIdent = checker_->AllocNode<ir::Identifier>(name, allocator_);
        auto *field = checker_->AllocNode<ir::ClassProperty>(
            fieldIdent, nullptr, nullptr, ir::ModifierFlags::STATIC | ir::ModifierFlags::PUBLIC, allocator_, false);
        field->SetTsType(type);

        auto *decl = allocator_->New<varbinder::LetDecl>(fieldIdent->Name());
        decl->BindNode(field);

        auto *var = scope->AddDecl(allocator_, decl, checker_->VarBinder()->Extension());
        var->AddFlag(varbinder::VariableFlags::PROPERTY);
        var->SetTsType(type);
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
    std::vector<checker::Type *> parameters;

    mda.EnumerateTypesInProto([&](panda_file::Type type, panda_file::File::EntityId classId) -> void{
        if (type.GetId() == panda_file::Type::TypeId::REFERENCE) {
            // NOTE: todo
            (void) classId;
            return;
        }
        parameters.push_back(ToCheckerType(type));
    },
    true); // true -- skip this parameter
 
    return parameters;
}

ir::ETSParameterExpression *ClassDeclarationCreator::AddParam(varbinder::FunctionParamScope *paramScope, 
                                                              util::StringView name,
                                                              checker::Type *type)
{
    auto paramCtx = varbinder::LexicalScope<varbinder::FunctionParamScope>::Enter(checker_->VarBinder(), paramScope, false);

    auto *paramIdent = checker_->AllocNode<ir::Identifier>(name, allocator_);
    auto *param = checker_->AllocNode<ir::ETSParameterExpression>(paramIdent, nullptr);
    auto *paramVar = std::get<1>(checker_->VarBinder()->AddParamDecl(param));
    
    paramVar->SetTsType(type);
    param->Ident()->SetVariable(paramVar);
    param->Ident()->SetTsType(type);
    
    return param;
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
                auto *thisParam = AddParam(paramScope, thisParamName.View(), classType);
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
        case panda_file::Type::TypeId::TAGGED:
        case panda_file::Type::TypeId::INVALID:
        default:
            break;
    }
    UNREACHABLE();
}

}  // namespace ark::es2panda
