/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ir/expressions/literals/bigIntLiteral.h"
#include "ir/expressions/literals/numberLiteral.h"
#include "ir/expressions/literals/stringLiteral.h"
#include "ir/expressions/functionExpression.h"
#include "ir/expressions/memberExpression.h"
#include "ir/expressions/identifier.h"
#include "ir/base/property.h"
#include "ir/base/scriptFunction.h"
#include "ir/base/spreadElement.h"
#include "ir/base/tsIndexSignature.h"
#include "ir/base/tsMethodSignature.h"
#include "ir/base/tsPropertySignature.h"
#include "ir/base/tsSignatureDeclaration.h"
#include "ir/ts/tsTypeLiteral.h"
#include "ir/ts/tsInterfaceDeclaration.h"
#include "ir/ts/tsInterfaceHeritage.h"
#include "ir/ts/tsInterfaceBody.h"
#include "util/helpers.h"
#include "varbinder/variable.h"
#include "varbinder/scope.h"

#include "checker/TSchecker.h"
#include "checker/types/ts/indexInfo.h"

namespace panda::es2panda::checker {
void TSChecker::CheckIndexConstraints(Type *type)
{
    if (!type->IsObjectType()) {
        return;
    }

    ObjectType *objType = type->AsObjectType();
    ResolveStructuredTypeMembers(objType);

    const IndexInfo *numberInfo = objType->NumberIndexInfo();
    const IndexInfo *stringInfo = objType->StringIndexInfo();
    const ArenaVector<varbinder::LocalVariable *> &properties = objType->Properties();

    if (numberInfo != nullptr) {
        for (auto *it : properties) {
            if (it->HasFlag(varbinder::VariableFlags::NUMERIC_NAME)) {
                Type *propType = GetTypeOfVariable(it);
                IsTypeAssignableTo(propType, numberInfo->GetType(),
                                   {"Property '", it->Name(), "' of type '", propType,
                                    "' is not assignable to numeric index type '", numberInfo->GetType(), "'."},
                                   it->Declaration()->Node()->Start());
            }
        }
    }

    if (stringInfo != nullptr) {
        for (auto *it : properties) {
            Type *propType = GetTypeOfVariable(it);
            IsTypeAssignableTo(propType, stringInfo->GetType(),
                               {"Property '", it->Name(), "' of type '", propType,
                                "' is not assignable to string index type '", stringInfo->GetType(), "'."},
                               it->Declaration()->Node()->Start());
        }

        if (numberInfo != nullptr && !IsTypeAssignableTo(numberInfo->GetType(), stringInfo->GetType())) {
            ThrowTypeError({"Number index info type ", numberInfo->GetType(),
                            " is not assignable to string index info type ", stringInfo->GetType(), "."},
                           numberInfo->Pos());
        }
    }
}

void TSChecker::ResolveStructuredTypeMembers(Type *type)
{
    if (type->IsObjectType()) {
        ObjectType *objType = type->AsObjectType();

        if (objType->IsObjectLiteralType()) {
            ResolveObjectTypeMembers(objType);
            return;
        }

        if (objType->IsInterfaceType()) {
            ResolveInterfaceOrClassTypeMembers(objType->AsInterfaceType());
            return;
        }
    }

    if (type->IsUnionType()) {
        ResolveUnionTypeMembers(type->AsUnionType());
        return;
    }
}

void TSChecker::ResolveUnionTypeMembers(UnionType *type)
{
    if (type->MergedObjectType() != nullptr) {
        return;
    }

    ObjectDescriptor *desc = Allocator()->New<ObjectDescriptor>(Allocator());
    UnionType::ConstituentsT stringInfoTypes(Allocator()->Adapter());
    UnionType::ConstituentsT numberInfoTypes(Allocator()->Adapter());
    ArenaVector<Signature *> callSignatures(Allocator()->Adapter());
    ArenaVector<Signature *> constructSignatures(Allocator()->Adapter());

    for (auto *it : type->AsUnionType()->ConstituentTypes()) {
        if (!it->IsObjectType()) {
            continue;
        }

        ObjectType *objType = it->AsObjectType();
        ResolveObjectTypeMembers(objType);

        if (!objType->CallSignatures().empty()) {
            for (auto *signature : objType->CallSignatures()) {
                callSignatures.push_back(signature);
            }
        }

        if (!objType->ConstructSignatures().empty()) {
            for (auto *signature : objType->ConstructSignatures()) {
                constructSignatures.push_back(signature);
            }
        }

        if (objType->StringIndexInfo() != nullptr) {
            stringInfoTypes.push_back(objType->StringIndexInfo()->GetType());
        }

        if (objType->NumberIndexInfo() != nullptr) {
            numberInfoTypes.push_back(objType->NumberIndexInfo()->GetType());
        }
    }

    desc->callSignatures = callSignatures;
    desc->constructSignatures = constructSignatures;

    if (!stringInfoTypes.empty()) {
        desc->stringIndexInfo = Allocator()->New<IndexInfo>(CreateUnionType(std::move(stringInfoTypes)), "x", false);
    }

    if (!numberInfoTypes.empty()) {
        desc->numberIndexInfo = Allocator()->New<IndexInfo>(CreateUnionType(std::move(numberInfoTypes)), "x", false);
    }

    ObjectType *mergedType = Allocator()->New<ObjectLiteralType>(desc);
    mergedType->AddObjectFlag(ObjectFlags::RESOLVED_MEMBERS);
    type->SetMergedObjectType(mergedType);
}

void TSChecker::ResolveInterfaceOrClassTypeMembers(InterfaceType *type)
{
    if (type->HasObjectFlag(ObjectFlags::RESOLVED_MEMBERS)) {
        return;
    }

    ResolveDeclaredMembers(type);
    GetBaseTypes(type);

    type->AddObjectFlag(ObjectFlags::RESOLVED_MEMBERS);
}

void TSChecker::ResolveObjectTypeMembers(ObjectType *type)
{
    if (!type->IsObjectLiteralType() || type->HasObjectFlag(ObjectFlags::RESOLVED_MEMBERS)) {
        return;
    }

    ASSERT(type->Variable() && type->Variable()->Declaration()->Node()->IsTSTypeLiteral());
    auto *typeLiteral = type->Variable()->Declaration()->Node()->AsTSTypeLiteral();
    ArenaVector<ir::TSSignatureDeclaration *> signatureDeclarations(Allocator()->Adapter());
    ArenaVector<ir::TSIndexSignature *> indexDeclarations(Allocator()->Adapter());

    for (auto *it : typeLiteral->Members()) {
        ResolvePropertiesOfObjectType(type, it, signatureDeclarations, indexDeclarations, false);
    }

    type->AddObjectFlag(ObjectFlags::RESOLVED_MEMBERS);

    ResolveSignaturesOfObjectType(type, signatureDeclarations);
    ResolveIndexInfosOfObjectType(type, indexDeclarations);
}

void TSChecker::ResolvePropertiesOfObjectType(ObjectType *type, ir::AstNode *member,
                                              ArenaVector<ir::TSSignatureDeclaration *> &signatureDeclarations,
                                              ArenaVector<ir::TSIndexSignature *> &indexDeclarations, bool isInterface)
{
    if (member->IsTSPropertySignature()) {
        varbinder::Variable *prop = member->AsTSPropertySignature()->Variable();

        if (!isInterface ||
            ValidateInterfaceMemberRedeclaration(type, prop, member->AsTSPropertySignature()->Key()->Start())) {
            type->AddProperty(prop->AsLocalVariable());
        }

        return;
    }

    if (member->IsTSMethodSignature()) {
        varbinder::Variable *method = member->AsTSMethodSignature()->Variable();

        if (!isInterface ||
            ValidateInterfaceMemberRedeclaration(type, method, member->AsTSMethodSignature()->Key()->Start())) {
            type->AddProperty(method->AsLocalVariable());
        }

        return;
    }

    if (member->IsTSSignatureDeclaration()) {
        signatureDeclarations.push_back(member->AsTSSignatureDeclaration());
        return;
    }

    ASSERT(member->IsTSIndexSignature());
    indexDeclarations.push_back(member->AsTSIndexSignature());
}

void TSChecker::ResolveSignaturesOfObjectType(ObjectType *type,
                                              ArenaVector<ir::TSSignatureDeclaration *> &signatureDeclarations)
{
    for (auto *it : signatureDeclarations) {
        Type *placeholderObj = it->Check(this);

        if (it->AsTSSignatureDeclaration()->Kind() ==
            ir::TSSignatureDeclaration::TSSignatureDeclarationKind::CALL_SIGNATURE) {
            type->AddCallSignature(placeholderObj->AsObjectType()->CallSignatures()[0]);
            continue;
        }

        type->AddConstructSignature(placeholderObj->AsObjectType()->ConstructSignatures()[0]);
    }
}
void TSChecker::ResolveIndexInfosOfObjectType(ObjectType *type, ArenaVector<ir::TSIndexSignature *> &indexDeclarations)
{
    for (auto *it : indexDeclarations) {
        Type *placeholderObj = it->Check(this);

        if (it->AsTSIndexSignature()->Kind() == ir::TSIndexSignature::TSIndexSignatureKind::NUMBER) {
            IndexInfo *numberInfo = placeholderObj->AsObjectType()->NumberIndexInfo();

            if (type->NumberIndexInfo() != nullptr) {
                ThrowTypeError("Duplicated index signature for type 'number'", it->Start());
            }

            type->Desc()->numberIndexInfo = numberInfo;
            continue;
        }

        IndexInfo *stringInfo = placeholderObj->AsObjectType()->StringIndexInfo();

        if (type->StringIndexInfo() != nullptr) {
            ThrowTypeError("Duplicated index signature for type 'string'", it->Start());
        }

        type->Desc()->stringIndexInfo = stringInfo;
    }
}

varbinder::Variable *TSChecker::GetPropertyOfType(Type *type, const util::StringView &name, bool getPartial,
                                                  varbinder::VariableFlags propagateFlags)
{
    if (type->IsObjectType()) {
        ResolveObjectTypeMembers(type->AsObjectType());
        return type->AsObjectType()->GetProperty(name, true);
    }

    if (type->IsUnionType()) {
        return GetPropertyOfUnionType(type->AsUnionType(), name, getPartial, propagateFlags);
    }

    return nullptr;
}

varbinder::Variable *TSChecker::GetPropertyOfUnionType(UnionType *type, const util::StringView &name, bool getPartial,
                                                       varbinder::VariableFlags propagateFlags)
{
    auto found = type->CachedSyntheticProperties().find(name);

    if (found != type->CachedSyntheticProperties().end()) {
        return found->second;
    }

    varbinder::VariableFlags flags = varbinder::VariableFlags::PROPERTY;
    UnionType::ConstituentsT collectedTypes(Allocator()->Adapter());

    for (auto *it : type->ConstituentTypes()) {
        varbinder::Variable *prop = GetPropertyOfType(it, name);

        if (prop == nullptr) {
            if (it->IsArrayType()) {
                collectedTypes.push_back(it->AsArrayType()->ElementType());
                continue;
            }

            if (!it->IsObjectType()) {
                if (getPartial) {
                    continue;
                }

                return nullptr;
            }

            CObjectType *objType = it->AsObjectType();

            if (objType->StringIndexInfo() == nullptr) {
                if (getPartial) {
                    continue;
                }

                return nullptr;
            }

            collectedTypes.push_back(objType->StringIndexInfo()->GetType());
            continue;
        }

        prop->AddFlag(propagateFlags);

        if (prop->HasFlag(varbinder::VariableFlags::OPTIONAL)) {
            flags |= varbinder::VariableFlags::OPTIONAL;
        }

        collectedTypes.push_back(GetTypeOfVariable(prop));
    }

    if (collectedTypes.empty()) {
        return nullptr;
    }

    varbinder::Variable *syntheticProp = varbinder::Scope::CreateVar(Allocator(), name, flags, nullptr);
    syntheticProp->SetTsType(CreateUnionType(std::move(collectedTypes)));
    type->CachedSyntheticProperties().insert({name, syntheticProp});
    return syntheticProp;
}

Type *TSChecker::CheckComputedPropertyName(ir::Expression *key)
{
    if (key->TsType() != nullptr) {
        return key->TsType();
    }

    Type *keyType = key->Check(this);

    if (!keyType->HasTypeFlag(TypeFlag::STRING_LIKE | TypeFlag::NUMBER_LIKE)) {
        ThrowTypeError(
            "A computed property name in a type literal must refer to an expression whose type is a literal "
            "type "
            "or a 'unique symbol' type",
            key->Start());
    }

    key->SetTsType(keyType);
    return keyType;
}

IndexInfo *TSChecker::GetApplicableIndexInfo(Type *type, CheckerType *indexType)
{
    ResolveStructuredTypeMembers(type);
    bool getNumberInfo = indexType->HasTypeFlag(TypeFlag::NUMBER_LIKE);

    if (type->IsObjectType()) {
        if (getNumberInfo) {
            return type->AsObjectType()->NumberIndexInfo();
        }

        return type->AsObjectType()->StringIndexInfo();
    }

    if (type->IsUnionType()) {
        ASSERT(type->AsUnionType()->MergedObjectType());

        if (getNumberInfo) {
            return type->AsUnionType()->MergedObjectType()->NumberIndexInfo();
        }

        return type->AsUnionType()->MergedObjectType()->StringIndexInfo();
    }

    return nullptr;
}

Type *TSChecker::GetPropertyTypeForIndexType(Type *type, CheckerType *indexType)
{
    if (type->IsArrayType()) {
        return type->AsArrayType()->ElementType();
    }

    if (indexType->HasTypeFlag(TypeFlag::STRING_LITERAL | TypeFlag::NUMBER_LITERAL)) {
        varbinder::Variable *prop = nullptr;

        if (indexType->IsStringLiteralType()) {
            prop = GetPropertyOfType(type, indexType->AsStringLiteralType()->Value());
        } else {
            util::StringView propName =
                util::Helpers::ToStringView(Allocator(), indexType->AsNumberLiteralType()->Value());
            prop = GetPropertyOfType(type, propName);
        }

        if (prop != nullptr) {
            Type *propType = GetTypeOfVariable(prop);

            if (prop->HasFlag(varbinder::VariableFlags::READONLY)) {
                propType->AddTypeFlag(TypeFlag::READONLY);
            }

            return propType;
        }
    }

    if (indexType->HasTypeFlag(TypeFlag::STRING_LIKE | TypeFlag::NUMBER_LIKE)) {
        IndexInfo *indexInfo = GetApplicableIndexInfo(type, indexType);

        if (indexInfo != nullptr) {
            Type *indexInfoType = indexInfo->GetType();

            if (indexInfo->Readonly()) {
                indexInfoType->AddTypeFlag(TypeFlag::READONLY);
            }

            return indexInfoType;
        }
    }

    return nullptr;
}

ArenaVector<ObjectType *> TSChecker::GetBaseTypes(InterfaceType *type)
{
    if (type->HasObjectFlag(ObjectFlags::RESOLVED_BASE_TYPES)) {
        return type->Bases();
    }

    ASSERT(type->Variable() && type->Variable()->Declaration()->IsInterfaceDecl());
    varbinder::InterfaceDecl *decl = type->Variable()->Declaration()->AsInterfaceDecl();

    TypeStackElement tse(this, type, {"Type ", type->Name(), " recursively references itself as a base type."},
                         decl->Node()->AsTSInterfaceDeclaration()->Id()->Start());

    for (const auto *declaration : decl->Decls()) {
        if (declaration->Extends().empty()) {
            continue;
        }

        for (auto *extends : declaration->Extends()) {
            Type *baseType = extends->Expr()->GetType(this);

            if (!baseType->HasTypeFlag(TypeFlag::OBJECT | TypeFlag::NON_PRIMITIVE | TypeFlag::ANY)) {
                ThrowTypeError(
                    "An interface can only extend an object type or intersection of object types with statically "
                    "known "
                    "members",
                    extends->Start());
            }

            if (!baseType->IsObjectType()) {
                continue;
            }

            ObjectType *baseObj = baseType->AsObjectType();

            if (baseType == type) {
                ThrowTypeError({"Type ", type->Name(), " recursively references itself as a base type."},
                               decl->Node()->AsTSInterfaceDeclaration()->Id()->Start());
            }

            type->AddBase(baseObj);

            if (!baseObj->IsInterfaceType()) {
                continue;
            }

            ArenaVector<ObjectType *> extendsBases = GetBaseTypes(baseObj->AsInterfaceType());
            for (auto *extendBase : extendsBases) {
                if (extendBase == type) {
                    ThrowTypeError({"Type ", type->Name(), " recursively references itself as a base type."},
                                   decl->Node()->AsTSInterfaceDeclaration()->Id()->Start());
                }
            }
        }
    }

    type->AddObjectFlag(ObjectFlags::RESOLVED_BASE_TYPES);
    return type->Bases();
}

void TSChecker::ResolveDeclaredMembers(InterfaceType *type)
{
    if (type->HasObjectFlag(ObjectFlags::RESOLVED_DECLARED_MEMBERS)) {
        return;
    }

    ASSERT(type->Variable() && type->Variable()->Declaration()->IsInterfaceDecl());
    varbinder::InterfaceDecl *decl = type->Variable()->Declaration()->AsInterfaceDecl();

    ArenaVector<ir::TSSignatureDeclaration *> signatureDeclarations(Allocator()->Adapter());
    ArenaVector<ir::TSIndexSignature *> indexDeclarations(Allocator()->Adapter());

    for (const auto *declaration : decl->Decls()) {
        for (auto *member : declaration->Body()->Body()) {
            ResolvePropertiesOfObjectType(type, member, signatureDeclarations, indexDeclarations, true);
        }

        type->AddObjectFlag(ObjectFlags::RESOLVED_DECLARED_MEMBERS);

        ResolveSignaturesOfObjectType(type, signatureDeclarations);
        ResolveIndexInfosOfObjectType(type, indexDeclarations);
    }
}

Type *TSChecker::GetTypeForProperty(ir::Property *prop)
{
    if (prop->IsAccessor()) {
        checker::Type *funcType = prop->Value()->Check(this);

        if (prop->Kind() == ir::PropertyKind::SET) {
            return GlobalAnyType();
        }

        ASSERT(funcType->IsObjectType() && funcType->AsObjectType()->IsFunctionType());
        return funcType->AsObjectType()->CallSignatures()[0]->ReturnType();
    }

    if (prop->IsShorthand()) {
        return prop->Key()->Check(this);
    }

    return prop->Value()->Check(this);
}

varbinder::VariableFlags TSChecker::GetFlagsForProperty(const ir::Property *prop)
{
    if (!prop->IsMethod()) {
        return varbinder::VariableFlags::PROPERTY;
    }

    varbinder::VariableFlags propFlags = varbinder::VariableFlags::METHOD;

    if (prop->IsAccessor() && prop->Kind() == ir::PropertyKind::GET) {
        propFlags |= varbinder::VariableFlags::READONLY;
    }

    return propFlags;
}

const util::StringView &TSChecker::GetPropertyName(const ir::Expression *key)
{
    if (key->IsIdentifier()) {
        return key->AsIdentifier()->Name();
    }

    if (key->IsStringLiteral()) {
        return key->AsStringLiteral()->Str();
    }

    ASSERT(key->IsNumberLiteral());
    return key->AsNumberLiteral()->Str();
}

void TSChecker::CheckSpreadElement(ObjectDescriptor *desc, ir::SpreadElement *spread,
                                   const std::unordered_map<util::StringView, lexer::SourcePosition> &propertiesMap)
{
    checker::Type *const spreadType = spread->Argument()->Check(this);

    // NOTE: aszilagyi. handle union of object types
    if (!spreadType->IsObjectType()) {
        ThrowTypeError("Spread types may only be created from object types.", spread->Start());
    }

    for (auto *spreadProp : spreadType->AsObjectType()->Properties()) {
        auto found = propertiesMap.find(spreadProp->Name());
        if (found != propertiesMap.end()) {
            ThrowTypeError({found->first, " is specified more than once, so this usage will be overwritten."},
                           found->second);
        }

        // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
        varbinder::LocalVariable *foundMember = desc->FindProperty(spreadProp->Name());
        if (foundMember != nullptr) {
            foundMember->SetTsType(spreadProp->TsType());
            continue;
        }

        desc->properties.push_back(spreadProp);  // NOLINT(clang-analyzer-core.CallAndMessage)
    }
}

TSChecker::PropertiesCheck TSChecker::CheckObjectExpression(ir::ObjectExpression *expr)
{
    std::unordered_map<util::StringView, lexer::SourcePosition> allPropertiesMap;
    PropertiesCheck result = {ArenaVector<checker::Type *>(Allocator()->Adapter()),
                              ArenaVector<checker::Type *>(Allocator()->Adapter()),
                              Allocator()->New<checker::ObjectDescriptor>(Allocator())};
    for (auto *it : expr->Properties()) {
        if (!it->IsProperty()) {
            ASSERT(it->IsSpreadElement());
            result.seenSpread = true;
            CheckSpreadElement(result.desc, it->AsSpreadElement(), allPropertiesMap);
            continue;
        }
        auto *prop = it->AsProperty();
        if (prop->IsComputed()) {
            checker::Type *computedNameType = CheckComputedPropertyName(prop->Key());

            if (computedNameType->IsNumberType()) {
                result.computedNumberPropTypes.push_back(prop->Value()->Check(this));
                continue;
            }

            if (computedNameType->IsStringType()) {
                result.computedStringPropTypes.push_back(prop->Value()->Check(this));
                continue;
            }
        }

        checker::Type *propType = GetTypeForProperty(prop);
        varbinder::VariableFlags flags = GetFlagsForProperty(prop);
        const util::StringView &propName = GetPropertyName(prop->Key());

        auto *memberVar = varbinder::Scope::CreateVar(Allocator(), propName, flags, it);

        if (HasStatus(checker::CheckerStatus::IN_CONST_CONTEXT)) {
            memberVar->AddFlag(varbinder::VariableFlags::READONLY);
        } else {
            propType = GetBaseTypeOfLiteralType(propType);
        }

        memberVar->SetTsType(propType);

        if (prop->Key()->IsNumberLiteral()) {
            memberVar->AddFlag(varbinder::VariableFlags::NUMERIC_NAME);
        }

        allPropertiesMap.emplace(propName, it->Start());
        varbinder::LocalVariable *foundMember = result.desc->FindProperty(propName);
        if (foundMember != nullptr) {
            foundMember->SetTsType(propType);
            continue;
        }

        result.desc->properties.push_back(memberVar);
    }
    return result;
}

bool TSChecker::ValidateInterfaceMemberRedeclaration(CObjectType *type, varbinder::Variable *prop,
                                                     const lexer::SourcePosition &locInfo)
{
    if (prop->HasFlag(varbinder::VariableFlags::COMPUTED)) {
        return true;
    }

    varbinder::Variable *found = type->GetProperty(prop->Name(), false);

    if (found == nullptr) {
        return true;
    }

    Type *targetType = GetTypeOfVariable(prop);
    Type *sourceType = GetTypeOfVariable(found);
    IsTypeIdenticalTo(targetType, sourceType,
                      {"Subsequent property declarations must have the same type.  Property ", prop->Name(),
                       " must be of type ", sourceType, ", but here has type ", targetType, "."},
                      locInfo);
    return false;
}
}  // namespace panda::es2panda::checker
