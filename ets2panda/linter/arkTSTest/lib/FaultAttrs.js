"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.faultsAttrs = exports.FaultAttributes = void 0;
var _Problems = require("./Problems");
var _ProblemSeverity = require("./ProblemSeverity");
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

class FaultAttributes {
  constructor(cookBookRef, severity = _ProblemSeverity.ProblemSeverity.ERROR) {
    this.cookBookRef = cookBookRef;
    this.severity = severity;
  }
}
exports.FaultAttributes = FaultAttributes;
const faultsAttrs = exports.faultsAttrs = [];
faultsAttrs[_Problems.FaultID.LiteralAsPropertyName] = new FaultAttributes(1);
faultsAttrs[_Problems.FaultID.ComputedPropertyName] = new FaultAttributes(1);
faultsAttrs[_Problems.FaultID.SymbolType] = new FaultAttributes(2);
faultsAttrs[_Problems.FaultID.PrivateIdentifier] = new FaultAttributes(3);
faultsAttrs[_Problems.FaultID.DeclWithDuplicateName] = new FaultAttributes(4);
faultsAttrs[_Problems.FaultID.VarDeclaration] = new FaultAttributes(5);
faultsAttrs[_Problems.FaultID.AnyType] = new FaultAttributes(8);
faultsAttrs[_Problems.FaultID.UnknownType] = new FaultAttributes(8);
faultsAttrs[_Problems.FaultID.CallSignature] = new FaultAttributes(14);
faultsAttrs[_Problems.FaultID.ConstructorType] = new FaultAttributes(15);
faultsAttrs[_Problems.FaultID.MultipleStaticBlocks] = new FaultAttributes(16);
faultsAttrs[_Problems.FaultID.IndexMember] = new FaultAttributes(17);
faultsAttrs[_Problems.FaultID.IntersectionType] = new FaultAttributes(19);
faultsAttrs[_Problems.FaultID.ThisType] = new FaultAttributes(21);
faultsAttrs[_Problems.FaultID.ConditionalType] = new FaultAttributes(22);
faultsAttrs[_Problems.FaultID.ParameterProperties] = new FaultAttributes(25);
faultsAttrs[_Problems.FaultID.ConstructorIface] = new FaultAttributes(27);
faultsAttrs[_Problems.FaultID.IndexedAccessType] = new FaultAttributes(28);
faultsAttrs[_Problems.FaultID.PropertyAccessByIndex] = new FaultAttributes(29);
faultsAttrs[_Problems.FaultID.StructuralIdentity] = new FaultAttributes(30);
faultsAttrs[_Problems.FaultID.GenericCallNoTypeArgs] = new FaultAttributes(34);
faultsAttrs[_Problems.FaultID.ObjectLiteralNoContextType] = new FaultAttributes(38);
faultsAttrs[_Problems.FaultID.ObjectTypeLiteral] = new FaultAttributes(40);
faultsAttrs[_Problems.FaultID.ArrayLiteralNoContextType] = new FaultAttributes(43);
faultsAttrs[_Problems.FaultID.FunctionExpression] = new FaultAttributes(46);
faultsAttrs[_Problems.FaultID.ClassExpression] = new FaultAttributes(50);
faultsAttrs[_Problems.FaultID.ImplementsClass] = new FaultAttributes(51);
faultsAttrs[_Problems.FaultID.MethodReassignment] = new FaultAttributes(52);
faultsAttrs[_Problems.FaultID.TypeAssertion] = new FaultAttributes(53);
faultsAttrs[_Problems.FaultID.JsxElement] = new FaultAttributes(54);
faultsAttrs[_Problems.FaultID.UnaryArithmNotNumber] = new FaultAttributes(55);
faultsAttrs[_Problems.FaultID.DeleteOperator] = new FaultAttributes(59);
faultsAttrs[_Problems.FaultID.TypeQuery] = new FaultAttributes(60);
faultsAttrs[_Problems.FaultID.InstanceofUnsupported] = new FaultAttributes(65);
faultsAttrs[_Problems.FaultID.InOperator] = new FaultAttributes(66);
faultsAttrs[_Problems.FaultID.DestructuringAssignment] = new FaultAttributes(69);
faultsAttrs[_Problems.FaultID.CommaOperator] = new FaultAttributes(71);
faultsAttrs[_Problems.FaultID.DestructuringDeclaration] = new FaultAttributes(74);
faultsAttrs[_Problems.FaultID.CatchWithUnsupportedType] = new FaultAttributes(79);
faultsAttrs[_Problems.FaultID.ForInStatement] = new FaultAttributes(80);
faultsAttrs[_Problems.FaultID.MappedType] = new FaultAttributes(83);
faultsAttrs[_Problems.FaultID.WithStatement] = new FaultAttributes(84);
faultsAttrs[_Problems.FaultID.ThrowStatement] = new FaultAttributes(87);
faultsAttrs[_Problems.FaultID.LimitedReturnTypeInference] = new FaultAttributes(90);
faultsAttrs[_Problems.FaultID.DestructuringParameter] = new FaultAttributes(91);
faultsAttrs[_Problems.FaultID.LocalFunction] = new FaultAttributes(92);
faultsAttrs[_Problems.FaultID.FunctionContainsThis] = new FaultAttributes(93);
faultsAttrs[_Problems.FaultID.GeneratorFunction] = new FaultAttributes(94);
faultsAttrs[_Problems.FaultID.YieldExpression] = new FaultAttributes(94);
faultsAttrs[_Problems.FaultID.IsOperator] = new FaultAttributes(96);
faultsAttrs[_Problems.FaultID.SpreadOperator] = new FaultAttributes(99);
faultsAttrs[_Problems.FaultID.IntefaceExtendDifProps] = new FaultAttributes(102);
faultsAttrs[_Problems.FaultID.InterfaceMerging] = new FaultAttributes(103);
faultsAttrs[_Problems.FaultID.InterfaceExtendsClass] = new FaultAttributes(104);
faultsAttrs[_Problems.FaultID.ConstructorFuncs] = new FaultAttributes(106);
faultsAttrs[_Problems.FaultID.EnumMemberNonConstInit] = new FaultAttributes(111);
faultsAttrs[_Problems.FaultID.EnumMerging] = new FaultAttributes(113);
faultsAttrs[_Problems.FaultID.NamespaceAsObject] = new FaultAttributes(114);
faultsAttrs[_Problems.FaultID.NonDeclarationInNamespace] = new FaultAttributes(116);
faultsAttrs[_Problems.FaultID.ImportAssignment] = new FaultAttributes(121);
faultsAttrs[_Problems.FaultID.ExportAssignment] = new FaultAttributes(126);
faultsAttrs[_Problems.FaultID.ShorthandAmbientModuleDecl] = new FaultAttributes(128);
faultsAttrs[_Problems.FaultID.WildcardsInModuleName] = new FaultAttributes(129);
faultsAttrs[_Problems.FaultID.UMDModuleDefinition] = new FaultAttributes(130);
faultsAttrs[_Problems.FaultID.NewTarget] = new FaultAttributes(132);
faultsAttrs[_Problems.FaultID.DefiniteAssignment] = new FaultAttributes(134, _ProblemSeverity.ProblemSeverity.WARNING);
faultsAttrs[_Problems.FaultID.Prototype] = new FaultAttributes(136);
faultsAttrs[_Problems.FaultID.GlobalThis] = new FaultAttributes(137, _ProblemSeverity.ProblemSeverity.WARNING);
faultsAttrs[_Problems.FaultID.UtilityType] = new FaultAttributes(138);
faultsAttrs[_Problems.FaultID.PropertyDeclOnFunction] = new FaultAttributes(139);
faultsAttrs[_Problems.FaultID.FunctionBind] = new FaultAttributes(140, _ProblemSeverity.ProblemSeverity.WARNING);
faultsAttrs[_Problems.FaultID.ConstAssertion] = new FaultAttributes(142);
faultsAttrs[_Problems.FaultID.ImportAssertion] = new FaultAttributes(143);
faultsAttrs[_Problems.FaultID.LimitedStdLibApi] = new FaultAttributes(144);
faultsAttrs[_Problems.FaultID.StrictDiagnostic] = new FaultAttributes(145);
faultsAttrs[_Problems.FaultID.ErrorSuppression] = new FaultAttributes(146);
faultsAttrs[_Problems.FaultID.ClassAsObject] = new FaultAttributes(149, _ProblemSeverity.ProblemSeverity.WARNING);
faultsAttrs[_Problems.FaultID.ImportAfterStatement] = new FaultAttributes(150);
faultsAttrs[_Problems.FaultID.EsObjectType] = new FaultAttributes(151, _ProblemSeverity.ProblemSeverity.WARNING);
faultsAttrs[_Problems.FaultID.FunctionApplyCall] = new FaultAttributes(152);
faultsAttrs[_Problems.FaultID.SendableClassInheritance] = new FaultAttributes(153);
faultsAttrs[_Problems.FaultID.SendablePropType] = new FaultAttributes(154);
faultsAttrs[_Problems.FaultID.SendableDefiniteAssignment] = new FaultAttributes(155);
faultsAttrs[_Problems.FaultID.SendableGenericTypes] = new FaultAttributes(156);
faultsAttrs[_Problems.FaultID.SendableCapturedVars] = new FaultAttributes(157);
faultsAttrs[_Problems.FaultID.SendableClassDecorator] = new FaultAttributes(158);
faultsAttrs[_Problems.FaultID.SendableObjectInitialization] = new FaultAttributes(159);
faultsAttrs[_Problems.FaultID.SendableComputedPropName] = new FaultAttributes(160);
faultsAttrs[_Problems.FaultID.SendableAsExpr] = new FaultAttributes(161);
faultsAttrs[_Problems.FaultID.SharedNoSideEffectImport] = new FaultAttributes(162);
faultsAttrs[_Problems.FaultID.SharedModuleExports] = new FaultAttributes(163);
faultsAttrs[_Problems.FaultID.SharedModuleNoWildcardExport] = new FaultAttributes(164);
faultsAttrs[_Problems.FaultID.NoTsImportEts] = new FaultAttributes(165);
faultsAttrs[_Problems.FaultID.SendableTypeInheritance] = new FaultAttributes(166);
faultsAttrs[_Problems.FaultID.SendableTypeExported] = new FaultAttributes(167);
faultsAttrs[_Problems.FaultID.NoTsReExportEts] = new FaultAttributes(168);
faultsAttrs[_Problems.FaultID.NoNameSpaceImportEtsToTs] = new FaultAttributes(169);
faultsAttrs[_Problems.FaultID.NoSideEffectImportEtsToTs] = new FaultAttributes(170);
//# sourceMappingURL=FaultAttrs.js.map