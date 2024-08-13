"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.FaultID = void 0;
/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
let FaultID = exports.FaultID = /*#__PURE__*/function (FaultID) {
  FaultID[FaultID["AnyType"] = 0] = "AnyType";
  FaultID[FaultID["SymbolType"] = 1] = "SymbolType";
  FaultID[FaultID["ObjectLiteralNoContextType"] = 2] = "ObjectLiteralNoContextType";
  FaultID[FaultID["ArrayLiteralNoContextType"] = 3] = "ArrayLiteralNoContextType";
  FaultID[FaultID["ComputedPropertyName"] = 4] = "ComputedPropertyName";
  FaultID[FaultID["LiteralAsPropertyName"] = 5] = "LiteralAsPropertyName";
  FaultID[FaultID["TypeQuery"] = 6] = "TypeQuery";
  FaultID[FaultID["IsOperator"] = 7] = "IsOperator";
  FaultID[FaultID["DestructuringParameter"] = 8] = "DestructuringParameter";
  FaultID[FaultID["YieldExpression"] = 9] = "YieldExpression";
  FaultID[FaultID["InterfaceMerging"] = 10] = "InterfaceMerging";
  FaultID[FaultID["EnumMerging"] = 11] = "EnumMerging";
  FaultID[FaultID["InterfaceExtendsClass"] = 12] = "InterfaceExtendsClass";
  FaultID[FaultID["IndexMember"] = 13] = "IndexMember";
  FaultID[FaultID["WithStatement"] = 14] = "WithStatement";
  FaultID[FaultID["ThrowStatement"] = 15] = "ThrowStatement";
  FaultID[FaultID["IndexedAccessType"] = 16] = "IndexedAccessType";
  FaultID[FaultID["UnknownType"] = 17] = "UnknownType";
  FaultID[FaultID["ForInStatement"] = 18] = "ForInStatement";
  FaultID[FaultID["InOperator"] = 19] = "InOperator";
  FaultID[FaultID["FunctionExpression"] = 20] = "FunctionExpression";
  FaultID[FaultID["IntersectionType"] = 21] = "IntersectionType";
  FaultID[FaultID["ObjectTypeLiteral"] = 22] = "ObjectTypeLiteral";
  FaultID[FaultID["CommaOperator"] = 23] = "CommaOperator";
  FaultID[FaultID["LimitedReturnTypeInference"] = 24] = "LimitedReturnTypeInference";
  FaultID[FaultID["ClassExpression"] = 25] = "ClassExpression";
  FaultID[FaultID["DestructuringAssignment"] = 26] = "DestructuringAssignment";
  FaultID[FaultID["DestructuringDeclaration"] = 27] = "DestructuringDeclaration";
  FaultID[FaultID["VarDeclaration"] = 28] = "VarDeclaration";
  FaultID[FaultID["CatchWithUnsupportedType"] = 29] = "CatchWithUnsupportedType";
  FaultID[FaultID["DeleteOperator"] = 30] = "DeleteOperator";
  FaultID[FaultID["DeclWithDuplicateName"] = 31] = "DeclWithDuplicateName";
  FaultID[FaultID["UnaryArithmNotNumber"] = 32] = "UnaryArithmNotNumber";
  FaultID[FaultID["ConstructorType"] = 33] = "ConstructorType";
  FaultID[FaultID["ConstructorIface"] = 34] = "ConstructorIface";
  FaultID[FaultID["ConstructorFuncs"] = 35] = "ConstructorFuncs";
  FaultID[FaultID["CallSignature"] = 36] = "CallSignature";
  FaultID[FaultID["TypeAssertion"] = 37] = "TypeAssertion";
  FaultID[FaultID["PrivateIdentifier"] = 38] = "PrivateIdentifier";
  FaultID[FaultID["LocalFunction"] = 39] = "LocalFunction";
  FaultID[FaultID["ConditionalType"] = 40] = "ConditionalType";
  FaultID[FaultID["MappedType"] = 41] = "MappedType";
  FaultID[FaultID["NamespaceAsObject"] = 42] = "NamespaceAsObject";
  FaultID[FaultID["ClassAsObject"] = 43] = "ClassAsObject";
  FaultID[FaultID["NonDeclarationInNamespace"] = 44] = "NonDeclarationInNamespace";
  FaultID[FaultID["GeneratorFunction"] = 45] = "GeneratorFunction";
  FaultID[FaultID["FunctionContainsThis"] = 46] = "FunctionContainsThis";
  FaultID[FaultID["PropertyAccessByIndex"] = 47] = "PropertyAccessByIndex";
  FaultID[FaultID["JsxElement"] = 48] = "JsxElement";
  FaultID[FaultID["EnumMemberNonConstInit"] = 49] = "EnumMemberNonConstInit";
  FaultID[FaultID["ImplementsClass"] = 50] = "ImplementsClass";
  FaultID[FaultID["MethodReassignment"] = 51] = "MethodReassignment";
  FaultID[FaultID["MultipleStaticBlocks"] = 52] = "MultipleStaticBlocks";
  FaultID[FaultID["ThisType"] = 53] = "ThisType";
  FaultID[FaultID["IntefaceExtendDifProps"] = 54] = "IntefaceExtendDifProps";
  FaultID[FaultID["StructuralIdentity"] = 55] = "StructuralIdentity";
  FaultID[FaultID["ExportAssignment"] = 56] = "ExportAssignment";
  FaultID[FaultID["ImportAssignment"] = 57] = "ImportAssignment";
  FaultID[FaultID["GenericCallNoTypeArgs"] = 58] = "GenericCallNoTypeArgs";
  FaultID[FaultID["ParameterProperties"] = 59] = "ParameterProperties";
  FaultID[FaultID["InstanceofUnsupported"] = 60] = "InstanceofUnsupported";
  FaultID[FaultID["ShorthandAmbientModuleDecl"] = 61] = "ShorthandAmbientModuleDecl";
  FaultID[FaultID["WildcardsInModuleName"] = 62] = "WildcardsInModuleName";
  FaultID[FaultID["UMDModuleDefinition"] = 63] = "UMDModuleDefinition";
  FaultID[FaultID["NewTarget"] = 64] = "NewTarget";
  FaultID[FaultID["DefiniteAssignment"] = 65] = "DefiniteAssignment";
  FaultID[FaultID["Prototype"] = 66] = "Prototype";
  FaultID[FaultID["GlobalThis"] = 67] = "GlobalThis";
  FaultID[FaultID["UtilityType"] = 68] = "UtilityType";
  FaultID[FaultID["PropertyDeclOnFunction"] = 69] = "PropertyDeclOnFunction";
  FaultID[FaultID["FunctionApplyCall"] = 70] = "FunctionApplyCall";
  FaultID[FaultID["FunctionBind"] = 71] = "FunctionBind";
  FaultID[FaultID["ConstAssertion"] = 72] = "ConstAssertion";
  FaultID[FaultID["ImportAssertion"] = 73] = "ImportAssertion";
  FaultID[FaultID["SpreadOperator"] = 74] = "SpreadOperator";
  FaultID[FaultID["LimitedStdLibApi"] = 75] = "LimitedStdLibApi";
  FaultID[FaultID["ErrorSuppression"] = 76] = "ErrorSuppression";
  FaultID[FaultID["StrictDiagnostic"] = 77] = "StrictDiagnostic";
  FaultID[FaultID["ImportAfterStatement"] = 78] = "ImportAfterStatement";
  FaultID[FaultID["EsObjectType"] = 79] = "EsObjectType";
  FaultID[FaultID["SendableClassInheritance"] = 80] = "SendableClassInheritance";
  FaultID[FaultID["SendablePropType"] = 81] = "SendablePropType";
  FaultID[FaultID["SendableDefiniteAssignment"] = 82] = "SendableDefiniteAssignment";
  FaultID[FaultID["SendableGenericTypes"] = 83] = "SendableGenericTypes";
  FaultID[FaultID["SendableCapturedVars"] = 84] = "SendableCapturedVars";
  FaultID[FaultID["SendableClassDecorator"] = 85] = "SendableClassDecorator";
  FaultID[FaultID["SendableObjectInitialization"] = 86] = "SendableObjectInitialization";
  FaultID[FaultID["SendableComputedPropName"] = 87] = "SendableComputedPropName";
  FaultID[FaultID["SendableAsExpr"] = 88] = "SendableAsExpr";
  FaultID[FaultID["SharedNoSideEffectImport"] = 89] = "SharedNoSideEffectImport";
  FaultID[FaultID["SharedModuleExports"] = 90] = "SharedModuleExports";
  FaultID[FaultID["SharedModuleNoWildcardExport"] = 91] = "SharedModuleNoWildcardExport";
  FaultID[FaultID["NoTsImportEts"] = 92] = "NoTsImportEts";
  FaultID[FaultID["SendableTypeInheritance"] = 93] = "SendableTypeInheritance";
  FaultID[FaultID["SendableTypeExported"] = 94] = "SendableTypeExported";
  FaultID[FaultID["NoTsReExportEts"] = 95] = "NoTsReExportEts";
  FaultID[FaultID["NoNameSpaceImportEtsToTs"] = 96] = "NoNameSpaceImportEtsToTs";
  FaultID[FaultID["NoSideEffectImportEtsToTs"] = 97] = "NoSideEffectImportEtsToTs";
  FaultID[FaultID["LAST_ID"] = 98] = "LAST_ID";
  return FaultID;
}({});
//# sourceMappingURL=Problems.js.map