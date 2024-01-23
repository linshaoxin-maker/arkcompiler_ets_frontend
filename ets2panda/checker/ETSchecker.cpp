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

#include "ETSchecker.h"

#include "es2panda.h"
#include "ir/base/classDefinition.h"
#include "ir/expression.h"
#include "ir/expressions/callExpression.h"
#include "ir/ts/tsInterfaceDeclaration.h"
#include "ir/statements/blockStatement.h"
#include "varbinder/ETSBinder.h"
#include "parser/program/program.h"
#include "checker/ets/aliveAnalyzer.h"

#include "ir/base/scriptFunction.h"
#include "util/helpers.h"

namespace panda::es2panda::checker {
void ETSChecker::InitializeBuiltins(varbinder::ETSBinder *varbinder)
{
    if (HasStatus(CheckerStatus::BUILTINS_INITIALIZED)) {
        return;
    }

    const auto varMap = varbinder->TopScope()->Bindings();

    auto initBuiltin = [varMap](ETSChecker *checker, std::string_view signature) -> util::StringView {
        const auto iterator = varMap.find(signature);
        ASSERT(iterator != varMap.end());
        checker->GetGlobalTypesHolder()->InitializeBuiltin(
            iterator->first,
            checker->BuildClassProperties(iterator->second->Declaration()->Node()->AsClassDefinition()));
        return iterator->first;
    };

    auto const objectName = initBuiltin(this, compiler::Signatures::BUILTIN_OBJECT_CLASS);
    auto const voidName = initBuiltin(this, compiler::Signatures::BUILTIN_VOID_CLASS);

    for (const auto &[name, var] : varMap) {
        if (name == objectName || name == voidName) {
            continue;
        }

        if (var->HasFlag(varbinder::VariableFlags::BUILTIN_TYPE)) {
            InitializeBuiltin(var, name);
        }
    }

    AddStatus(CheckerStatus::BUILTINS_INITIALIZED);
}

void ETSChecker::InitializeBuiltin(varbinder::Variable *var, const util::StringView &name)
{
    Type *type {nullptr};
    if (var->Declaration()->Node()->IsClassDefinition()) {
        type = BuildClassProperties(var->Declaration()->Node()->AsClassDefinition());
    } else {
        ASSERT(var->Declaration()->Node()->IsTSInterfaceDeclaration());
        type = BuildInterfaceProperties(var->Declaration()->Node()->AsTSInterfaceDeclaration());
    }
    GetGlobalTypesHolder()->InitializeBuiltin(name, type);
}

bool ETSChecker::StartChecker([[maybe_unused]] varbinder::VarBinder *varbinder, const CompilerOptions &options)
{
    Initialize(varbinder);

    if (options.dumpAst) {
        std::cout << Program()->Dump() << std::endl;
    }

    if (options.opDumpAstOnlySilent) {
        Program()->DumpSilent();
        return false;
    }

    if (options.parseOnly) {
        return false;
    }

    varbinder->SetGenStdLib(options.compilationMode == CompilationMode::GEN_STD_LIB);
    varbinder->IdentifierAnalysis();

    auto *etsBinder = varbinder->AsETSBinder();
    InitializeBuiltins(etsBinder);

    for (auto &entry : etsBinder->DynamicImportVars()) {
        auto &data = entry.second;
        if (data.import->IsPureDynamic()) {
            data.variable->SetTsType(GlobalBuiltinDynamicType(data.import->Language()));
        }
    }

    CheckProgram(Program(), true);

    BuildDynamicCallClass(true);
    BuildDynamicCallClass(false);

    BuildDynamicImportClass();

#ifndef NDEBUG
    for (auto lambda : etsBinder->LambdaObjects()) {
        ASSERT(!lambda.second.first->TsType()->AsETSObjectType()->AssemblerName().Empty());
    }
    for (auto *func : varbinder->Functions()) {
        ASSERT(!func->Node()->AsScriptFunction()->Scope()->InternalName().Empty());
    }
#endif

    if (options.dumpCheckedAst) {
        std::cout << Program()->Dump() << std::endl;
    }

    return true;
}

void ETSChecker::CheckProgram(parser::Program *program, bool runAnalysis)
{
    auto *savedProgram = Program();
    SetProgram(program);

    for (auto &[_, extPrograms] : program->ExternalSources()) {
        (void)_;
        for (auto *extProg : extPrograms) {
            CheckProgram(extProg);
        }
    }

    ASSERT(Program()->Ast()->IsProgram());
    Program()->Ast()->Check(this);

    if (runAnalysis) {
        AliveAnalyzer(Program()->Ast(), this);
    }

    ASSERT(VarBinder()->AsETSBinder()->GetExternalRecordTable().find(program)->second);

    SetProgram(savedProgram);
}

Type *ETSChecker::CheckTypeCached(ir::Expression *expr)
{
    if (expr->TsType() == nullptr) {
        expr->SetTsType(expr->Check(this));
    }

    return expr->TsType();
}

ETSObjectType *ETSChecker::AsETSObjectType(Type *(GlobalTypesHolder::*typeFunctor)()) const
{
    auto *ret = (GetGlobalTypesHolder()->*typeFunctor)();
    return ret != nullptr ? ret->AsETSObjectType() : nullptr;
}

Type *ETSChecker::GlobalByteType() const
{
    return GetGlobalTypesHolder()->GlobalByteType();
}

Type *ETSChecker::GlobalShortType() const
{
    return GetGlobalTypesHolder()->GlobalShortType();
}

Type *ETSChecker::GlobalIntType() const
{
    return GetGlobalTypesHolder()->GlobalIntType();
}

Type *ETSChecker::GlobalLongType() const
{
    return GetGlobalTypesHolder()->GlobalLongType();
}

Type *ETSChecker::GlobalFloatType() const
{
    return GetGlobalTypesHolder()->GlobalFloatType();
}

Type *ETSChecker::GlobalDoubleType() const
{
    return GetGlobalTypesHolder()->GlobalDoubleType();
}

Type *ETSChecker::GlobalCharType() const
{
    return GetGlobalTypesHolder()->GlobalCharType();
}

Type *ETSChecker::GlobalETSBooleanType() const
{
    return GetGlobalTypesHolder()->GlobalETSBooleanType();
}

Type *ETSChecker::GlobalVoidType() const
{
    return GetGlobalTypesHolder()->GlobalETSVoidType();
}

Type *ETSChecker::GlobalETSNullType() const
{
    return GetGlobalTypesHolder()->GlobalETSNullType();
}

Type *ETSChecker::GlobalETSUndefinedType() const
{
    return GetGlobalTypesHolder()->GlobalETSUndefinedType();
}

Type *ETSChecker::GlobalETSStringLiteralType() const
{
    return GetGlobalTypesHolder()->GlobalETSStringLiteralType();
}

Type *ETSChecker::GlobalWildcardType() const
{
    return GetGlobalTypesHolder()->GlobalWildcardType();
}

ETSObjectType *ETSChecker::GlobalETSObjectType() const
{
    return AsETSObjectType(&GlobalTypesHolder::GlobalETSObjectType);
}

ETSObjectType *ETSChecker::GlobalETSNullishObjectType() const
{
    return AsETSObjectType(&GlobalTypesHolder::GlobalETSNullishObjectType);
}

ETSObjectType *ETSChecker::GlobalBuiltinETSStringType() const
{
    return AsETSObjectType(&GlobalTypesHolder::GlobalETSStringBuiltinType);
}

ETSObjectType *ETSChecker::GlobalBuiltinTypeType() const
{
    return AsETSObjectType(&GlobalTypesHolder::GlobalTypeBuiltinType);
}

ETSObjectType *ETSChecker::GlobalBuiltinExceptionType() const
{
    return AsETSObjectType(&GlobalTypesHolder::GlobalExceptionBuiltinType);
}

ETSObjectType *ETSChecker::GlobalBuiltinErrorType() const
{
    return AsETSObjectType(&GlobalTypesHolder::GlobalErrorBuiltinType);
}

ETSObjectType *ETSChecker::GlobalStringBuilderBuiltinType() const
{
    return AsETSObjectType(&GlobalTypesHolder::GlobalStringBuilderBuiltinType);
}

ETSObjectType *ETSChecker::GlobalBuiltinPromiseType() const
{
    return AsETSObjectType(&GlobalTypesHolder::GlobalPromiseBuiltinType);
}

ETSObjectType *ETSChecker::GlobalBuiltinJSRuntimeType() const
{
    return AsETSObjectType(&GlobalTypesHolder::GlobalJSRuntimeBuiltinType);
}

ETSObjectType *ETSChecker::GlobalBuiltinJSValueType() const
{
    return AsETSObjectType(&GlobalTypesHolder::GlobalJSValueBuiltinType);
}

ETSObjectType *ETSChecker::GlobalBuiltinVoidType() const
{
    return AsETSObjectType(&GlobalTypesHolder::GlobalBuiltinVoidType);
}

ETSObjectType *ETSChecker::GlobalBuiltinDynamicType(Language lang) const
{
    if (lang.GetId() == Language::Id::JS) {
        return GlobalBuiltinJSValueType();
    }
    return nullptr;
}

ETSObjectType *ETSChecker::GlobalBuiltinBoxType(const Type *contents) const
{
    switch (TypeKind(contents)) {
        case TypeFlag::ETS_BOOLEAN:
            return AsETSObjectType(&GlobalTypesHolder::GlobalBooleanBoxBuiltinType);
        case TypeFlag::BYTE:
            return AsETSObjectType(&GlobalTypesHolder::GlobalByteBoxBuiltinType);
        case TypeFlag::CHAR:
            return AsETSObjectType(&GlobalTypesHolder::GlobalCharBoxBuiltinType);
        case TypeFlag::SHORT:
            return AsETSObjectType(&GlobalTypesHolder::GlobalShortBoxBuiltinType);
        case TypeFlag::INT:
            return AsETSObjectType(&GlobalTypesHolder::GlobalIntBoxBuiltinType);
        case TypeFlag::LONG:
            return AsETSObjectType(&GlobalTypesHolder::GlobalLongBoxBuiltinType);
        case TypeFlag::FLOAT:
            return AsETSObjectType(&GlobalTypesHolder::GlobalFloatBoxBuiltinType);
        case TypeFlag::DOUBLE:
            return AsETSObjectType(&GlobalTypesHolder::GlobalDoubleBoxBuiltinType);
        default:
            return AsETSObjectType(&GlobalTypesHolder::GlobalBoxBuiltinType);
    }
}

const checker::WrapperDesc &ETSChecker::PrimitiveWrapper() const
{
    return primitiveWrappers_.Wrappers();
}

GlobalArraySignatureMap &ETSChecker::GlobalArrayTypes()
{
    return globalArraySignatures_;
}

const GlobalArraySignatureMap &ETSChecker::GlobalArrayTypes() const
{
    return globalArraySignatures_;
}

// For use in Signature::ToAssemblerType
const Type *MaybeBoxedType(Checker *checker, const varbinder::Variable *var)
{
    return checker->AsETSChecker()->MaybeBoxedType(var);
}

void ETSChecker::HandleUpdatedCallExpressionNode(ir::CallExpression *callExpr)
{
    VarBinder()->AsETSBinder()->HandleCustomNodes(callExpr);
}

}  // namespace panda::es2panda::checker
