"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.faultDesc = void 0;
var _Problems = require("./Problems");
/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

const faultDesc = exports.faultDesc = [];
faultDesc[_Problems.FaultID.AnyType] = '"any" type';
faultDesc[_Problems.FaultID.SymbolType] = '"symbol" type';
faultDesc[_Problems.FaultID.ObjectLiteralNoContextType] = 'Object literals with no context Class or Interface type';
faultDesc[_Problems.FaultID.ArrayLiteralNoContextType] = 'Array literals with no context Array type';
faultDesc[_Problems.FaultID.ComputedPropertyName] = 'Computed properties';
faultDesc[_Problems.FaultID.LiteralAsPropertyName] = 'String or integer literal as property name';
faultDesc[_Problems.FaultID.TypeQuery] = '"typeof" operations';
faultDesc[_Problems.FaultID.IsOperator] = '"is" operations';
faultDesc[_Problems.FaultID.DestructuringParameter] = 'destructuring parameters';
faultDesc[_Problems.FaultID.YieldExpression] = '"yield" operations';
faultDesc[_Problems.FaultID.InterfaceMerging] = 'merging interfaces';
faultDesc[_Problems.FaultID.EnumMerging] = 'merging enums';
faultDesc[_Problems.FaultID.InterfaceExtendsClass] = 'interfaces inherited from classes';
faultDesc[_Problems.FaultID.IndexMember] = 'index members';
faultDesc[_Problems.FaultID.WithStatement] = '"with" statements';
faultDesc[_Problems.FaultID.ThrowStatement] = '"throw" statements with expression of wrong type';
faultDesc[_Problems.FaultID.IndexedAccessType] = 'Indexed access type';
faultDesc[_Problems.FaultID.UnknownType] = '"unknown" type';
faultDesc[_Problems.FaultID.ForInStatement] = '"for-In" statements';
faultDesc[_Problems.FaultID.InOperator] = '"in" operations';
faultDesc[_Problems.FaultID.FunctionExpression] = 'function expressions';
faultDesc[_Problems.FaultID.IntersectionType] = 'intersection types and type literals';
faultDesc[_Problems.FaultID.ObjectTypeLiteral] = 'Object type literals';
faultDesc[_Problems.FaultID.CommaOperator] = 'comma operator';
faultDesc[_Problems.FaultID.LimitedReturnTypeInference] = 'Functions with limited return type inference';
faultDesc[_Problems.FaultID.ClassExpression] = 'Class expressions';
faultDesc[_Problems.FaultID.DestructuringAssignment] = 'Destructuring assignments';
faultDesc[_Problems.FaultID.DestructuringDeclaration] = 'Destructuring variable declarations';
faultDesc[_Problems.FaultID.VarDeclaration] = '"var" declarations';
faultDesc[_Problems.FaultID.CatchWithUnsupportedType] = '"catch" clause with unsupported exception type';
faultDesc[_Problems.FaultID.DeleteOperator] = '"delete" operations';
faultDesc[_Problems.FaultID.DeclWithDuplicateName] = 'Declarations with duplicate name';
faultDesc[_Problems.FaultID.UnaryArithmNotNumber] = 'Unary arithmetics with not-numeric values';
faultDesc[_Problems.FaultID.ConstructorType] = 'Constructor type';
faultDesc[_Problems.FaultID.ConstructorFuncs] = 'Constructor function type is not supported';
faultDesc[_Problems.FaultID.ConstructorIface] = 'Construct signatures are not supported in interfaces';
faultDesc[_Problems.FaultID.CallSignature] = 'Call signatures';
faultDesc[_Problems.FaultID.TypeAssertion] = 'Type assertion expressions';
faultDesc[_Problems.FaultID.PrivateIdentifier] = 'Private identifiers (with "#" prefix)';
faultDesc[_Problems.FaultID.LocalFunction] = 'Local function declarations';
faultDesc[_Problems.FaultID.ConditionalType] = 'Conditional type';
faultDesc[_Problems.FaultID.MappedType] = 'Mapped type';
faultDesc[_Problems.FaultID.NamespaceAsObject] = 'Namespaces used as objects';
faultDesc[_Problems.FaultID.ClassAsObject] = 'Class used as object';
faultDesc[_Problems.FaultID.NonDeclarationInNamespace] = 'Non-declaration statements in namespaces';
faultDesc[_Problems.FaultID.GeneratorFunction] = 'Generator functions';
faultDesc[_Problems.FaultID.FunctionContainsThis] = 'Functions containing "this"';
faultDesc[_Problems.FaultID.PropertyAccessByIndex] = 'property access by index';
faultDesc[_Problems.FaultID.JsxElement] = 'JSX Elements';
faultDesc[_Problems.FaultID.EnumMemberNonConstInit] = 'Enum members with non-constant initializer';
faultDesc[_Problems.FaultID.ImplementsClass] = 'Class type mentioned in "implements" clause';
faultDesc[_Problems.FaultID.MethodReassignment] = 'Access to undefined field';
faultDesc[_Problems.FaultID.MultipleStaticBlocks] = 'Multiple static blocks';
faultDesc[_Problems.FaultID.ThisType] = '"this" type';
faultDesc[_Problems.FaultID.IntefaceExtendDifProps] = 'Extends same properties with different types';
faultDesc[_Problems.FaultID.StructuralIdentity] = 'Use of type structural identity';
faultDesc[_Problems.FaultID.ExportAssignment] = 'Export assignments (export = ..)';
faultDesc[_Problems.FaultID.ImportAssignment] = 'Import assignments (import = ..)';
faultDesc[_Problems.FaultID.GenericCallNoTypeArgs] = 'Generic calls without type arguments';
faultDesc[_Problems.FaultID.ParameterProperties] = 'Parameter properties in constructor';
faultDesc[_Problems.FaultID.InstanceofUnsupported] = 'Left-hand side of "instanceof" is wrong';
faultDesc[_Problems.FaultID.ShorthandAmbientModuleDecl] = 'Shorthand ambient module declaration';
faultDesc[_Problems.FaultID.WildcardsInModuleName] = 'Wildcards in module name';
faultDesc[_Problems.FaultID.UMDModuleDefinition] = 'UMD module definition';
faultDesc[_Problems.FaultID.NewTarget] = '"new.target" meta-property';
faultDesc[_Problems.FaultID.DefiniteAssignment] = 'Definite assignment assertion';
faultDesc[_Problems.FaultID.Prototype] = 'Prototype assignment';
faultDesc[_Problems.FaultID.GlobalThis] = 'Use of globalThis';
faultDesc[_Problems.FaultID.UtilityType] = 'Standard Utility types';
faultDesc[_Problems.FaultID.PropertyDeclOnFunction] = 'Property declaration on function';
faultDesc[_Problems.FaultID.FunctionApplyCall] = 'Invoking methods of function objects';
faultDesc[_Problems.FaultID.FunctionBind] = 'Invoking methods of function objects';
faultDesc[_Problems.FaultID.ConstAssertion] = '"as const" assertion';
faultDesc[_Problems.FaultID.ImportAssertion] = 'Import assertion';
faultDesc[_Problems.FaultID.SpreadOperator] = 'Spread operation';
faultDesc[_Problems.FaultID.LimitedStdLibApi] = 'Limited standard library API';
faultDesc[_Problems.FaultID.ErrorSuppression] = 'Error suppression annotation';
faultDesc[_Problems.FaultID.StrictDiagnostic] = 'Strict diagnostic';
faultDesc[_Problems.FaultID.ImportAfterStatement] = 'Import declaration after other declaration or statement';
faultDesc[_Problems.FaultID.EsObjectType] = 'Restricted "ESObject" type';
faultDesc[_Problems.FaultID.SendableClassInheritance] = 'Sendable class inheritance';
faultDesc[_Problems.FaultID.SendablePropType] = 'Sendable class property';
faultDesc[_Problems.FaultID.SendableDefiniteAssignment] = 'Use of definite assignment assertion in "Sendable" class';
faultDesc[_Problems.FaultID.SendableGenericTypes] = 'Sendable generic types';
faultDesc[_Problems.FaultID.SendableCapturedVars] = 'Sendable class captured variables';
faultDesc[_Problems.FaultID.SendableClassDecorator] = 'Sendable class decorator';
faultDesc[_Problems.FaultID.SendableObjectInitialization] = 'Object literal or array literal is not allowed to initialize a "Sendable" object';
faultDesc[_Problems.FaultID.SendableComputedPropName] = 'Sendable computed property name';
faultDesc[_Problems.FaultID.SendableAsExpr] = 'Sendable as expr';
faultDesc[_Problems.FaultID.SharedNoSideEffectImport] = 'Shared no side effect import';
faultDesc[_Problems.FaultID.SharedModuleExports] = 'Shared module exports';
faultDesc[_Problems.FaultID.SharedModuleNoWildcardExport] = 'Share module no wildcard export';
faultDesc[_Problems.FaultID.NoTsImportEts] = 'No ts Import Ets';
faultDesc[_Problems.FaultID.SendableTypeInheritance] = 'Sendable type inheritance';
faultDesc[_Problems.FaultID.SendableTypeExported] = 'Sendable type exported';
faultDesc[_Problems.FaultID.NoTsReExportEts] = 'No ts re-export ets';
faultDesc[_Problems.FaultID.NoSideEffectImportEtsToTs] = 'No side effect import ets to ts';
faultDesc[_Problems.FaultID.NoNameSpaceImportEtsToTs] = 'No namespace import ets to ts';
//# sourceMappingURL=FaultDesc.js.map