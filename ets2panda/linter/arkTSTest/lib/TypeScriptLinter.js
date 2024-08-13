"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.TypeScriptLinter = void 0;
exports.consoleLog = consoleLog;
var path = _interopRequireWildcard(require("node:path"));
var ts = _interopRequireWildcard(require("typescript"));
var _CookBookMsg = require("./CookBookMsg");
var _FaultAttrs = require("./FaultAttrs");
var _FaultDesc = require("./FaultDesc");
var _Logger = require("./Logger");
var _ProblemSeverity = require("./ProblemSeverity");
var _Problems = require("./Problems");
var _TypeScriptLinterConfig = require("./TypeScriptLinterConfig");
var _AutofixTitles = require("./autofixes/AutofixTitles");
var _Autofixer = require("./autofixes/Autofixer");
var _TsUtils = require("./utils/TsUtils");
var _AllowedStdSymbolAPI = require("./utils/consts/AllowedStdSymbolAPI");
var _FunctionHasNoReturnErrorCode = require("./utils/consts/FunctionHasNoReturnErrorCode");
var _LimitedStandardUtilityTypes = require("./utils/consts/LimitedStandardUtilityTypes");
var _LimitedStdGlobalFunc = require("./utils/consts/LimitedStdGlobalFunc");
var _LimitedStdObjectAPI = require("./utils/consts/LimitedStdObjectAPI");
var _LimitedStdProxyHandlerAPI = require("./utils/consts/LimitedStdProxyHandlerAPI");
var _LimitedStdReflectAPI = require("./utils/consts/LimitedStdReflectAPI");
var _NonInitializablePropertyDecorators = require("./utils/consts/NonInitializablePropertyDecorators");
var _NonReturnFunctionDecorators = require("./utils/consts/NonReturnFunctionDecorators");
var _PropertyHasNoInitializerErrorCode = require("./utils/consts/PropertyHasNoInitializerErrorCode");
var _ForEachNodeInSubtree = require("./utils/functions/ForEachNodeInSubtree");
var _HasPredecessor = require("./utils/functions/HasPredecessor");
var _IsStdLibrary = require("./utils/functions/IsStdLibrary");
var _IsStruct = require("./utils/functions/IsStruct");
var _LibraryTypeCallDiagnosticChecker = require("./utils/functions/LibraryTypeCallDiagnosticChecker");
var _SupportedStdCallAPI = require("./utils/functions/SupportedStdCallAPI");
var _identiferUseInValueContext = require("./utils/functions/identiferUseInValueContext");
var _isAssignmentOperator = require("./utils/functions/isAssignmentOperator");
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
function _newArrowCheck(n, r) { if (n !== r) throw new TypeError("Cannot instantiate an arrow function"); }
function _defineProperty(e, r, t) { return (r = _toPropertyKey(r)) in e ? Object.defineProperty(e, r, { value: t, enumerable: !0, configurable: !0, writable: !0 }) : e[r] = t, e; }
function _toPropertyKey(t) { var i = _toPrimitive(t, "string"); return "symbol" == typeof i ? i : i + ""; }
function _toPrimitive(t, r) { if ("object" != typeof t || !t) return t; var e = t[Symbol.toPrimitive]; if (void 0 !== e) { var i = e.call(t, r || "default"); if ("object" != typeof i) return i; throw new TypeError("@@toPrimitive must return a primitive value."); } return ("string" === r ? String : Number)(t); } /*
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
function consoleLog(...args) {
  if (TypeScriptLinter.ideMode) {
    return;
  }
  let outLine = '';
  for (let k = 0; k < args.length; k++) {
    outLine += `${args[k]} `;
  }
  _Logger.Logger.info(outLine);
}
class TypeScriptLinter {
  static initGlobals() {
    TypeScriptLinter.filteredDiagnosticMessages = new Set();
    TypeScriptLinter.sharedModulesCache = new Map();
  }
  initEtsHandlers() {
    /*
     * some syntax elements are ArkTs-specific and are only implemented inside patched
     * compiler, so we initialize those handlers if corresponding properties do exist
     */
    const etsComponentExpression = ts.SyntaxKind.EtsComponentExpression;
    if (etsComponentExpression) {
      this.handlersMap.set(etsComponentExpression, this.handleEtsComponentExpression);
    }
  }
  initCounters() {
    for (let i = 0; i < _Problems.FaultID.LAST_ID; i++) {
      this.nodeCounters[i] = 0;
      this.lineCounters[i] = 0;
    }
  }
  constructor(tsTypeChecker, enableAutofix, useRtLogic, cancellationToken, incrementalLintInfo, tscStrictDiagnostics, reportAutofixCb, isEtsFileCb) {
    this.tsTypeChecker = tsTypeChecker;
    this.enableAutofix = enableAutofix;
    this.useRtLogic = useRtLogic;
    this.cancellationToken = cancellationToken;
    this.incrementalLintInfo = incrementalLintInfo;
    this.tscStrictDiagnostics = tscStrictDiagnostics;
    this.reportAutofixCb = reportAutofixCb;
    this.isEtsFileCb = isEtsFileCb;
    _defineProperty(this, "totalVisitedNodes", 0);
    _defineProperty(this, "nodeCounters", []);
    _defineProperty(this, "lineCounters", []);
    _defineProperty(this, "totalErrorLines", 0);
    _defineProperty(this, "errorLineNumbersString", '');
    _defineProperty(this, "totalWarningLines", 0);
    _defineProperty(this, "warningLineNumbersString", '');
    _defineProperty(this, "problemsInfos", []);
    _defineProperty(this, "tsUtils", void 0);
    _defineProperty(this, "currentErrorLine", void 0);
    _defineProperty(this, "currentWarningLine", void 0);
    _defineProperty(this, "walkedComments", void 0);
    _defineProperty(this, "libraryTypeCallDiagnosticChecker", void 0);
    _defineProperty(this, "supportedStdCallApiChecker", void 0);
    _defineProperty(this, "autofixer", void 0);
    _defineProperty(this, "sourceFile", void 0);
    _defineProperty(this, "handlersMap", new Map([[ts.SyntaxKind.ObjectLiteralExpression, this.handleObjectLiteralExpression], [ts.SyntaxKind.ArrayLiteralExpression, this.handleArrayLiteralExpression], [ts.SyntaxKind.Parameter, this.handleParameter], [ts.SyntaxKind.EnumDeclaration, this.handleEnumDeclaration], [ts.SyntaxKind.InterfaceDeclaration, this.handleInterfaceDeclaration], [ts.SyntaxKind.ThrowStatement, this.handleThrowStatement], [ts.SyntaxKind.ImportClause, this.handleImportClause], [ts.SyntaxKind.ForStatement, this.handleForStatement], [ts.SyntaxKind.ForInStatement, this.handleForInStatement], [ts.SyntaxKind.ForOfStatement, this.handleForOfStatement], [ts.SyntaxKind.ImportDeclaration, this.handleImportDeclaration], [ts.SyntaxKind.PropertyAccessExpression, this.handlePropertyAccessExpression], [ts.SyntaxKind.PropertyDeclaration, this.handlePropertyDeclaration], [ts.SyntaxKind.PropertyAssignment, this.handlePropertyAssignment], [ts.SyntaxKind.PropertySignature, this.handlePropertySignature], [ts.SyntaxKind.FunctionExpression, this.handleFunctionExpression], [ts.SyntaxKind.ArrowFunction, this.handleArrowFunction], [ts.SyntaxKind.CatchClause, this.handleCatchClause], [ts.SyntaxKind.FunctionDeclaration, this.handleFunctionDeclaration], [ts.SyntaxKind.PrefixUnaryExpression, this.handlePrefixUnaryExpression], [ts.SyntaxKind.BinaryExpression, this.handleBinaryExpression], [ts.SyntaxKind.VariableDeclarationList, this.handleVariableDeclarationList], [ts.SyntaxKind.VariableDeclaration, this.handleVariableDeclaration], [ts.SyntaxKind.ClassDeclaration, this.handleClassDeclaration], [ts.SyntaxKind.ModuleDeclaration, this.handleModuleDeclaration], [ts.SyntaxKind.TypeAliasDeclaration, this.handleTypeAliasDeclaration], [ts.SyntaxKind.ImportSpecifier, this.handleImportSpecifier], [ts.SyntaxKind.NamespaceImport, this.handleNamespaceImport], [ts.SyntaxKind.TypeAssertionExpression, this.handleTypeAssertionExpression], [ts.SyntaxKind.MethodDeclaration, this.handleMethodDeclaration], [ts.SyntaxKind.MethodSignature, this.handleMethodSignature], [ts.SyntaxKind.ClassStaticBlockDeclaration, this.handleClassStaticBlockDeclaration], [ts.SyntaxKind.Identifier, this.handleIdentifier], [ts.SyntaxKind.ElementAccessExpression, this.handleElementAccessExpression], [ts.SyntaxKind.EnumMember, this.handleEnumMember], [ts.SyntaxKind.TypeReference, this.handleTypeReference], [ts.SyntaxKind.ExportAssignment, this.handleExportAssignment], [ts.SyntaxKind.CallExpression, this.handleCallExpression], [ts.SyntaxKind.MetaProperty, this.handleMetaProperty], [ts.SyntaxKind.NewExpression, this.handleNewExpression], [ts.SyntaxKind.AsExpression, this.handleAsExpression], [ts.SyntaxKind.SpreadElement, this.handleSpreadOp], [ts.SyntaxKind.SpreadAssignment, this.handleSpreadOp], [ts.SyntaxKind.GetAccessor, this.handleGetAccessor], [ts.SyntaxKind.SetAccessor, this.handleSetAccessor], [ts.SyntaxKind.ConstructSignature, this.handleConstructSignature], [ts.SyntaxKind.ExpressionWithTypeArguments, this.handleExpressionWithTypeArguments], [ts.SyntaxKind.ComputedPropertyName, this.handleComputedPropertyName], [ts.SyntaxKind.Constructor, this.handleConstructorDeclaration], [ts.SyntaxKind.PrivateIdentifier, this.handlePrivateIdentifier], [ts.SyntaxKind.IndexSignature, this.handleIndexSignature], [ts.SyntaxKind.TypeLiteral, this.handleTypeLiteral], [ts.SyntaxKind.ExportKeyword, this.handleExportKeyword], [ts.SyntaxKind.ExportDeclaration, this.handleExportDeclaration]]));
    _defineProperty(this, "validatedTypesSet", new Set());
    this.tsUtils = new _TsUtils.TsUtils(this.tsTypeChecker, TypeScriptLinter.testMode, TypeScriptLinter.advancedClassChecks, useRtLogic);
    this.currentErrorLine = 0;
    this.currentWarningLine = 0;
    this.walkedComments = new Set();
    this.libraryTypeCallDiagnosticChecker = new _LibraryTypeCallDiagnosticChecker.LibraryTypeCallDiagnosticChecker(TypeScriptLinter.filteredDiagnosticMessages);
    this.supportedStdCallApiChecker = new _SupportedStdCallAPI.SupportedStdCallApiChecker(this.tsUtils, this.tsTypeChecker);
    this.initEtsHandlers();
    this.initCounters();
  }
  getLineAndCharacterOfNode(node) {
    const startPos = _TsUtils.TsUtils.getStartPos(node);
    const {
      line,
      character
    } = this.sourceFile.getLineAndCharacterOfPosition(startPos);
    // TSC counts lines and columns from zero
    return {
      line: line + 1,
      character: character + 1
    };
  }
  incrementCounters(node, faultId, autofix) {
    this.nodeCounters[faultId]++;
    const {
      line,
      character
    } = this.getLineAndCharacterOfNode(node);
    if (TypeScriptLinter.ideMode) {
      this.incrementCountersIdeMode(node, faultId, autofix);
    } else {
      const faultDescr = _FaultDesc.faultDesc[faultId];
      const faultType = _TypeScriptLinterConfig.LinterConfig.tsSyntaxKindNames[node.kind];
      // Logger.info(
      //   `Warning: ${this.sourceFile!.fileName} (${line}, ${character}): ${faultDescr ? faultDescr : faultType}`
      // );
    }
    this.lineCounters[faultId]++;
    switch (_FaultAttrs.faultsAttrs[faultId].severity) {
      case _ProblemSeverity.ProblemSeverity.ERROR:
        {
          this.currentErrorLine = line;
          ++this.totalErrorLines;
          this.errorLineNumbersString += line + ', ';
          break;
        }
      case _ProblemSeverity.ProblemSeverity.WARNING:
        {
          if (line === this.currentWarningLine) {
            break;
          }
          this.currentWarningLine = line;
          ++this.totalWarningLines;
          this.warningLineNumbersString += line + ', ';
          break;
        }
    }
  }
  incrementCountersIdeMode(node, faultId, autofix) {
    if (!TypeScriptLinter.ideMode) {
      return;
    }
    const [startOffset, endOffset] = _TsUtils.TsUtils.getHighlightRange(node, faultId);
    const startPos = this.sourceFile.getLineAndCharacterOfPosition(startOffset);
    const endPos = this.sourceFile.getLineAndCharacterOfPosition(endOffset);
    const faultDescr = _FaultDesc.faultDesc[faultId];
    const faultType = _TypeScriptLinterConfig.LinterConfig.tsSyntaxKindNames[node.kind];
    const cookBookMsgNum = _FaultAttrs.faultsAttrs[faultId] ? _FaultAttrs.faultsAttrs[faultId].cookBookRef : 0;
    const cookBookTg = _CookBookMsg.cookBookTag[cookBookMsgNum];
    const severity = _FaultAttrs.faultsAttrs[faultId]?.severity ?? _ProblemSeverity.ProblemSeverity.ERROR;
    const isMsgNumValid = cookBookMsgNum > 0;
    const badNodeInfo = {
      line: startPos.line + 1,
      column: startPos.character + 1,
      endLine: endPos.line + 1,
      endColumn: endPos.character + 1,
      start: startOffset,
      end: endOffset,
      type: faultType,
      severity: severity,
      problem: _Problems.FaultID[faultId],
      suggest: isMsgNumValid ? _CookBookMsg.cookBookMsg[cookBookMsgNum] : '',
      rule: isMsgNumValid && cookBookTg !== '' ? cookBookTg : faultDescr ? faultDescr : faultType,
      ruleTag: cookBookMsgNum,
      autofix: autofix,
      autofixTitle: isMsgNumValid && autofix !== undefined ? _AutofixTitles.cookBookRefToFixTitle.get(cookBookMsgNum) : undefined
    };
    this.problemsInfos.push(badNodeInfo);
    // problems with autofixes might be collected separately
    if (this.reportAutofixCb && badNodeInfo.autofix) {
      this.reportAutofixCb(badNodeInfo);
    }
  }
  visitSourceFile(sf) {
    var _this = this;
    const callback = function callback(node) {
      _newArrowCheck(this, _this);
      this.totalVisitedNodes++;
      if ((0, _IsStruct.isStructDeclaration)(node)) {
        // early exit via exception if cancellation was requested
        this.cancellationToken?.throwIfCancellationRequested();
      }
      const incrementedType = _TypeScriptLinterConfig.LinterConfig.incrementOnlyTokens.get(node.kind);
      if (incrementedType !== undefined) {
        this.incrementCounters(node, incrementedType);
      } else {
        const handler = this.handlersMap.get(node.kind);
        if (handler !== undefined) {
          /*
           * possibly requested cancellation will be checked in a limited number of handlers
           * checked nodes are selected as construct nodes, similar to how TSC does
           */
          handler.call(this, node);
        }
      }
    }.bind(this);
    const stopCondition = function stopCondition(node) {
      _newArrowCheck(this, _this);
      if (!node) {
        return true;
      }
      if (this.incrementalLintInfo?.shouldSkipCheck(node)) {
        return true;
      }
      // Skip synthetic constructor in Struct declaration.
      if (node.parent && (0, _IsStruct.isStructDeclaration)(node.parent) && ts.isConstructorDeclaration(node)) {
        return true;
      }
      if (_TypeScriptLinterConfig.LinterConfig.terminalTokens.has(node.kind)) {
        return true;
      }
      return false;
    }.bind(this);
    (0, _ForEachNodeInSubtree.forEachNodeInSubtree)(sf, callback, stopCondition);
  }
  countInterfaceExtendsDifferentPropertyTypes(node, prop2type, propName, type) {
    if (type) {
      const methodType = type.getText();
      const propType = prop2type.get(propName);
      if (!propType) {
        prop2type.set(propName, methodType);
      } else if (propType !== methodType) {
        this.incrementCounters(node, _Problems.FaultID.IntefaceExtendDifProps);
      }
    }
  }
  countDeclarationsWithDuplicateName(tsNode, tsDeclNode, tsDeclKind) {
    const symbol = this.tsTypeChecker.getSymbolAtLocation(tsNode);

    /*
     * If specific declaration kind is provided, check against it.
     * Otherwise, use syntax kind of corresponding declaration node.
     */
    if (!!symbol && _TsUtils.TsUtils.symbolHasDuplicateName(symbol, tsDeclKind ?? tsDeclNode.kind)) {
      this.incrementCounters(tsDeclNode, _Problems.FaultID.DeclWithDuplicateName);
    }
  }
  countClassMembersWithDuplicateName(tsClassDecl) {
    for (const currentMember of tsClassDecl.members) {
      if (this.tsUtils.classMemberHasDuplicateName(currentMember, tsClassDecl, false)) {
        this.incrementCounters(currentMember, _Problems.FaultID.DeclWithDuplicateName);
      }
    }
  }
  isPrototypePropertyAccess(tsPropertyAccess, propAccessSym, baseExprSym, baseExprType) {
    if (!(ts.isIdentifier(tsPropertyAccess.name) && tsPropertyAccess.name.text === 'prototype')) {
      return false;
    }

    // #13600: Relax prototype check when expression comes from interop.
    let curPropAccess = tsPropertyAccess;
    while (curPropAccess && ts.isPropertyAccessExpression(curPropAccess)) {
      const baseExprSym = this.tsUtils.trueSymbolAtLocation(curPropAccess.expression);
      if (this.tsUtils.isLibrarySymbol(baseExprSym)) {
        return false;
      }
      curPropAccess = curPropAccess.expression;
    }
    if (ts.isIdentifier(curPropAccess) && curPropAccess.text !== 'prototype') {
      const type = this.tsTypeChecker.getTypeAtLocation(curPropAccess);
      if (_TsUtils.TsUtils.isAnyType(type)) {
        return false;
      }
    }

    // Check if property symbol is 'Prototype'
    if (_TsUtils.TsUtils.isPrototypeSymbol(propAccessSym)) {
      return true;
    }
    // Check if symbol of LHS-expression is Class or Function.
    if (_TsUtils.TsUtils.isTypeSymbol(baseExprSym) || _TsUtils.TsUtils.isFunctionSymbol(baseExprSym)) {
      return true;
    }

    /*
     * Check if type of LHS expression Function type or Any type.
     * The latter check is to cover cases with multiple prototype
     * chain (as the 'Prototype' property should be 'Any' type):
     *      X.prototype.prototype.prototype = ...
     */
    const baseExprTypeNode = this.tsTypeChecker.typeToTypeNode(baseExprType, undefined, ts.NodeBuilderFlags.None);
    return baseExprTypeNode && ts.isFunctionTypeNode(baseExprTypeNode) || _TsUtils.TsUtils.isAnyType(baseExprType);
  }
  interfaceInheritanceLint(node, heritageClauses) {
    for (const hClause of heritageClauses) {
      if (hClause.token !== ts.SyntaxKind.ExtendsKeyword) {
        continue;
      }
      const prop2type = new Map();
      for (const tsTypeExpr of hClause.types) {
        const tsExprType = this.tsTypeChecker.getTypeAtLocation(tsTypeExpr.expression);
        if (tsExprType.isClass()) {
          this.incrementCounters(tsTypeExpr, _Problems.FaultID.InterfaceExtendsClass);
        } else if (tsExprType.isClassOrInterface()) {
          this.lintForInterfaceExtendsDifferentPorpertyTypes(node, tsExprType, prop2type);
        }
      }
    }
  }
  lintForInterfaceExtendsDifferentPorpertyTypes(node, tsExprType, prop2type) {
    const props = tsExprType.getProperties();
    for (const p of props) {
      if (!p.declarations) {
        continue;
      }
      const decl = p.declarations[0];
      const isPropertyDecl = ts.isPropertySignature(decl) || ts.isPropertyDeclaration(decl);
      const isMethodDecl = ts.isMethodSignature(decl) || ts.isMethodDeclaration(decl);
      if (isMethodDecl || isPropertyDecl) {
        this.countInterfaceExtendsDifferentPropertyTypes(node, prop2type, p.name, decl.type);
      }
    }
  }
  handleObjectLiteralExpression(node) {
    const objectLiteralExpr = node;
    // If object literal is a part of destructuring assignment, then don't process it further.
    if (_TsUtils.TsUtils.isDestructuringAssignmentLHS(objectLiteralExpr)) {
      return;
    }
    const objectLiteralType = this.tsTypeChecker.getContextualType(objectLiteralExpr);
    if (objectLiteralType && this.tsUtils.typeContainsSendableClassOrInterface(objectLiteralType)) {
      this.incrementCounters(node, _Problems.FaultID.SendableObjectInitialization);
    } else if (
    // issue 13082: Allow initializing struct instances with object literal.
    !this.tsUtils.isStructObjectInitializer(objectLiteralExpr) && !this.tsUtils.isDynamicLiteralInitializer(objectLiteralExpr) && !this.tsUtils.isObjectLiteralAssignable(objectLiteralType, objectLiteralExpr)) {
      const autofix = this.autofixer?.fixUntypedObjectLiteral(objectLiteralExpr, objectLiteralType);
      this.incrementCounters(node, _Problems.FaultID.ObjectLiteralNoContextType, autofix);
    }
  }
  handleArrayLiteralExpression(node) {
    /*
     * If array literal is a part of destructuring assignment, then
     * don't process it further.
     */
    if (_TsUtils.TsUtils.isDestructuringAssignmentLHS(node)) {
      return;
    }
    const arrayLitNode = node;
    let noContextTypeForArrayLiteral = false;
    const arrayLitType = this.tsTypeChecker.getContextualType(arrayLitNode);
    if (arrayLitType && this.tsUtils.typeContainsSendableClassOrInterface(arrayLitType)) {
      this.incrementCounters(node, _Problems.FaultID.SendableObjectInitialization);
      return;
    }

    /*
     * check that array literal consists of inferrable types
     * e.g. there is no element which is untyped object literals
     */
    const arrayLitElements = arrayLitNode.elements;
    for (const element of arrayLitElements) {
      if (ts.isObjectLiteralExpression(element)) {
        const objectLiteralType = this.tsTypeChecker.getContextualType(element);
        if (!this.tsUtils.isDynamicLiteralInitializer(arrayLitNode) && !this.tsUtils.isObjectLiteralAssignable(objectLiteralType, element)) {
          noContextTypeForArrayLiteral = true;
          break;
        }
      }
    }
    if (noContextTypeForArrayLiteral) {
      this.incrementCounters(node, _Problems.FaultID.ArrayLiteralNoContextType);
    }
  }
  handleParameter(node) {
    var _this2 = this;
    const tsParam = node;
    _TsUtils.TsUtils.getDecoratorsIfInSendableClass(tsParam)?.forEach(function (decorator) {
      _newArrowCheck(this, _this2);
      this.incrementCounters(decorator, _Problems.FaultID.SendableClassDecorator);
    }.bind(this));
    this.handleDeclarationDestructuring(tsParam);
    this.handleDeclarationInferredType(tsParam);
  }
  handleEnumDeclaration(node) {
    const enumNode = node;
    this.countDeclarationsWithDuplicateName(enumNode.name, enumNode);
    const enumSymbol = this.tsUtils.trueSymbolAtLocation(enumNode.name);
    if (!enumSymbol) {
      return;
    }
    const enumDecls = enumSymbol.getDeclarations();
    if (!enumDecls) {
      return;
    }

    /*
     * Since type checker merges all declarations with the same name
     * into one symbol, we need to check that there's more than one
     * enum declaration related to that specific symbol.
     * See 'countDeclarationsWithDuplicateName' method for details.
     */
    let enumDeclCount = 0;
    const enumDeclsInFile = [];
    const nodeSrcFile = enumNode.getSourceFile();
    for (const decl of enumDecls) {
      if (decl.kind === ts.SyntaxKind.EnumDeclaration) {
        if (nodeSrcFile === decl.getSourceFile()) {
          enumDeclsInFile.push(decl);
        }
        enumDeclCount++;
      }
    }
    if (enumDeclCount > 1) {
      const autofix = this.autofixer?.fixEnumMerging(enumSymbol, enumDeclsInFile);
      this.incrementCounters(node, _Problems.FaultID.EnumMerging, autofix);
    }
  }
  handleInterfaceDeclaration(node) {
    // early exit via exception if cancellation was requested
    this.cancellationToken?.throwIfCancellationRequested();
    const interfaceNode = node;
    const iSymbol = this.tsUtils.trueSymbolAtLocation(interfaceNode.name);
    const iDecls = iSymbol ? iSymbol.getDeclarations() : null;
    if (iDecls) {
      /*
       * Since type checker merges all declarations with the same name
       * into one symbol, we need to check that there's more than one
       * interface declaration related to that specific symbol.
       * See 'countDeclarationsWithDuplicateName' method for details.
       */
      let iDeclCount = 0;
      for (const decl of iDecls) {
        if (decl.kind === ts.SyntaxKind.InterfaceDeclaration) {
          iDeclCount++;
        }
      }
      if (iDeclCount > 1) {
        this.incrementCounters(node, _Problems.FaultID.InterfaceMerging);
      }
    }
    if (interfaceNode.heritageClauses) {
      this.interfaceInheritanceLint(node, interfaceNode.heritageClauses);
    }
    this.countDeclarationsWithDuplicateName(interfaceNode.name, interfaceNode);
  }
  handleThrowStatement(node) {
    const throwStmt = node;
    const throwExprType = this.tsTypeChecker.getTypeAtLocation(throwStmt.expression);
    if (!throwExprType.isClassOrInterface() || !this.tsUtils.isOrDerivedFrom(throwExprType, this.tsUtils.isStdErrorType)) {
      this.incrementCounters(node, _Problems.FaultID.ThrowStatement);
    }
  }
  checkForLoopDestructuring(forInit) {
    if (ts.isVariableDeclarationList(forInit) && forInit.declarations.length === 1) {
      const varDecl = forInit.declarations[0];
      if (this.useRtLogic && (ts.isArrayBindingPattern(varDecl.name) || ts.isObjectBindingPattern(varDecl.name))) {
        this.incrementCounters(varDecl, _Problems.FaultID.DestructuringDeclaration);
      }
    }
    if (ts.isArrayLiteralExpression(forInit) || ts.isObjectLiteralExpression(forInit)) {
      this.incrementCounters(forInit, _Problems.FaultID.DestructuringAssignment);
    }
  }
  handleForStatement(node) {
    const tsForStmt = node;
    const tsForInit = tsForStmt.initializer;
    if (tsForInit) {
      this.checkForLoopDestructuring(tsForInit);
    }
  }
  handleForInStatement(node) {
    const tsForInStmt = node;
    const tsForInInit = tsForInStmt.initializer;
    this.checkForLoopDestructuring(tsForInInit);
    this.incrementCounters(node, _Problems.FaultID.ForInStatement);
  }
  handleForOfStatement(node) {
    const tsForOfStmt = node;
    const tsForOfInit = tsForOfStmt.initializer;
    this.checkForLoopDestructuring(tsForOfInit);
  }
  handleImportDeclaration(node) {
    // early exit via exception if cancellation was requested
    this.cancellationToken?.throwIfCancellationRequested();
    const importDeclNode = node;
    for (const stmt of importDeclNode.parent.statements) {
      if (stmt === importDeclNode) {
        break;
      }
      if (!ts.isImportDeclaration(stmt)) {
        this.incrementCounters(node, _Problems.FaultID.ImportAfterStatement);
        break;
      }
    }
    const expr = importDeclNode.moduleSpecifier;
    if (expr.kind === ts.SyntaxKind.StringLiteral) {
      if (importDeclNode.assertClause) {
        this.incrementCounters(importDeclNode.assertClause, _Problems.FaultID.ImportAssertion);
      }
    }

    // handle no side effect import in sendable module
    this.handleSharedModuleNoSideEffectImport(importDeclNode);
  }
  handleSharedModuleNoSideEffectImport(node) {
    // check 'use shared'
    if (TypeScriptLinter.inSharedModule(node) && !node.importClause) {
      this.incrementCounters(node, _Problems.FaultID.SharedNoSideEffectImport);
    }
  }
  static inSharedModule(node) {
    const sourceFile = node.getSourceFile();
    const modulePath = path.normalize(sourceFile.fileName);
    if (TypeScriptLinter.sharedModulesCache.has(modulePath)) {
      return TypeScriptLinter.sharedModulesCache.get(modulePath);
    }
    const isSharedModule = _TsUtils.TsUtils.isSharedModule(sourceFile);
    TypeScriptLinter.sharedModulesCache.set(modulePath, isSharedModule);
    return isSharedModule;
  }
  handlePropertyAccessExpression(node) {
    if (ts.isCallExpression(node.parent) && node === node.parent.expression) {
      return;
    }
    const propertyAccessNode = node;
    const exprSym = this.tsUtils.trueSymbolAtLocation(propertyAccessNode);
    const baseExprSym = this.tsUtils.trueSymbolAtLocation(propertyAccessNode.expression);
    const baseExprType = this.tsTypeChecker.getTypeAtLocation(propertyAccessNode.expression);
    if (this.isPrototypePropertyAccess(propertyAccessNode, exprSym, baseExprSym, baseExprType)) {
      this.incrementCounters(propertyAccessNode.name, _Problems.FaultID.Prototype);
    }
    if (!!exprSym && this.tsUtils.isSymbolAPI(exprSym) && !_AllowedStdSymbolAPI.ALLOWED_STD_SYMBOL_API.includes(exprSym.getName())) {
      this.incrementCounters(propertyAccessNode, _Problems.FaultID.SymbolType);
    }
    if (TypeScriptLinter.advancedClassChecks && this.tsUtils.isClassObjectExpression(propertyAccessNode.expression)) {
      // missing exact rule
      this.incrementCounters(propertyAccessNode.expression, _Problems.FaultID.ClassAsObject);
    }
    if (!!baseExprSym && _TsUtils.TsUtils.symbolHasEsObjectType(baseExprSym)) {
      this.incrementCounters(propertyAccessNode, _Problems.FaultID.EsObjectType);
    }
  }
  handlePropertyDeclaration(node) {
    const propName = node.name;
    if (!!propName && ts.isNumericLiteral(propName)) {
      const autofix = this.autofixer?.fixLiteralAsPropertyNamePropertyName(propName);
      this.incrementCounters(node, _Problems.FaultID.LiteralAsPropertyName, autofix);
    }
    const decorators = ts.getDecorators(node);
    this.filterOutDecoratorsDiagnostics(decorators, this.useRtLogic ? _NonInitializablePropertyDecorators.NON_INITIALIZABLE_PROPERTY_DECORATORS : _NonInitializablePropertyDecorators.NON_INITIALIZABLE_PROPERTY_DECORATORS_TSC, {
      begin: propName.getStart(),
      end: propName.getStart()
    }, _PropertyHasNoInitializerErrorCode.PROPERTY_HAS_NO_INITIALIZER_ERROR_CODE);
    const classDecorators = ts.getDecorators(node.parent);
    const propType = node.type?.getText();
    this.filterOutDecoratorsDiagnostics(classDecorators, _NonInitializablePropertyDecorators.NON_INITIALIZABLE_PROPERTY_CLASS_DECORATORS, {
      begin: propName.getStart(),
      end: propName.getStart()
    }, _PropertyHasNoInitializerErrorCode.PROPERTY_HAS_NO_INITIALIZER_ERROR_CODE, propType);
    this.handleDeclarationInferredType(node);
    this.handleDefiniteAssignmentAssertion(node);
    this.handleSendableClassProperty(node);
  }
  handleSendableClassProperty(node) {
    var _this3 = this;
    const typeNode = node.type;
    if (!typeNode) {
      return;
    }
    const classNode = node.parent;
    if (!ts.isClassDeclaration(classNode) || !_TsUtils.TsUtils.hasSendableDecorator(classNode)) {
      return;
    }
    _TsUtils.TsUtils.getDecoratorsIfInSendableClass(node)?.forEach(function (decorator) {
      _newArrowCheck(this, _this3);
      this.incrementCounters(decorator, _Problems.FaultID.SendableClassDecorator);
    }.bind(this));
    if (!this.tsUtils.isSendableTypeNode(typeNode)) {
      this.incrementCounters(node, _Problems.FaultID.SendablePropType);
    }
  }
  handlePropertyAssignment(node) {
    const propName = node.name;
    if (!(!!propName && ts.isNumericLiteral(propName))) {
      return;
    }

    /*
     * We can use literals as property names only when creating Record or any interop instances.
     * We can also initialize with constant string literals.
     * Assignment with string enum values is handled in handleComputedPropertyName
     */
    let isRecordObjectInitializer = false;
    let isLibraryType = false;
    let isDynamic = false;
    const objectLiteralType = this.tsTypeChecker.getContextualType(node.parent);
    if (objectLiteralType) {
      isRecordObjectInitializer = this.tsUtils.checkTypeSet(objectLiteralType, this.tsUtils.isStdRecordType);
      isLibraryType = this.tsUtils.isLibraryType(objectLiteralType);
    }
    isDynamic = isLibraryType || this.tsUtils.isDynamicLiteralInitializer(node.parent);
    if (!isRecordObjectInitializer && !isDynamic) {
      const autofix = this.autofixer?.fixLiteralAsPropertyNamePropertyAssignment(node);
      this.incrementCounters(node, _Problems.FaultID.LiteralAsPropertyName, autofix);
    }
  }
  handlePropertySignature(node) {
    const propName = node.name;
    if (!!propName && ts.isNumericLiteral(propName)) {
      const autofix = this.autofixer?.fixLiteralAsPropertyNamePropertyName(propName);
      this.incrementCounters(node, _Problems.FaultID.LiteralAsPropertyName, autofix);
    }
    this.handleSendableInterfaceProperty(node);
  }
  handleSendableInterfaceProperty(node) {
    const typeNode = node.type;
    if (!typeNode) {
      return;
    }
    const interfaceNode = node.parent;
    const interfaceNodeType = this.tsTypeChecker.getTypeAtLocation(interfaceNode);
    if (!ts.isInterfaceDeclaration(interfaceNode) || !this.tsUtils.isSendableClassOrInterface(interfaceNodeType)) {
      return;
    }
    if (!this.tsUtils.isSendableTypeNode(typeNode)) {
      this.incrementCounters(node, _Problems.FaultID.SendablePropType);
    }
  }
  filterOutDecoratorsDiagnostics(decorators, expectedDecorators, range, code, propType) {
    var _this4 = this;
    // Filter out non-initializable property decorators from strict diagnostics.
    if (this.tscStrictDiagnostics && this.sourceFile) {
      if (decorators?.some(function (decorator) {
        _newArrowCheck(this, _this4);
        const decoratorName = _TsUtils.TsUtils.getDecoratorName(decorator);
        // special case for property of type CustomDialogController of the @CustomDialog-decorated class
        if (expectedDecorators.includes(_NonInitializablePropertyDecorators.NON_INITIALIZABLE_PROPERTY_CLASS_DECORATORS[0])) {
          return expectedDecorators.includes(decoratorName) && propType === 'CustomDialogController';
        }
        return expectedDecorators.includes(decoratorName);
      }.bind(this))) {
        this.filterOutDiagnostics(range, code);
      }
    }
  }
  filterOutDiagnostics(range, code) {
    var _this5 = this;
    // Filter out strict diagnostics within the given range with the given code.
    if (!this.tscStrictDiagnostics || !this.sourceFile) {
      return;
    }
    const file = path.normalize(this.sourceFile.fileName);
    const tscDiagnostics = this.tscStrictDiagnostics.get(file);
    if (tscDiagnostics) {
      const filteredDiagnostics = tscDiagnostics.filter(function (val) {
        _newArrowCheck(this, _this5);
        if (val.code !== code) {
          return true;
        }
        if (val.start === undefined) {
          return true;
        }
        if (val.start < range.begin) {
          return true;
        }
        if (val.start > range.end) {
          return true;
        }
        return false;
      }.bind(this));
      this.tscStrictDiagnostics.set(file, filteredDiagnostics);
    }
  }
  static checkInRange(rangesToFilter, pos) {
    for (let i = 0; i < rangesToFilter.length; i++) {
      if (pos >= rangesToFilter[i].begin && pos < rangesToFilter[i].end) {
        return false;
      }
    }
    return true;
  }
  filterStrictDiagnostics(filters, diagnosticChecker) {
    var _this6 = this;
    if (!this.tscStrictDiagnostics || !this.sourceFile) {
      return false;
    }
    const file = path.normalize(this.sourceFile.fileName);
    const tscDiagnostics = this.tscStrictDiagnostics.get(file);
    if (!tscDiagnostics) {
      return false;
    }
    const checkDiagnostic = function checkDiagnostic(val) {
      _newArrowCheck(this, _this6);
      const checkInRange = filters[val.code];
      if (!checkInRange) {
        return true;
      }
      if (val.start === undefined || checkInRange(val.start)) {
        return true;
      }
      return diagnosticChecker.checkDiagnosticMessage(val.messageText);
    }.bind(this);
    if (tscDiagnostics.every(checkDiagnostic)) {
      return false;
    }
    this.tscStrictDiagnostics.set(file, tscDiagnostics.filter(checkDiagnostic));
    return true;
  }
  static isClassLikeOrIface(node) {
    return ts.isClassLike(node) || ts.isInterfaceDeclaration(node);
  }
  handleFunctionExpression(node) {
    const funcExpr = node;
    const isGenerator = funcExpr.asteriskToken !== undefined;
    const [hasUnfixableReturnType, newRetTypeNode] = this.handleMissingReturnType(funcExpr);
    const autofix = this.autofixer?.fixFunctionExpression(funcExpr, newRetTypeNode, ts.getModifiers(funcExpr), isGenerator, hasUnfixableReturnType);
    this.incrementCounters(funcExpr, _Problems.FaultID.FunctionExpression, autofix);
    if (isGenerator) {
      this.incrementCounters(funcExpr, _Problems.FaultID.GeneratorFunction);
    }
    if (!(0, _HasPredecessor.hasPredecessor)(funcExpr, TypeScriptLinter.isClassLikeOrIface)) {
      this.reportThisKeywordsInScope(funcExpr.body);
    }
    if (hasUnfixableReturnType) {
      this.incrementCounters(funcExpr, _Problems.FaultID.LimitedReturnTypeInference);
    }
  }
  handleArrowFunction(node) {
    const arrowFunc = node;
    if (!(0, _HasPredecessor.hasPredecessor)(arrowFunc, TypeScriptLinter.isClassLikeOrIface)) {
      this.reportThisKeywordsInScope(arrowFunc.body);
    }
    const contextType = this.tsTypeChecker.getContextualType(arrowFunc);
    if (!(contextType && this.tsUtils.isLibraryType(contextType))) {
      if (!arrowFunc.type) {
        this.handleMissingReturnType(arrowFunc);
      }
    }
  }
  handleFunctionDeclaration(node) {
    // early exit via exception if cancellation was requested
    this.cancellationToken?.throwIfCancellationRequested();
    const tsFunctionDeclaration = node;
    if (!tsFunctionDeclaration.type) {
      this.handleMissingReturnType(tsFunctionDeclaration);
    }
    if (tsFunctionDeclaration.name) {
      this.countDeclarationsWithDuplicateName(tsFunctionDeclaration.name, tsFunctionDeclaration);
    }
    if (tsFunctionDeclaration.body) {
      this.reportThisKeywordsInScope(tsFunctionDeclaration.body);
    }
    const funcDeclParent = tsFunctionDeclaration.parent;
    if (!ts.isSourceFile(funcDeclParent) && !ts.isModuleBlock(funcDeclParent)) {
      const autofix = this.autofixer?.fixNestedFunction(tsFunctionDeclaration);
      this.incrementCounters(tsFunctionDeclaration, _Problems.FaultID.LocalFunction, autofix);
    }
    if (tsFunctionDeclaration.asteriskToken) {
      this.incrementCounters(node, _Problems.FaultID.GeneratorFunction);
    }
  }
  handleMissingReturnType(funcLikeDecl) {
    if (this.useRtLogic && funcLikeDecl.type) {
      return [false, funcLikeDecl.type];
    }

    // Note: Return type can't be inferred for function without body.
    const isSignature = ts.isMethodSignature(funcLikeDecl);
    if (isSignature || !funcLikeDecl.body) {
      // Ambient flag is not exposed, so we apply dirty hack to make it visible
      const isAmbientDeclaration = !!(funcLikeDecl.flags & ts.NodeFlags.Ambient);
      if ((isSignature || isAmbientDeclaration) && !funcLikeDecl.type) {
        this.incrementCounters(funcLikeDecl, _Problems.FaultID.LimitedReturnTypeInference);
      }
      return [false, undefined];
    }
    return this.tryAutofixMissingReturnType(funcLikeDecl);
  }
  tryAutofixMissingReturnType(funcLikeDecl) {
    if (!funcLikeDecl.body) {
      return [false, undefined];
    }
    let autofix;
    let newRetTypeNode;
    const isFuncExpr = ts.isFunctionExpression(funcLikeDecl);

    /*
     * Currently, ArkTS can't infer return type of function, when expression
     * in the return statement is a call to a function or method whose return
     * value type is omitted. In that case, we attempt to prepare an autofix.
     */
    let hasLimitedRetTypeInference = this.hasLimitedTypeInferenceFromReturnExpr(funcLikeDecl.body);
    const tsSignature = this.tsTypeChecker.getSignatureFromDeclaration(funcLikeDecl);
    if (tsSignature) {
      const tsRetType = this.tsTypeChecker.getReturnTypeOfSignature(tsSignature);
      if (!tsRetType || _TsUtils.TsUtils.isUnsupportedType(tsRetType)) {
        hasLimitedRetTypeInference = true;
      } else if (hasLimitedRetTypeInference) {
        newRetTypeNode = this.tsTypeChecker.typeToTypeNode(tsRetType, funcLikeDecl, ts.NodeBuilderFlags.None);
        if (this.autofixer !== undefined && newRetTypeNode && !isFuncExpr) {
          autofix = this.autofixer.fixMissingReturnType(funcLikeDecl, newRetTypeNode);
        }
      }
    }

    /*
     * Don't report here if in function expression context.
     * See handleFunctionExpression for details.
     */
    if (hasLimitedRetTypeInference && !isFuncExpr) {
      this.incrementCounters(funcLikeDecl, _Problems.FaultID.LimitedReturnTypeInference, autofix);
    }
    return [hasLimitedRetTypeInference && !newRetTypeNode, newRetTypeNode];
  }
  hasLimitedTypeInferenceFromReturnExpr(funBody) {
    var _this7 = this;
    let hasLimitedTypeInference = false;
    const callback = function callback(node) {
      _newArrowCheck(this, _this7);
      if (hasLimitedTypeInference) {
        return;
      }
      if (ts.isReturnStatement(node) && node.expression && this.tsUtils.isCallToFunctionWithOmittedReturnType(_TsUtils.TsUtils.unwrapParenthesized(node.expression))) {
        hasLimitedTypeInference = true;
      }
    }.bind(this);
    // Don't traverse other nested function-like declarations.
    const stopCondition = function stopCondition(node) {
      _newArrowCheck(this, _this7);
      return ts.isFunctionDeclaration(node) || ts.isFunctionExpression(node) || ts.isMethodDeclaration(node) || ts.isAccessor(node) || ts.isArrowFunction(node);
    }.bind(this);
    if (ts.isBlock(funBody)) {
      (0, _ForEachNodeInSubtree.forEachNodeInSubtree)(funBody, callback, stopCondition);
    } else {
      const tsExpr = _TsUtils.TsUtils.unwrapParenthesized(funBody);
      hasLimitedTypeInference = this.tsUtils.isCallToFunctionWithOmittedReturnType(tsExpr);
    }
    return hasLimitedTypeInference;
  }
  isValidTypeForUnaryArithmeticOperator(type) {
    const typeFlags = type.getFlags();
    const numberLiteralFlags = ts.TypeFlags.BigIntLiteral | ts.TypeFlags.NumberLiteral;
    const numberLikeFlags = ts.TypeFlags.BigIntLike | ts.TypeFlags.NumberLike;
    const isNumberLike = !!(typeFlags & (numberLiteralFlags | numberLikeFlags));
    const isAllowedNumericType = this.tsUtils.isStdBigIntType(type) || this.tsUtils.isStdNumberType(type);
    return isNumberLike || isAllowedNumericType;
  }
  handlePrefixUnaryExpression(node) {
    const tsUnaryArithm = node;
    const tsUnaryOp = tsUnaryArithm.operator;
    const tsUnaryOperand = tsUnaryArithm.operand;
    if (tsUnaryOp === ts.SyntaxKind.PlusToken || tsUnaryOp === ts.SyntaxKind.MinusToken || tsUnaryOp === ts.SyntaxKind.TildeToken) {
      const tsOperatndType = this.tsTypeChecker.getTypeAtLocation(tsUnaryOperand);
      const isTilde = tsUnaryOp === ts.SyntaxKind.TildeToken;
      const isInvalidTilde = isTilde && ts.isNumericLiteral(tsUnaryOperand) && !this.tsUtils.isIntegerConstantValue(tsUnaryOperand);
      if (!this.isValidTypeForUnaryArithmeticOperator(tsOperatndType) || isInvalidTilde) {
        this.incrementCounters(node, _Problems.FaultID.UnaryArithmNotNumber);
      }
    }
  }
  handleBinaryExpression(node) {
    const tsBinaryExpr = node;
    const tsLhsExpr = tsBinaryExpr.left;
    const tsRhsExpr = tsBinaryExpr.right;
    if ((0, _isAssignmentOperator.isAssignmentOperator)(tsBinaryExpr.operatorToken)) {
      this.processBinaryAssignment(node, tsLhsExpr, tsRhsExpr);
    }
    const leftOperandType = this.tsTypeChecker.getTypeAtLocation(tsLhsExpr);
    const rightOperandType = this.tsTypeChecker.getTypeAtLocation(tsRhsExpr);
    const typeNode = this.tsUtils.getVariableDeclarationTypeNode(tsLhsExpr);
    switch (tsBinaryExpr.operatorToken.kind) {
      // FaultID.BitOpWithWrongType - removed as rule #61
      case ts.SyntaxKind.CommaToken:
        this.processBinaryComma(tsBinaryExpr);
        break;
      case ts.SyntaxKind.InstanceOfKeyword:
        this.processBinaryInstanceOf(node, tsLhsExpr, leftOperandType);
        break;
      case ts.SyntaxKind.InKeyword:
        this.incrementCounters(tsBinaryExpr.operatorToken, _Problems.FaultID.InOperator);
        break;
      case ts.SyntaxKind.EqualsToken:
        if (this.tsUtils.needToDeduceStructuralIdentity(leftOperandType, rightOperandType, tsRhsExpr)) {
          this.incrementCounters(tsBinaryExpr, _Problems.FaultID.StructuralIdentity);
        }
        this.handleEsObjectAssignment(tsBinaryExpr, typeNode, tsRhsExpr);
        break;
      default:
    }
  }
  processBinaryAssignment(node, tsLhsExpr, tsRhsExpr) {
    if (ts.isObjectLiteralExpression(tsLhsExpr)) {
      this.incrementCounters(node, _Problems.FaultID.DestructuringAssignment);
    } else if (ts.isArrayLiteralExpression(tsLhsExpr)) {
      // Array destructuring is allowed only for Arrays/Tuples and without spread operator.
      const rhsType = this.tsTypeChecker.getTypeAtLocation(tsRhsExpr);
      const isArrayOrTuple = this.tsUtils.isOrDerivedFrom(rhsType, this.tsUtils.isArray) || this.tsUtils.isOrDerivedFrom(rhsType, _TsUtils.TsUtils.isTuple);
      const hasNestedObjectDestructuring = _TsUtils.TsUtils.hasNestedObjectDestructuring(tsLhsExpr);
      if (!TypeScriptLinter.useRelaxedRules || !isArrayOrTuple || hasNestedObjectDestructuring || _TsUtils.TsUtils.destructuringAssignmentHasSpreadOperator(tsLhsExpr)) {
        this.incrementCounters(node, _Problems.FaultID.DestructuringAssignment);
      }
    }
    if (ts.isPropertyAccessExpression(tsLhsExpr)) {
      const tsLhsSymbol = this.tsUtils.trueSymbolAtLocation(tsLhsExpr);
      const tsLhsBaseSymbol = this.tsUtils.trueSymbolAtLocation(tsLhsExpr.expression);
      if (tsLhsSymbol && tsLhsSymbol.flags & ts.SymbolFlags.Method) {
        this.incrementCounters(tsLhsExpr, _Problems.FaultID.MethodReassignment);
      }
      if (_TsUtils.TsUtils.isMethodAssignment(tsLhsSymbol) && tsLhsBaseSymbol && (tsLhsBaseSymbol.flags & ts.SymbolFlags.Function) !== 0) {
        this.incrementCounters(tsLhsExpr, _Problems.FaultID.PropertyDeclOnFunction);
      }
    }
  }
  processBinaryComma(tsBinaryExpr) {
    // CommaOpertor is allowed in 'for' statement initalizer and incrementor
    let tsExprNode = tsBinaryExpr;
    let tsParentNode = tsExprNode.parent;
    while (tsParentNode && tsParentNode.kind === ts.SyntaxKind.BinaryExpression) {
      tsExprNode = tsParentNode;
      tsParentNode = tsExprNode.parent;
      if (tsExprNode.operatorToken.kind === ts.SyntaxKind.CommaToken) {
        // Need to return if one comma enclosed in expression with another comma to avoid multiple reports on one line
        return;
      }
    }
    if (tsParentNode && tsParentNode.kind === ts.SyntaxKind.ForStatement) {
      const tsForNode = tsParentNode;
      if (tsExprNode === tsForNode.initializer || tsExprNode === tsForNode.incrementor) {
        return;
      }
    }
    if (tsParentNode && tsParentNode.kind === ts.SyntaxKind.ExpressionStatement) {
      const autofix = this.autofixer?.fixCommaOperator(tsExprNode);
      this.incrementCounters(tsExprNode, _Problems.FaultID.CommaOperator, autofix);
      return;
    }
    this.incrementCounters(tsBinaryExpr, _Problems.FaultID.CommaOperator);
  }
  processBinaryInstanceOf(node, tsLhsExpr, leftOperandType) {
    const leftExpr = _TsUtils.TsUtils.unwrapParenthesized(tsLhsExpr);
    const leftSymbol = this.tsUtils.trueSymbolAtLocation(leftExpr);

    /*
     * In STS, the left-hand side expression may be of any reference type, otherwise
     * a compile-time error occurs. In addition, the left operand in STS cannot be a type.
     */
    if (tsLhsExpr.kind === ts.SyntaxKind.ThisKeyword) {
      return;
    }
    if (_TsUtils.TsUtils.isPrimitiveType(leftOperandType) || ts.isTypeNode(leftExpr) || _TsUtils.TsUtils.isTypeSymbol(leftSymbol)) {
      this.incrementCounters(node, _Problems.FaultID.InstanceofUnsupported);
    }
  }
  handleVariableDeclarationList(node) {
    const varDeclFlags = ts.getCombinedNodeFlags(node);
    if (!(varDeclFlags & (ts.NodeFlags.Let | ts.NodeFlags.Const))) {
      const autofix = this.autofixer?.fixVarDeclaration(node);
      this.incrementCounters(node, _Problems.FaultID.VarDeclaration, autofix);
    }
  }
  handleVariableDeclaration(node) {
    const tsVarDecl = node;
    if (!this.useRtLogic || ts.isVariableDeclarationList(tsVarDecl.parent) && ts.isVariableStatement(tsVarDecl.parent.parent)) {
      this.handleDeclarationDestructuring(tsVarDecl);
    }

    // Check variable declaration for duplicate name.
    this.checkVarDeclForDuplicateNames(tsVarDecl.name);
    if (tsVarDecl.type && tsVarDecl.initializer) {
      const tsVarInit = tsVarDecl.initializer;
      const tsVarType = this.tsTypeChecker.getTypeAtLocation(tsVarDecl.type);
      const tsInitType = this.tsTypeChecker.getTypeAtLocation(tsVarInit);
      if (this.tsUtils.needToDeduceStructuralIdentity(tsVarType, tsInitType, tsVarInit)) {
        this.incrementCounters(tsVarDecl, _Problems.FaultID.StructuralIdentity);
      }
    }
    this.handleEsObjectDelaration(tsVarDecl);
    this.handleDeclarationInferredType(tsVarDecl);
    this.handleDefiniteAssignmentAssertion(tsVarDecl);
  }
  handleDeclarationDestructuring(decl) {
    const faultId = ts.isVariableDeclaration(decl) ? _Problems.FaultID.DestructuringDeclaration : _Problems.FaultID.DestructuringParameter;
    if (ts.isObjectBindingPattern(decl.name)) {
      this.incrementCounters(decl, faultId);
    } else if (ts.isArrayBindingPattern(decl.name)) {
      if (!TypeScriptLinter.useRelaxedRules) {
        this.incrementCounters(decl, faultId);
        return;
      }

      // Array destructuring is allowed only for Arrays/Tuples and without spread operator.
      const rhsType = this.tsTypeChecker.getTypeAtLocation(decl.initializer ?? decl.name);
      const isArrayOrTuple = rhsType && (this.tsUtils.isOrDerivedFrom(rhsType, this.tsUtils.isArray) || this.tsUtils.isOrDerivedFrom(rhsType, _TsUtils.TsUtils.isTuple));
      const hasNestedObjectDestructuring = _TsUtils.TsUtils.hasNestedObjectDestructuring(decl.name);
      if (!isArrayOrTuple || hasNestedObjectDestructuring || _TsUtils.TsUtils.destructuringDeclarationHasSpreadOperator(decl.name)) {
        this.incrementCounters(decl, faultId);
      }
    }
  }
  checkVarDeclForDuplicateNames(tsBindingName) {
    if (ts.isIdentifier(tsBindingName)) {
      // The syntax kind of the declaration is defined here by the parent of 'BindingName' node.
      this.countDeclarationsWithDuplicateName(tsBindingName, tsBindingName, tsBindingName.parent.kind);
      return;
    }
    for (const tsBindingElem of tsBindingName.elements) {
      if (ts.isOmittedExpression(tsBindingElem)) {
        continue;
      }
      this.checkVarDeclForDuplicateNames(tsBindingElem.name);
    }
  }
  handleEsObjectDelaration(node) {
    const isDeclaredESObject = !!node.type && _TsUtils.TsUtils.isEsObjectType(node.type);
    const initalizerTypeNode = node.initializer && this.tsUtils.getVariableDeclarationTypeNode(node.initializer);
    const isInitializedWithESObject = !!initalizerTypeNode && _TsUtils.TsUtils.isEsObjectType(initalizerTypeNode);
    const isLocal = _TsUtils.TsUtils.isInsideBlock(node);
    if ((isDeclaredESObject || isInitializedWithESObject) && !isLocal) {
      this.incrementCounters(node, _Problems.FaultID.EsObjectType);
      return;
    }
    if (node.initializer) {
      this.handleEsObjectAssignment(node, node.type, node.initializer);
    }
  }
  handleEsObjectAssignment(node, nodeDeclType, initializer) {
    const isTypeAnnotated = !!nodeDeclType;
    const isDeclaredESObject = isTypeAnnotated && _TsUtils.TsUtils.isEsObjectType(nodeDeclType);
    const initalizerTypeNode = this.tsUtils.getVariableDeclarationTypeNode(initializer);
    const isInitializedWithESObject = !!initalizerTypeNode && _TsUtils.TsUtils.isEsObjectType(initalizerTypeNode);
    if (isTypeAnnotated && !isDeclaredESObject && isInitializedWithESObject) {
      this.incrementCounters(node, _Problems.FaultID.EsObjectType);
      return;
    }
    if (isDeclaredESObject && !this.tsUtils.isValueAssignableToESObject(initializer)) {
      this.incrementCounters(node, _Problems.FaultID.EsObjectType);
    }
  }
  handleCatchClause(node) {
    const tsCatch = node;

    /*
     * In TS catch clause doesn't permit specification of the exception varible type except 'any' or 'unknown'.
     * It is not compatible with STS 'catch' where the exception variable has to be of type
     * Error or derived from it.
     * So each 'catch' which has explicit type for the exception object goes to problems.
     */
    if (tsCatch.variableDeclaration?.type) {
      const autofix = this.autofixer?.dropTypeOnVarDecl(tsCatch.variableDeclaration);
      this.incrementCounters(node, _Problems.FaultID.CatchWithUnsupportedType, autofix);
    }
  }
  handleClassDeclaration(node) {
    var _this8 = this;
    // early exit via exception if cancellation was requested
    this.cancellationToken?.throwIfCancellationRequested();
    const tsClassDecl = node;
    if (tsClassDecl.name) {
      this.countDeclarationsWithDuplicateName(tsClassDecl.name, tsClassDecl);
    }
    this.countClassMembersWithDuplicateName(tsClassDecl);
    const isSendableClass = _TsUtils.TsUtils.hasSendableDecorator(tsClassDecl);
    if (isSendableClass) {
      _TsUtils.TsUtils.getNonSendableDecorators(tsClassDecl)?.forEach(function (decorator) {
        _newArrowCheck(this, _this8);
        this.incrementCounters(decorator, _Problems.FaultID.SendableClassDecorator);
      }.bind(this));
      tsClassDecl.typeParameters?.forEach(function (typeParamDecl) {
        _newArrowCheck(this, _this8);
        this.checkSendableTypeParameter(typeParamDecl);
      }.bind(this));
    }
    if (tsClassDecl.heritageClauses) {
      for (const hClause of tsClassDecl.heritageClauses) {
        if (!hClause) {
          continue;
        }
        this.checkClassDeclarationHeritageClause(hClause, isSendableClass);
      }
    }

    // Check captured variables for sendable class
    if (isSendableClass) {
      tsClassDecl.members.forEach(function (classMember) {
        _newArrowCheck(this, _this8);
        this.scanCapturedVarsInSendableScope(classMember, tsClassDecl);
      }.bind(this));
    }
    this.processClassStaticBlocks(tsClassDecl);
  }
  checkClassDeclarationHeritageClause(hClause, isSendableClass) {
    for (const tsTypeExpr of hClause.types) {
      /*
       * Always resolve type from 'tsTypeExpr' node, not from 'tsTypeExpr.expression' node,
       * as for the latter, type checker will return incorrect type result for classes in
       * 'extends' clause. Additionally, reduce reference, as mostly type checker returns
       * the TypeReference type objects for classes and interfaces.
       */
      const tsExprType = _TsUtils.TsUtils.reduceReference(this.tsTypeChecker.getTypeAtLocation(tsTypeExpr));
      const isSendableBaseType = this.tsUtils.isSendableClassOrInterface(tsExprType);
      if (tsExprType.isClass() && hClause.token === ts.SyntaxKind.ImplementsKeyword) {
        this.incrementCounters(tsTypeExpr, _Problems.FaultID.ImplementsClass);
      }
      if (!isSendableClass) {
        // Non-Sendable class can not implements sendable interface / extends sendable class
        if (isSendableBaseType) {
          this.incrementCounters(tsTypeExpr, _Problems.FaultID.SendableClassInheritance);
        }
        continue;
      }

      /*
       * Sendable class can implements any interface / extends only sendable class
       * Sendable class can not extends sendable class variable(local / import)
       */
      if (hClause.token === ts.SyntaxKind.ExtendsKeyword) {
        if (!isSendableBaseType) {
          this.incrementCounters(tsTypeExpr, _Problems.FaultID.SendableClassInheritance);
          continue;
        }
        if (!this.isValidSendableClassExtends(tsTypeExpr)) {
          this.incrementCounters(tsTypeExpr, _Problems.FaultID.SendableClassInheritance);
        }
      }
    }
  }
  isValidSendableClassExtends(tsTypeExpr) {
    const expr = tsTypeExpr.expression;
    const sym = this.tsTypeChecker.getSymbolAtLocation(expr);
    if (sym && (sym.flags & ts.SymbolFlags.Class) === 0) {
      // handle non-class situation(local / import)
      if ((sym.flags & ts.SymbolFlags.Alias) !== 0) {
        /*
         * Sendable class can not extends imported sendable class variable
         * Sendable class can extends imported sendable class
         */
        const realSym = this.tsTypeChecker.getAliasedSymbol(sym);
        if (realSym && (realSym.flags & ts.SymbolFlags.Class) === 0) {
          return false;
        }
        return true;
      }
      return false;
    }
    return true;
  }
  checkSendableTypeParameter(typeParamDecl) {
    const defaultTypeNode = typeParamDecl.default;
    if (defaultTypeNode) {
      if (!this.tsUtils.isSendableTypeNode(defaultTypeNode)) {
        this.incrementCounters(defaultTypeNode, _Problems.FaultID.SendableGenericTypes);
      }
    }
  }
  processClassStaticBlocks(classDecl) {
    let staticBlocksCntr = 0;
    const staticBlockNodes = [];
    for (const element of classDecl.members) {
      if (ts.isClassStaticBlockDeclaration(element)) {
        staticBlockNodes[staticBlocksCntr] = element;
        staticBlocksCntr++;
      }
    }
    if (staticBlocksCntr > 1) {
      const autofix = this.autofixer?.fixMultipleStaticBlocks(staticBlockNodes);
      // autofixes for all additional static blocks are the same
      for (let i = 1; i < staticBlocksCntr; i++) {
        this.incrementCounters(staticBlockNodes[i], _Problems.FaultID.MultipleStaticBlocks, autofix);
      }
    }
  }
  handleModuleDeclaration(node) {
    // early exit via exception if cancellation was requested
    this.cancellationToken?.throwIfCancellationRequested();
    const tsModuleDecl = node;
    this.countDeclarationsWithDuplicateName(tsModuleDecl.name, tsModuleDecl);
    const tsModuleBody = tsModuleDecl.body;
    const tsModifiers = ts.getModifiers(tsModuleDecl);
    if (tsModuleBody) {
      if (ts.isModuleBlock(tsModuleBody)) {
        this.handleModuleBlock(tsModuleBody);
      }
    }
    if (!(tsModuleDecl.flags & ts.NodeFlags.Namespace) && _TsUtils.TsUtils.hasModifier(tsModifiers, ts.SyntaxKind.DeclareKeyword)) {
      this.incrementCounters(tsModuleDecl, _Problems.FaultID.ShorthandAmbientModuleDecl);
    }
    if (ts.isStringLiteral(tsModuleDecl.name) && tsModuleDecl.name.text.includes('*')) {
      this.incrementCounters(tsModuleDecl, _Problems.FaultID.WildcardsInModuleName);
    }
  }
  handleModuleBlock(moduleBlock) {
    for (const tsModuleStmt of moduleBlock.statements) {
      switch (tsModuleStmt.kind) {
        case ts.SyntaxKind.VariableStatement:
        case ts.SyntaxKind.FunctionDeclaration:
        case ts.SyntaxKind.ClassDeclaration:
        case ts.SyntaxKind.InterfaceDeclaration:
        case ts.SyntaxKind.TypeAliasDeclaration:
        case ts.SyntaxKind.EnumDeclaration:
        case ts.SyntaxKind.ExportDeclaration:
          break;

        /*
         * Nested namespace declarations are prohibited
         * but there is no cookbook recipe for it!
         */
        case ts.SyntaxKind.ModuleDeclaration:
          break;
        default:
          this.incrementCounters(tsModuleStmt, _Problems.FaultID.NonDeclarationInNamespace);
          break;
      }
    }
  }
  handleTypeAliasDeclaration(node) {
    const tsTypeAlias = node;
    this.countDeclarationsWithDuplicateName(tsTypeAlias.name, tsTypeAlias);
  }
  handleImportClause(node) {
    const tsImportClause = node;
    if (tsImportClause.name) {
      this.countDeclarationsWithDuplicateName(tsImportClause.name, tsImportClause);
    }
  }
  handleImportSpecifier(node) {
    const importSpec = node;
    this.countDeclarationsWithDuplicateName(importSpec.name, importSpec);
  }
  handleNamespaceImport(node) {
    const tsNamespaceImport = node;
    this.countDeclarationsWithDuplicateName(tsNamespaceImport.name, tsNamespaceImport);
  }
  handleTypeAssertionExpression(node) {
    const tsTypeAssertion = node;
    if (tsTypeAssertion.type.getText() === 'const') {
      this.incrementCounters(tsTypeAssertion, _Problems.FaultID.ConstAssertion);
    } else {
      const autofix = this.autofixer?.fixTypeAssertion(tsTypeAssertion);
      this.incrementCounters(node, _Problems.FaultID.TypeAssertion, autofix);
    }
  }
  handleMethodDeclaration(node) {
    var _this9 = this;
    const tsMethodDecl = node;
    _TsUtils.TsUtils.getDecoratorsIfInSendableClass(tsMethodDecl)?.forEach(function (decorator) {
      _newArrowCheck(this, _this9);
      this.incrementCounters(decorator, _Problems.FaultID.SendableClassDecorator);
    }.bind(this));
    let isStatic = false;
    if (tsMethodDecl.modifiers) {
      for (const mod of tsMethodDecl.modifiers) {
        if (mod.kind === ts.SyntaxKind.StaticKeyword) {
          isStatic = true;
          break;
        }
      }
    }
    if (tsMethodDecl.body && isStatic) {
      this.reportThisKeywordsInScope(tsMethodDecl.body);
    }
    if (!tsMethodDecl.type) {
      this.handleMissingReturnType(tsMethodDecl);
    }
    if (tsMethodDecl.asteriskToken) {
      this.incrementCounters(node, _Problems.FaultID.GeneratorFunction);
    }
    this.filterOutDecoratorsDiagnostics(ts.getDecorators(tsMethodDecl), _NonReturnFunctionDecorators.NON_RETURN_FUNCTION_DECORATORS, {
      begin: tsMethodDecl.parameters.end,
      end: tsMethodDecl.body?.getStart() ?? tsMethodDecl.parameters.end
    }, _FunctionHasNoReturnErrorCode.FUNCTION_HAS_NO_RETURN_ERROR_CODE);
  }
  handleMethodSignature(node) {
    const tsMethodSign = node;
    if (!tsMethodSign.type) {
      this.handleMissingReturnType(tsMethodSign);
    }
  }
  handleClassStaticBlockDeclaration(node) {
    const classStaticBlockDecl = node;
    if (!ts.isClassDeclaration(classStaticBlockDecl.parent)) {
      return;
    }
    this.reportThisKeywordsInScope(classStaticBlockDecl.body);
  }
  handleIdentifier(node) {
    const tsIdentifier = node;
    const tsIdentSym = this.tsUtils.trueSymbolAtLocation(tsIdentifier);
    if (!tsIdentSym) {
      return;
    }
    if ((tsIdentSym.flags & ts.SymbolFlags.Module) !== 0 && (tsIdentSym.flags & ts.SymbolFlags.Transient) !== 0 && tsIdentifier.text === 'globalThis') {
      this.incrementCounters(node, _Problems.FaultID.GlobalThis);
    } else {
      this.handleRestrictedValues(tsIdentifier, tsIdentSym);
    }
  }

  // hard-coded alternative to TypeScriptLinter.advancedClassChecks
  isAllowedClassValueContext(tsIdentifier) {
    let ctx = tsIdentifier;
    while (ts.isPropertyAccessExpression(ctx.parent) || ts.isQualifiedName(ctx.parent)) {
      ctx = ctx.parent;
    }
    if (ts.isPropertyAssignment(ctx.parent) && ts.isObjectLiteralExpression(ctx.parent.parent)) {
      ctx = ctx.parent.parent;
    }
    if (ts.isArrowFunction(ctx.parent) && ctx.parent.body === ctx) {
      ctx = ctx.parent;
    }
    if (ts.isCallExpression(ctx.parent) || ts.isNewExpression(ctx.parent)) {
      const callee = ctx.parent.expression;
      const isAny = _TsUtils.TsUtils.isAnyType(this.tsTypeChecker.getTypeAtLocation(callee));
      const isDynamic = isAny || this.tsUtils.hasLibraryType(callee);
      if (callee !== ctx && isDynamic) {
        return true;
      }
    }
    return false;
  }
  handleRestrictedValues(tsIdentifier, tsIdentSym) {
    const illegalValues = ts.SymbolFlags.ConstEnum | ts.SymbolFlags.RegularEnum | ts.SymbolFlags.ValueModule | (TypeScriptLinter.advancedClassChecks ? 0 : ts.SymbolFlags.Class);

    /*
     * If module name is duplicated by another declaration, this increases the possibility
     * of finding a lot of false positives. Thus, do not check further in that case.
     */
    if ((tsIdentSym.flags & ts.SymbolFlags.ValueModule) !== 0) {
      if (!!tsIdentSym && _TsUtils.TsUtils.symbolHasDuplicateName(tsIdentSym, ts.SyntaxKind.ModuleDeclaration)) {
        return;
      }
    }
    if ((tsIdentSym.flags & illegalValues) === 0 || (0, _IsStruct.isStruct)(tsIdentSym) || !(0, _identiferUseInValueContext.identiferUseInValueContext)(tsIdentifier, tsIdentSym)) {
      return;
    }
    if ((tsIdentSym.flags & ts.SymbolFlags.Class) !== 0) {
      if (!TypeScriptLinter.advancedClassChecks && this.isAllowedClassValueContext(tsIdentifier)) {
        return;
      }
    }
    if (tsIdentSym.flags & ts.SymbolFlags.ValueModule) {
      this.incrementCounters(tsIdentifier, _Problems.FaultID.NamespaceAsObject);
    } else {
      // missing EnumAsObject
      this.incrementCounters(tsIdentifier, _Problems.FaultID.ClassAsObject);
    }
  }
  isElementAcessAllowed(type, argType) {
    if (type.isUnion()) {
      for (const t of type.types) {
        if (!this.isElementAcessAllowed(t, argType)) {
          return false;
        }
      }
      return true;
    }
    const typeNode = this.tsTypeChecker.typeToTypeNode(type, undefined, ts.NodeBuilderFlags.None);
    if (this.tsUtils.isArkTSCollectionsArrayLikeType(type)) {
      return this.tsUtils.isNumberLikeType(argType);
    }
    return this.tsUtils.isLibraryType(type) || _TsUtils.TsUtils.isAnyType(type) || this.tsUtils.isOrDerivedFrom(type, this.tsUtils.isArray) || this.tsUtils.isOrDerivedFrom(type, _TsUtils.TsUtils.isTuple) || this.tsUtils.isOrDerivedFrom(type, this.tsUtils.isStdRecordType) || this.tsUtils.isOrDerivedFrom(type, this.tsUtils.isStringType) || this.tsUtils.isOrDerivedFrom(type, this.tsUtils.isStdMapType) || _TsUtils.TsUtils.isIntrinsicObjectType(type) || _TsUtils.TsUtils.isEnumType(type) ||
    // we allow EsObject here beacuse it is reported later using FaultId.EsObjectType
    _TsUtils.TsUtils.isEsObjectType(typeNode);
  }
  handleElementAccessExpression(node) {
    const tsElementAccessExpr = node;
    const tsElementAccessExprSymbol = this.tsUtils.trueSymbolAtLocation(tsElementAccessExpr.expression);
    const tsElemAccessBaseExprType = this.tsUtils.getNonNullableType(this.tsUtils.getTypeOrTypeConstraintAtLocation(tsElementAccessExpr.expression));
    const tsElemAccessArgType = this.tsTypeChecker.getTypeAtLocation(tsElementAccessExpr.argumentExpression);
    if (
    // unnamed types do not have symbol, so need to check that explicitly
    !this.tsUtils.isLibrarySymbol(tsElementAccessExprSymbol) && !ts.isArrayLiteralExpression(tsElementAccessExpr.expression) && !this.isElementAcessAllowed(tsElemAccessBaseExprType, tsElemAccessArgType)) {
      const autofix = this.autofixer?.fixPropertyAccessByIndex(tsElementAccessExpr);
      this.incrementCounters(node, _Problems.FaultID.PropertyAccessByIndex, autofix);
    }
    if (this.tsUtils.hasEsObjectType(tsElementAccessExpr.expression)) {
      this.incrementCounters(node, _Problems.FaultID.EsObjectType);
    }
  }
  handleEnumMember(node) {
    const tsEnumMember = node;
    const tsEnumMemberType = this.tsTypeChecker.getTypeAtLocation(tsEnumMember);
    const constVal = this.tsTypeChecker.getConstantValue(tsEnumMember);
    if (tsEnumMember.initializer && !this.tsUtils.isValidEnumMemberInit(tsEnumMember.initializer)) {
      this.incrementCounters(node, _Problems.FaultID.EnumMemberNonConstInit);
    }
    // check for type - all members should be of same type
    const enumDecl = tsEnumMember.parent;
    const firstEnumMember = enumDecl.members[0];
    const firstEnumMemberType = this.tsTypeChecker.getTypeAtLocation(firstEnumMember);
    const firstElewmVal = this.tsTypeChecker.getConstantValue(firstEnumMember);

    /*
     * each string enum member has its own type
     * so check that value type is string
     */
    if (constVal !== undefined && typeof constVal === 'string' && firstElewmVal !== undefined && typeof firstElewmVal === 'string') {
      return;
    }
    if (constVal !== undefined && typeof constVal === 'number' && firstElewmVal !== undefined && typeof firstElewmVal === 'number') {
      return;
    }
    if (firstEnumMemberType !== tsEnumMemberType) {
      this.incrementCounters(node, _Problems.FaultID.EnumMemberNonConstInit);
    }
  }
  handleExportAssignment(node) {
    const exportAssignment = node;
    if (exportAssignment.isExportEquals) {
      this.incrementCounters(node, _Problems.FaultID.ExportAssignment);
    }
    if (!TypeScriptLinter.inSharedModule(node)) {
      return;
    }
    if (!this.tsUtils.isShareableEntity(exportAssignment.expression)) {
      this.incrementCounters(exportAssignment.expression, _Problems.FaultID.SharedModuleExports);
    }
  }
  handleCallExpression(node) {
    const tsCallExpr = node;
    const calleeSym = this.tsUtils.trueSymbolAtLocation(tsCallExpr.expression);
    const callSignature = this.tsTypeChecker.getResolvedSignature(tsCallExpr);
    this.handleImportCall(tsCallExpr);
    this.handleRequireCall(tsCallExpr);
    if (calleeSym !== undefined) {
      this.handleStdlibAPICall(tsCallExpr, calleeSym);
      this.handleFunctionApplyBindPropCall(tsCallExpr, calleeSym);
      if (_TsUtils.TsUtils.symbolHasEsObjectType(calleeSym)) {
        this.incrementCounters(tsCallExpr, _Problems.FaultID.EsObjectType);
      }
    }
    if (callSignature !== undefined && !this.tsUtils.isLibrarySymbol(calleeSym)) {
      this.handleGenericCallWithNoTypeArgs(tsCallExpr, callSignature);
      this.handleStructIdentAndUndefinedInArgs(tsCallExpr, callSignature);
    }
    this.handleLibraryTypeCall(tsCallExpr);
    if (ts.isPropertyAccessExpression(tsCallExpr.expression) && this.tsUtils.hasEsObjectType(tsCallExpr.expression.expression)) {
      this.incrementCounters(node, _Problems.FaultID.EsObjectType);
    }
  }
  handleEtsComponentExpression(node) {
    // for all the checks we make EtsComponentExpression is compatible with the CallExpression
    const etsComponentExpression = node;
    this.handleLibraryTypeCall(etsComponentExpression);
  }
  handleImportCall(tsCallExpr) {
    if (tsCallExpr.expression.kind === ts.SyntaxKind.ImportKeyword) {
      // relax rule#133 "arkts-no-runtime-import"
      const tsArgs = tsCallExpr.arguments;
      if (tsArgs.length > 1 && ts.isObjectLiteralExpression(tsArgs[1])) {
        for (const tsProp of tsArgs[1].properties) {
          if ((ts.isPropertyAssignment(tsProp) || ts.isShorthandPropertyAssignment(tsProp)) && tsProp.name.getText() === 'assert') {
            this.incrementCounters(tsProp, _Problems.FaultID.ImportAssertion);
            break;
          }
        }
      }
    }
  }
  handleRequireCall(tsCallExpr) {
    if (ts.isIdentifier(tsCallExpr.expression) && tsCallExpr.expression.text === 'require' && ts.isVariableDeclaration(tsCallExpr.parent)) {
      const tsType = this.tsTypeChecker.getTypeAtLocation(tsCallExpr.expression);
      if (_TsUtils.TsUtils.isInterfaceType(tsType) && tsType.symbol.name === 'NodeRequire') {
        this.incrementCounters(tsCallExpr.parent, _Problems.FaultID.ImportAssignment);
      }
    }
  }
  handleGenericCallWithNoTypeArgs(callLikeExpr, callSignature) {
    /*
     * Note: The PR!716 has led to a significant performance degradation.
     * Since initial problem was fixed in a more general way, this change
     * became redundant. Therefore, it was reverted. See #13721 comments
     * for a detailed analysis.
     */
    const tsSyntaxKind = ts.isNewExpression(callLikeExpr) ? ts.SyntaxKind.Constructor : ts.SyntaxKind.FunctionDeclaration;
    const signFlags = ts.NodeBuilderFlags.WriteTypeArgumentsOfSignature | ts.NodeBuilderFlags.IgnoreErrors;
    const signDecl = this.tsTypeChecker.signatureToSignatureDeclaration(callSignature, tsSyntaxKind, undefined, signFlags);
    if (!signDecl?.typeArguments) {
      return;
    }
    const resolvedTypeArgs = signDecl.typeArguments;
    const startTypeArg = callLikeExpr.typeArguments?.length ?? 0;
    for (let i = startTypeArg; i < resolvedTypeArgs.length; ++i) {
      const typeNode = resolvedTypeArgs[i];

      /*
       * if compiler infers 'unknown' type there are 2 possible cases:
       *   1. Compiler unable to infer type from arguments and use 'unknown'
       *   2. Compiler infer 'unknown' from arguments
       * We report error in both cases. It is ok because we cannot use 'unknown'
       * in ArkTS and already have separate check for it.
       */
      if (typeNode.kind === ts.SyntaxKind.UnknownKeyword) {
        this.incrementCounters(callLikeExpr, _Problems.FaultID.GenericCallNoTypeArgs);
        break;
      }
    }
  }
  handleFunctionApplyBindPropCall(tsCallExpr, calleeSym) {
    const exprName = this.tsTypeChecker.getFullyQualifiedName(calleeSym);
    if (TypeScriptLinter.listFunctionApplyCallApis.includes(exprName)) {
      this.incrementCounters(tsCallExpr, _Problems.FaultID.FunctionApplyCall);
    }
    if (TypeScriptLinter.listFunctionBindApis.includes(exprName)) {
      this.incrementCounters(tsCallExpr, _Problems.FaultID.FunctionBind);
    }
  }
  handleStructIdentAndUndefinedInArgs(tsCallOrNewExpr, callSignature) {
    if (!tsCallOrNewExpr.arguments) {
      return;
    }
    for (let argIndex = 0; argIndex < tsCallOrNewExpr.arguments.length; ++argIndex) {
      const tsArg = tsCallOrNewExpr.arguments[argIndex];
      const tsArgType = this.tsTypeChecker.getTypeAtLocation(tsArg);
      if (!tsArgType) {
        continue;
      }
      const paramIndex = argIndex < callSignature.parameters.length ? argIndex : callSignature.parameters.length - 1;
      const tsParamSym = callSignature.parameters[paramIndex];
      if (!tsParamSym) {
        continue;
      }
      const tsParamDecl = tsParamSym.valueDeclaration;
      if (tsParamDecl && ts.isParameter(tsParamDecl)) {
        let tsParamType = this.tsTypeChecker.getTypeOfSymbolAtLocation(tsParamSym, tsParamDecl);
        if (tsParamDecl.dotDotDotToken && _TsUtils.TsUtils.isGenericArrayType(tsParamType) && tsParamType.typeArguments) {
          tsParamType = tsParamType.typeArguments[0];
        }
        if (!tsParamType) {
          continue;
        }
        if (this.tsUtils.needToDeduceStructuralIdentity(tsParamType, tsArgType, tsArg)) {
          this.incrementCounters(tsArg, _Problems.FaultID.StructuralIdentity);
        }
      }
    }
  }
  handleStdlibAPICall(callExpr, calleeSym) {
    const name = calleeSym.getName();
    const parName = this.tsUtils.getParentSymbolName(calleeSym);
    if (parName === undefined) {
      if (_LimitedStdGlobalFunc.LIMITED_STD_GLOBAL_FUNC.includes(name)) {
        this.incrementCounters(callExpr, _Problems.FaultID.LimitedStdLibApi);
        return;
      }
      const escapedName = calleeSym.escapedName;
      if (escapedName === 'Symbol' || escapedName === 'SymbolConstructor') {
        this.incrementCounters(callExpr, _Problems.FaultID.SymbolType);
      }
      return;
    }
    const lookup = TypeScriptLinter.LimitedApis.get(parName);
    if (lookup !== undefined && (lookup.arr === null || lookup.arr.includes(name)) && (!TypeScriptLinter.useRelaxedRules || !this.supportedStdCallApiChecker.isSupportedStdCallAPI(callExpr, parName, name))) {
      this.incrementCounters(callExpr, lookup.fault);
    }
  }
  static findNonFilteringRangesFunctionCalls(callExpr) {
    const args = callExpr.arguments;
    const result = [];
    for (const arg of args) {
      if (ts.isArrowFunction(arg)) {
        const arrowFuncExpr = arg;
        result.push({
          begin: arrowFuncExpr.body.pos,
          end: arrowFuncExpr.body.end
        });
      } else if (ts.isCallExpression(arg)) {
        result.push({
          begin: arg.arguments.pos,
          end: arg.arguments.end
        });
      }
      // there may be other cases
    }
    return result;
  }
  handleLibraryTypeCall(callExpr) {
    var _this10 = this;
    const calleeType = this.tsTypeChecker.getTypeAtLocation(callExpr.expression);
    const inLibCall = this.tsUtils.isLibraryType(calleeType);
    const diagnosticMessages = [];
    this.libraryTypeCallDiagnosticChecker.configure(inLibCall, diagnosticMessages);
    const nonFilteringRanges = TypeScriptLinter.findNonFilteringRangesFunctionCalls(callExpr);
    const rangesToFilter = [];
    if (nonFilteringRanges.length !== 0) {
      const rangesSize = nonFilteringRanges.length;
      rangesToFilter.push({
        begin: callExpr.arguments.pos,
        end: nonFilteringRanges[0].begin
      });
      rangesToFilter.push({
        begin: nonFilteringRanges[rangesSize - 1].end,
        end: callExpr.arguments.end
      });
      for (let i = 0; i < rangesSize - 1; i++) {
        rangesToFilter.push({
          begin: nonFilteringRanges[i].end,
          end: nonFilteringRanges[i + 1].begin
        });
      }
    } else {
      rangesToFilter.push({
        begin: callExpr.arguments.pos,
        end: callExpr.arguments.end
      });
    }
    const hasFiltered = this.filterStrictDiagnostics({
      [_LibraryTypeCallDiagnosticChecker.ARGUMENT_OF_TYPE_0_IS_NOT_ASSIGNABLE_TO_PARAMETER_OF_TYPE_1_ERROR_CODE]: function (pos) {
        _newArrowCheck(this, _this10);
        return TypeScriptLinter.checkInRange(rangesToFilter, pos);
      }.bind(this),
      [_LibraryTypeCallDiagnosticChecker.NO_OVERLOAD_MATCHES_THIS_CALL_ERROR_CODE]: function (pos) {
        _newArrowCheck(this, _this10);
        return TypeScriptLinter.checkInRange(rangesToFilter, pos);
      }.bind(this),
      [_LibraryTypeCallDiagnosticChecker.TYPE_0_IS_NOT_ASSIGNABLE_TO_TYPE_1_ERROR_CODE]: function (pos) {
        _newArrowCheck(this, _this10);
        return TypeScriptLinter.checkInRange(rangesToFilter, pos);
      }.bind(this)
    }, this.libraryTypeCallDiagnosticChecker);
    if (hasFiltered) {
      this.filterOutDiagnostics({
        begin: callExpr.getStart(),
        end: callExpr.getEnd()
      }, _LibraryTypeCallDiagnosticChecker.OBJECT_IS_POSSIBLY_UNDEFINED_ERROR_CODE);
    }
    for (const msgChain of diagnosticMessages) {
      TypeScriptLinter.filteredDiagnosticMessages.add(msgChain);
    }
  }
  handleNewExpression(node) {
    const tsNewExpr = node;
    if (TypeScriptLinter.advancedClassChecks) {
      const calleeExpr = tsNewExpr.expression;
      const calleeType = this.tsTypeChecker.getTypeAtLocation(calleeExpr);
      if (!this.tsUtils.isClassTypeExrepssion(calleeExpr) && !(0, _IsStdLibrary.isStdLibraryType)(calleeType) && !this.tsUtils.isLibraryType(calleeType) && !this.tsUtils.hasEsObjectType(calleeExpr)) {
        // missing exact rule
        this.incrementCounters(calleeExpr, _Problems.FaultID.ClassAsObject);
      }
    }
    const callSignature = this.tsTypeChecker.getResolvedSignature(tsNewExpr);
    if (callSignature !== undefined) {
      this.handleStructIdentAndUndefinedInArgs(tsNewExpr, callSignature);
      this.handleGenericCallWithNoTypeArgs(tsNewExpr, callSignature);
    }
    this.handleSendableGenericTypes(tsNewExpr);
  }
  handleSendableGenericTypes(node) {
    const type = this.tsTypeChecker.getTypeAtLocation(node);
    if (!this.tsUtils.isSendableClassOrInterface(type)) {
      return;
    }
    const typeArgs = node.typeArguments;
    if (!typeArgs || typeArgs.length === 0) {
      return;
    }
    for (const arg of typeArgs) {
      if (!this.tsUtils.isSendableTypeNode(arg)) {
        this.incrementCounters(arg, _Problems.FaultID.SendableGenericTypes);
      }
    }
  }
  handleAsExpression(node) {
    const tsAsExpr = node;
    if (tsAsExpr.type.getText() === 'const') {
      this.incrementCounters(node, _Problems.FaultID.ConstAssertion);
    }
    const targetType = this.tsTypeChecker.getTypeAtLocation(tsAsExpr.type).getNonNullableType();
    const exprType = this.tsTypeChecker.getTypeAtLocation(tsAsExpr.expression).getNonNullableType();
    // check for rule#65:   'number as Number' and 'boolean as Boolean' are disabled
    if (this.tsUtils.isNumberLikeType(exprType) && this.tsUtils.isStdNumberType(targetType) || _TsUtils.TsUtils.isBooleanLikeType(exprType) && this.tsUtils.isStdBooleanType(targetType)) {
      this.incrementCounters(node, _Problems.FaultID.TypeAssertion);
    }
    if (!this.tsUtils.isSendableClassOrInterface(exprType) && !this.tsUtils.isObject(exprType) && !_TsUtils.TsUtils.isAnyType(exprType) && this.tsUtils.isSendableClassOrInterface(targetType)) {
      this.incrementCounters(tsAsExpr, _Problems.FaultID.SendableAsExpr);
    }
  }
  handleTypeReference(node) {
    const typeRef = node;
    const isESObject = _TsUtils.TsUtils.isEsObjectType(typeRef);
    const isPossiblyValidContext = _TsUtils.TsUtils.isEsObjectPossiblyAllowed(typeRef);
    if (isESObject && !isPossiblyValidContext) {
      this.incrementCounters(node, _Problems.FaultID.EsObjectType);
      return;
    }
    const typeName = this.tsUtils.entityNameToString(typeRef.typeName);
    const isStdUtilityType = _LimitedStandardUtilityTypes.LIMITED_STANDARD_UTILITY_TYPES.includes(typeName);
    if (isStdUtilityType) {
      this.incrementCounters(node, _Problems.FaultID.UtilityType);
      return;
    }
    this.checkPartialType(node);
    if (this.tsUtils.isClassNodeReference(typeRef.typeName) && this.tsUtils.isSendableTypeNode(typeRef)) {
      this.checkSendableTypeArguments(typeRef);
    }
  }
  checkPartialType(node) {
    const typeRef = node;
    // Using Partial<T> type is allowed only when its argument type is either Class or Interface.
    const isStdPartial = this.tsUtils.entityNameToString(typeRef.typeName) === 'Partial';
    if (!isStdPartial) {
      return;
    }
    const hasSingleTypeArgument = !!typeRef.typeArguments && typeRef.typeArguments.length === 1;
    let argType;
    if (!this.useRtLogic) {
      const firstTypeArg = !!typeRef.typeArguments && hasSingleTypeArgument && typeRef.typeArguments[0];
      argType = firstTypeArg && this.tsTypeChecker.getTypeFromTypeNode(firstTypeArg);
    } else {
      argType = hasSingleTypeArgument && this.tsTypeChecker.getTypeFromTypeNode(typeRef.typeArguments[0]);
    }
    if (argType && !argType.isClassOrInterface()) {
      this.incrementCounters(node, _Problems.FaultID.UtilityType);
    }
  }
  checkSendableTypeArguments(typeRef) {
    if (typeRef.typeArguments) {
      for (const typeArg of typeRef.typeArguments) {
        if (!this.tsUtils.isSendableTypeNode(typeArg)) {
          this.incrementCounters(typeArg, _Problems.FaultID.SendableGenericTypes);
        }
      }
    }
  }
  handleMetaProperty(node) {
    const tsMetaProperty = node;
    if (tsMetaProperty.name.text === 'target') {
      this.incrementCounters(node, _Problems.FaultID.NewTarget);
    }
  }
  handleSpreadOp(node) {
    /*
     * spread assignment is disabled
     * spread element is allowed only for arrays as rest parameter
     */
    if (ts.isSpreadElement(node)) {
      const spreadExprType = this.tsUtils.getTypeOrTypeConstraintAtLocation(node.expression);
      if (spreadExprType && (this.useRtLogic || ts.isCallLikeExpression(node.parent) || ts.isArrayLiteralExpression(node.parent)) && this.tsUtils.isOrDerivedFrom(spreadExprType, this.tsUtils.isArray)) {
        return;
      }
    }
    this.incrementCounters(node, _Problems.FaultID.SpreadOperator);
  }
  handleConstructSignature(node) {
    switch (node.parent.kind) {
      case ts.SyntaxKind.TypeLiteral:
        this.incrementCounters(node, _Problems.FaultID.ConstructorType);
        break;
      case ts.SyntaxKind.InterfaceDeclaration:
        this.incrementCounters(node, _Problems.FaultID.ConstructorIface);
        break;
      default:
    }
  }
  handleExpressionWithTypeArguments(node) {
    const tsTypeExpr = node;
    const symbol = this.tsUtils.trueSymbolAtLocation(tsTypeExpr.expression);
    if (!!symbol && _TsUtils.TsUtils.isEsObjectSymbol(symbol)) {
      this.incrementCounters(tsTypeExpr, _Problems.FaultID.EsObjectType);
    }
  }
  handleComputedPropertyName(node) {
    const computedProperty = node;
    if (this.isSendableInvalidCompPropName(computedProperty)) {
      this.incrementCounters(node, _Problems.FaultID.SendableComputedPropName);
    } else if (!this.tsUtils.isValidComputedPropertyName(computedProperty, false)) {
      this.incrementCounters(node, _Problems.FaultID.ComputedPropertyName);
    }
  }
  isSendableInvalidCompPropName(compProp) {
    const declNode = compProp.parent?.parent;
    if (declNode && ts.isClassDeclaration(declNode) && _TsUtils.TsUtils.hasSendableDecorator(declNode)) {
      return true;
    } else if (declNode && ts.isInterfaceDeclaration(declNode)) {
      const declNodeType = this.tsTypeChecker.getTypeAtLocation(declNode);
      if (this.tsUtils.isSendableClassOrInterface(declNodeType)) {
        return true;
      }
    }
    return false;
  }
  handleGetAccessor(node) {
    var _this11 = this;
    _TsUtils.TsUtils.getDecoratorsIfInSendableClass(node)?.forEach(function (decorator) {
      _newArrowCheck(this, _this11);
      this.incrementCounters(decorator, _Problems.FaultID.SendableClassDecorator);
    }.bind(this));
  }
  handleSetAccessor(node) {
    var _this12 = this;
    _TsUtils.TsUtils.getDecoratorsIfInSendableClass(node)?.forEach(function (decorator) {
      _newArrowCheck(this, _this12);
      this.incrementCounters(decorator, _Problems.FaultID.SendableClassDecorator);
    }.bind(this));
  }

  /*
   * issue 13987:
   * When variable have no type annotation and no initial value, and 'noImplicitAny'
   * option is enabled, compiler attempts to infer type from variable references:
   * see https://github.com/microsoft/TypeScript/pull/11263.
   * In this case, we still want to report the error, since ArkTS doesn't allow
   * to omit both type annotation and initializer.
   */
  proceedVarPropDeclaration(decl) {
    if ((ts.isVariableDeclaration(decl) && ts.isVariableStatement(decl.parent.parent) || ts.isPropertyDeclaration(decl)) && !decl.initializer) {
      if (ts.isPropertyDeclaration(decl) && this.tsUtils.skipPropertyInferredTypeCheck(decl, this.sourceFile, this.isEtsFileCb)) {
        return true;
      }
      this.incrementCounters(decl, _Problems.FaultID.AnyType);
      return true;
    }
    return undefined;
  }
  handleDeclarationInferredType(decl) {
    // The type is explicitly specified, no need to check inferred type.
    if (decl.type) {
      return;
    }

    /*
     * issue 13161:
     * In TypeScript, the catch clause variable must be 'any' or 'unknown' type. Since
     * ArkTS doesn't support these types, the type for such variable is simply omitted,
     * and we don't report it as an error. See TypeScriptLinter.handleCatchClause()
     * for reference.
     */
    if (ts.isCatchClause(decl.parent)) {
      return;
    }
    // Destructuring declarations are not supported, do not process them.
    if (ts.isArrayBindingPattern(decl.name) || ts.isObjectBindingPattern(decl.name)) {
      return;
    }
    if (this.proceedVarPropDeclaration(decl)) {
      return;
    }
    const type = this.tsTypeChecker.getTypeAtLocation(decl);
    if (type) {
      this.validateDeclInferredType(type, decl);
    }
  }
  handleDefiniteAssignmentAssertion(decl) {
    if (decl.exclamationToken === undefined) {
      return;
    }
    if (decl.kind === ts.SyntaxKind.PropertyDeclaration) {
      const parentDecl = decl.parent;
      if (parentDecl.kind === ts.SyntaxKind.ClassDeclaration && _TsUtils.TsUtils.hasSendableDecorator(parentDecl)) {
        this.incrementCounters(decl, _Problems.FaultID.SendableDefiniteAssignment);
        return;
      }
    }
    this.incrementCounters(decl, _Problems.FaultID.DefiniteAssignment);
  }
  checkAnyOrUnknownChildNode(node) {
    if (node.kind === ts.SyntaxKind.AnyKeyword || node.kind === ts.SyntaxKind.UnknownKeyword) {
      return true;
    }
    for (const child of node.getChildren()) {
      if (this.checkAnyOrUnknownChildNode(child)) {
        return true;
      }
    }
    return false;
  }
  handleInferredObjectreference(type, decl) {
    const typeArgs = this.tsTypeChecker.getTypeArguments(type);
    if (typeArgs) {
      const haveAnyOrUnknownNodes = this.checkAnyOrUnknownChildNode(decl);
      if (!haveAnyOrUnknownNodes) {
        for (const typeArg of typeArgs) {
          this.validateDeclInferredType(typeArg, decl);
        }
      }
    }
  }
  validateDeclInferredType(type, decl) {
    if (type.aliasSymbol !== undefined) {
      return;
    }
    if (_TsUtils.TsUtils.isObjectType(type) && !!(type.objectFlags & ts.ObjectFlags.Reference)) {
      this.handleInferredObjectreference(type, decl);
      return;
    }
    if (this.validatedTypesSet.has(type)) {
      return;
    }
    if (type.isUnion()) {
      this.validatedTypesSet.add(type);
      for (const unionElem of type.types) {
        this.validateDeclInferredType(unionElem, decl);
      }
    }
    if (_TsUtils.TsUtils.isAnyType(type)) {
      this.incrementCounters(decl, _Problems.FaultID.AnyType);
    } else if (_TsUtils.TsUtils.isUnknownType(type)) {
      this.incrementCounters(decl, _Problems.FaultID.UnknownType);
    }
  }
  handleCommentDirectives(sourceFile) {
    /*
     * We use a dirty hack to retrieve list of parsed comment directives by accessing
     * internal properties of SourceFile node.
     */

    // Handle comment directive '@ts-nocheck'
    const pragmas = sourceFile.pragmas;
    if (pragmas && pragmas instanceof Map) {
      const noCheckPragma = pragmas.get('ts-nocheck');
      if (noCheckPragma) {
        /*
         * The value is either a single entry or an array of entries.
         * Wrap up single entry with array to simplify processing.
         */
        const noCheckEntries = Array.isArray(noCheckPragma) ? noCheckPragma : [noCheckPragma];
        for (const entry of noCheckEntries) {
          this.processNoCheckEntry(entry);
        }
      }
    }

    // Handle comment directives '@ts-ignore' and '@ts-expect-error'
    const commentDirectives = sourceFile.commentDirectives;
    if (commentDirectives && Array.isArray(commentDirectives)) {
      for (const directive of commentDirectives) {
        if (directive.range?.pos === undefined || directive.range?.end === undefined) {
          continue;
        }
        const range = directive.range;
        const kind = sourceFile.text.slice(range.pos, range.pos + 2) === '/*' ? ts.SyntaxKind.MultiLineCommentTrivia : ts.SyntaxKind.SingleLineCommentTrivia;
        const commentRange = {
          pos: range.pos,
          end: range.end,
          kind
        };
        this.incrementCounters(commentRange, _Problems.FaultID.ErrorSuppression);
      }
    }
  }
  processNoCheckEntry(entry) {
    if (entry.range?.kind === undefined || entry.range?.pos === undefined || entry.range?.end === undefined) {
      return;
    }
    this.incrementCounters(entry.range, _Problems.FaultID.ErrorSuppression);
  }
  reportThisKeywordsInScope(scope) {
    var _this13 = this;
    const callback = function callback(node) {
      _newArrowCheck(this, _this13);
      if (node.kind === ts.SyntaxKind.ThisKeyword) {
        this.incrementCounters(node, _Problems.FaultID.FunctionContainsThis);
      }
    }.bind(this);
    const stopCondition = function stopCondition(node) {
      _newArrowCheck(this, _this13);
      const isClassLike = ts.isClassDeclaration(node) || ts.isClassExpression(node);
      const isFunctionLike = ts.isFunctionDeclaration(node) || ts.isFunctionExpression(node);
      const isModuleDecl = ts.isModuleDeclaration(node);
      return isClassLike || isFunctionLike || isModuleDecl;
    }.bind(this);
    (0, _ForEachNodeInSubtree.forEachNodeInSubtree)(scope, callback, stopCondition);
  }
  handleConstructorDeclaration(node) {
    var _this14 = this;
    const ctorDecl = node;
    if (ctorDecl.parameters.some(function (x) {
      _newArrowCheck(this, _this14);
      return this.tsUtils.hasAccessModifier(x);
    }.bind(this))) {
      let paramTypes;
      if (ctorDecl.body) {
        paramTypes = this.collectCtorParamTypes(ctorDecl);
      }
      const autofix = this.autofixer?.fixCtorParameterProperties(ctorDecl, paramTypes);
      this.incrementCounters(node, _Problems.FaultID.ParameterProperties, autofix);
    }
  }
  collectCtorParamTypes(ctorDecl) {
    const paramTypes = [];
    for (const param of ctorDecl.parameters) {
      let paramTypeNode = param.type;
      if (!paramTypeNode) {
        const paramType = this.tsTypeChecker.getTypeAtLocation(param);
        paramTypeNode = this.tsTypeChecker.typeToTypeNode(paramType, param, ts.NodeBuilderFlags.None);
      }
      if (!paramTypeNode || !this.tsUtils.isSupportedType(paramTypeNode)) {
        return undefined;
      }
      paramTypes.push(paramTypeNode);
    }
    return paramTypes;
  }
  handlePrivateIdentifier(node) {
    const ident = node;
    const autofix = this.autofixer?.fixPrivateIdentifier(ident);
    this.incrementCounters(node, _Problems.FaultID.PrivateIdentifier, autofix);
  }
  handleIndexSignature(node) {
    if (!this.tsUtils.isAllowedIndexSignature(node)) {
      this.incrementCounters(node, _Problems.FaultID.IndexMember);
    }
  }
  handleTypeLiteral(node) {
    const typeLiteral = node;
    const autofix = this.autofixer?.fixTypeliteral(typeLiteral);
    this.incrementCounters(node, _Problems.FaultID.ObjectTypeLiteral, autofix);
  }
  scanCapturedVarsInSendableScope(startNode, scope) {
    var _this15 = this;
    const callback = function callback(node) {
      _newArrowCheck(this, _this15);
      // Namespace import will introduce closure in the es2abc compiler stage
      if (!ts.isIdentifier(node) || this.checkNamespaceImportVar(node)) {
        return;
      }

      // The "b" of "A.b" should not be checked since it's load from object "A"
      const parent = node.parent;
      if (ts.isPropertyAccessExpression(parent) && parent.name === node) {
        return;
      }
      this.checkLocalDecl(node, scope);
    }.bind(this);
    // Type nodes should not checked because no closure will be introduced
    const stopCondition = function stopCondition(node) {
      _newArrowCheck(this, _this15);
      return ts.isTypeReferenceNode(node);
    }.bind(this);
    (0, _ForEachNodeInSubtree.forEachNodeInSubtree)(startNode, callback, stopCondition);
  }
  checkLocalDecl(node, scope) {
    const trueSym = this.tsUtils.trueSymbolAtLocation(node);
    // Sendable decorator should be used in method of Sendable classes
    if (trueSym === undefined) {
      return;
    }

    // Const enum member will be replaced by the exact value of it, no closure will be introduced
    if (_TsUtils.TsUtils.isConstEnum(trueSym)) {
      return;
    }
    const declarations = trueSym.getDeclarations();
    if (declarations?.length) {
      this.checkLocalDeclWithSendableClosure(node, scope, declarations[0]);
    }
  }
  checkLocalDeclWithSendableClosure(node, scope, decl) {
    const declPosition = decl.getStart();
    if (decl.getSourceFile().fileName !== node.getSourceFile().fileName || declPosition !== undefined && declPosition >= scope.getStart() && declPosition < scope.getEnd()) {
      return;
    }
    if (this.isTopSendableClosure(decl)) {
      return;
    }

    /**
     * The cases in condition will introduce closure if defined in the same file as the Sendable class. The following
     * cases are excluded because they are not allowed in ArkTS:
     * 1. ImportEqualDecalration
     * 2. BindingElement
     */
    if (ts.isVariableDeclaration(decl) || ts.isFunctionDeclaration(decl) || ts.isClassDeclaration(decl) || ts.isInterfaceDeclaration(decl) || ts.isEnumDeclaration(decl) || ts.isModuleDeclaration(decl) || ts.isParameter(decl)) {
      this.incrementCounters(node, _Problems.FaultID.SendableCapturedVars);
    }
  }
  isTopSendableClosure(decl) {
    return ts.isSourceFile(decl.parent) && ts.isClassDeclaration(decl) && this.tsUtils.isSendableClassOrInterface(this.tsTypeChecker.getTypeAtLocation(decl));
  }
  checkNamespaceImportVar(node) {
    // Namespace import cannot be determined by the true symbol
    const sym = this.tsTypeChecker.getSymbolAtLocation(node);
    const decls = sym?.getDeclarations();
    if (decls?.length) {
      if (ts.isNamespaceImport(decls[0])) {
        this.incrementCounters(node, _Problems.FaultID.SendableCapturedVars);
        return true;
      }
    }
    return false;
  }
  lint(sourceFile) {
    _Logger.Logger.info("lint====this.enableAutofix====" + !!this.enableAutofix);
    if (this.enableAutofix) {
      this.autofixer = new _Autofixer.Autofixer(this.tsTypeChecker, this.tsUtils, sourceFile, this.cancellationToken);
    }
    this.walkedComments.clear();
    this.sourceFile = sourceFile;
    this.visitSourceFile(this.sourceFile);
    this.handleCommentDirectives(this.sourceFile);
  }
  handleExportKeyword(node) {
    const parentNode = node.parent;
    if (!TypeScriptLinter.inSharedModule(node) || ts.isModuleBlock(parentNode.parent)) {
      return;
    }
    switch (parentNode.kind) {
      case ts.SyntaxKind.EnumDeclaration:
      case ts.SyntaxKind.InterfaceDeclaration:
      case ts.SyntaxKind.ClassDeclaration:
        if (!this.tsUtils.isShareableType(this.tsTypeChecker.getTypeAtLocation(parentNode))) {
          this.incrementCounters(parentNode.name ?? parentNode, _Problems.FaultID.SharedModuleExports);
        }
        return;
      case ts.SyntaxKind.VariableStatement:
        for (const variableDeclaration of parentNode.declarationList.declarations) {
          if (!this.tsUtils.isShareableEntity(variableDeclaration.name)) {
            this.incrementCounters(variableDeclaration.name, _Problems.FaultID.SharedModuleExports);
          }
        }
        return;
      case ts.SyntaxKind.TypeAliasDeclaration:
        return;
      default:
        this.incrementCounters(parentNode, _Problems.FaultID.SharedModuleExports);
    }
  }
  handleExportDeclaration(node) {
    if (!TypeScriptLinter.inSharedModule(node) || ts.isModuleBlock(node.parent)) {
      return;
    }
    const exportDecl = node;
    if (exportDecl.exportClause === undefined) {
      this.incrementCounters(exportDecl, _Problems.FaultID.SharedModuleNoWildcardExport);
      return;
    }
    if (ts.isNamespaceExport(exportDecl.exportClause)) {
      if (!this.tsUtils.isShareableType(this.tsTypeChecker.getTypeAtLocation(exportDecl.exportClause.name))) {
        this.incrementCounters(exportDecl.exportClause.name, _Problems.FaultID.SharedModuleExports);
      }
      return;
    }
    for (const exportSpecifier of exportDecl.exportClause.elements) {
      if (!this.tsUtils.isShareableEntity(exportSpecifier.name)) {
        this.incrementCounters(exportSpecifier.name, _Problems.FaultID.SharedModuleExports);
      }
    }
  }
}
exports.TypeScriptLinter = TypeScriptLinter;
_defineProperty(TypeScriptLinter, "sharedModulesCache", void 0);
_defineProperty(TypeScriptLinter, "filteredDiagnosticMessages", void 0);
_defineProperty(TypeScriptLinter, "ideMode", false);
_defineProperty(TypeScriptLinter, "testMode", false);
_defineProperty(TypeScriptLinter, "useRelaxedRules", false);
_defineProperty(TypeScriptLinter, "advancedClassChecks", false);
_defineProperty(TypeScriptLinter, "listFunctionApplyCallApis", ['Function.apply', 'Function.call', 'CallableFunction.apply', 'CallableFunction.call']);
_defineProperty(TypeScriptLinter, "listFunctionBindApis", ['Function.bind', 'CallableFunction.bind']);
_defineProperty(TypeScriptLinter, "LimitedApis", new Map([['global', {
  arr: _LimitedStdGlobalFunc.LIMITED_STD_GLOBAL_FUNC,
  fault: _Problems.FaultID.LimitedStdLibApi
}], ['Object', {
  arr: _LimitedStdObjectAPI.LIMITED_STD_OBJECT_API,
  fault: _Problems.FaultID.LimitedStdLibApi
}], ['ObjectConstructor', {
  arr: _LimitedStdObjectAPI.LIMITED_STD_OBJECT_API,
  fault: _Problems.FaultID.LimitedStdLibApi
}], ['Reflect', {
  arr: _LimitedStdReflectAPI.LIMITED_STD_REFLECT_API,
  fault: _Problems.FaultID.LimitedStdLibApi
}], ['ProxyHandler', {
  arr: _LimitedStdProxyHandlerAPI.LIMITED_STD_PROXYHANDLER_API,
  fault: _Problems.FaultID.LimitedStdLibApi
}], [_TsUtils.SYMBOL, {
  arr: null,
  fault: _Problems.FaultID.SymbolType
}], [_TsUtils.SYMBOL_CONSTRUCTOR, {
  arr: null,
  fault: _Problems.FaultID.SymbolType
}]]));
//# sourceMappingURL=TypeScriptLinter.js.map