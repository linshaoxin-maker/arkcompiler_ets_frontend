"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.InteropTypescriptLinter = void 0;
exports.consoleLog = consoleLog;
var ts = _interopRequireWildcard(require("typescript"));
var path = _interopRequireWildcard(require("node:path"));
var fs = _interopRequireWildcard(require("fs"));
var _TsUtils = require("./utils/TsUtils");
var _Problems = require("./Problems");
var _FaultAttrs = require("./FaultAttrs");
var _FaultDesc = require("./FaultDesc");
var _CookBookMsg = require("./CookBookMsg");
var _TypeScriptLinterConfig = require("./TypeScriptLinterConfig");
var _ProblemSeverity = require("./ProblemSeverity");
var _Logger = require("./Logger");
var _AutofixTitles = require("./autofixes/AutofixTitles");
var _ForEachNodeInSubtree = require("./utils/functions/ForEachNodeInSubtree");
var _SupportedDetsIndexableTypes = require("./utils/consts/SupportedDetsIndexableTypes");
var _TsSuffix = require("./utils/consts/TsSuffix");
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
function _newArrowCheck(n, r) { if (n !== r) throw new TypeError("Cannot instantiate an arrow function"); }
function _defineProperty(e, r, t) { return (r = _toPropertyKey(r)) in e ? Object.defineProperty(e, r, { value: t, enumerable: !0, configurable: !0, writable: !0 }) : e[r] = t, e; }
function _toPropertyKey(t) { var i = _toPrimitive(t, "string"); return "symbol" == typeof i ? i : i + ""; }
function _toPrimitive(t, r) { if ("object" != typeof t || !t) return t; var e = t[Symbol.toPrimitive]; if (void 0 !== e) { var i = e.call(t, r || "default"); if ("object" != typeof i) return i; throw new TypeError("@@toPrimitive must return a primitive value."); } return ("string" === r ? String : Number)(t); } /*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
  if (InteropTypescriptLinter.ideMode) {
    return;
  }
  let outLine = '';
  for (let k = 0; k < args.length; k++) {
    outLine += `${args[k]} `;
  }
  _Logger.Logger.info(outLine);
}
class InteropTypescriptLinter {
  initCounters() {
    for (let i = 0; i < _Problems.FaultID.LAST_ID; i++) {
      this.nodeCounters[i] = 0;
      this.lineCounters[i] = 0;
    }
  }
  constructor(tsTypeChecker, compileOptions, incrementalLintInfo, useRtLogic) {
    this.tsTypeChecker = tsTypeChecker;
    this.compileOptions = compileOptions;
    this.incrementalLintInfo = incrementalLintInfo;
    this.useRtLogic = useRtLogic;
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
    _defineProperty(this, "autofixer", void 0);
    _defineProperty(this, "sourceFile", void 0);
    _defineProperty(this, "isInSdk", void 0);
    _defineProperty(this, "handlersMap", new Map([[ts.SyntaxKind.ImportDeclaration, this.handleImportDeclaration], [ts.SyntaxKind.InterfaceDeclaration, this.handleInterfaceDeclaration], [ts.SyntaxKind.ClassDeclaration, this.handleClassDeclaration], [ts.SyntaxKind.NewExpression, this.handleNewExpression], [ts.SyntaxKind.ObjectLiteralExpression, this.handleObjectLiteralExpression], [ts.SyntaxKind.ArrayLiteralExpression, this.handleArrayLiteralExpression], [ts.SyntaxKind.AsExpression, this.handleAsExpression], [ts.SyntaxKind.ExportDeclaration, this.handleExportDeclaration], [ts.SyntaxKind.ExportAssignment, this.handleExportAssignment]]));
    this.tsUtils = new _TsUtils.TsUtils(this.tsTypeChecker, InteropTypescriptLinter.testMode, InteropTypescriptLinter.advancedClassChecks, !!this.useRtLogic);
    this.currentErrorLine = 0;
    this.currentWarningLine = 0;
    InteropTypescriptLinter.etsLoaderPath = compileOptions.etsLoaderPath ? compileOptions.etsLoaderPath : '';
    InteropTypescriptLinter.sdkPath = path.resolve(InteropTypescriptLinter.etsLoaderPath, '../..');
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
    if (InteropTypescriptLinter.ideMode) {
      this.incrementCountersIdeMode(node, faultId, autofix);
    } else {
      const faultDescr = _FaultDesc.faultDesc[faultId];
      const faultType = _TypeScriptLinterConfig.LinterConfig.tsSyntaxKindNames[node.kind];
      _Logger.Logger.info(`Warning: ${this.sourceFile.fileName} (${line}, ${character}): ${faultDescr ? faultDescr : faultType}`);
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
    if (!InteropTypescriptLinter.ideMode) {
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
  }
  visitSourceFile(sf) {
    var _this = this;
    const callback = function callback(node) {
      _newArrowCheck(this, _this);
      this.totalVisitedNodes++;
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
      if (_TypeScriptLinterConfig.LinterConfig.terminalTokens.has(node.kind)) {
        return true;
      }
      return false;
    }.bind(this);
    (0, _ForEachNodeInSubtree.forEachNodeInSubtree)(sf, callback, stopCondition);
  }
  handleImportDeclaration(node) {
    const importDeclaration = node;
    this.checkSendableClassorISendable(importDeclaration);
  }
  checkSendableClassorISendable(node) {
    const currentSourceFile = node.getSourceFile();
    const contextSpecifier = node.moduleSpecifier;
    if (!ts.isStringLiteralLike(contextSpecifier)) {
      return;
    }
    const resolvedModule = this.getResolveModule(contextSpecifier.text, currentSourceFile.fileName);
    const importClause = node.importClause;
    if (!resolvedModule) {
      return;
    }
    // handle kit
    const baseFileName = path.basename(resolvedModule.resolvedFileName);
    if (baseFileName.startsWith(_TsSuffix.KIT) && baseFileName.endsWith(_TsSuffix.D_TS)) {
      if (!InteropTypescriptLinter.etsLoaderPath) {
        return;
      }
      InteropTypescriptLinter.initKitInfos(baseFileName);
      if (!importClause) {
        return;
      }

      // skip default import
      if (importClause.name) {
        return;
      }
      if (importClause.namedBindings && ts.isNamedImports(importClause.namedBindings)) {
        this.checkKitImportClause(importClause.namedBindings, baseFileName);
      }
      return;
    }
    if (resolvedModule?.extension !== _TsSuffix.ETS && resolvedModule?.extension !== _TsSuffix.D_ETS || _TsUtils.TsUtils.isInImportWhiteList(resolvedModule)) {
      return;
    }
    if (!importClause) {
      this.incrementCounters(node, _Problems.FaultID.NoSideEffectImportEtsToTs);
      return;
    }
    this.checkImportClause(importClause, resolvedModule);
  }
  getResolveModule(moduleSpecifier, fileName) {
    const resolveModuleName = ts.resolveModuleName(moduleSpecifier, fileName, this.compileOptions, ts.sys);
    return resolveModuleName.resolvedModule;
  }
  checkKitImportClause(node, kitFileName) {
    const length = node.elements.length;
    for (let i = 0; i < length; i++) {
      const fileName = InteropTypescriptLinter.getKitModuleFileNames(kitFileName, node, i);
      if (fileName === '' || fileName.endsWith(_TsSuffix.D_TS)) {
        continue;
      }
      const element = node.elements[i];
      const decl = this.tsUtils.getDeclarationNode(element.name);
      if (!decl) {
        continue;
      }
      if (ts.isModuleDeclaration(decl) && fileName !== _SupportedDetsIndexableTypes.ARKTS_COLLECTIONS_D_ETS && fileName !== _SupportedDetsIndexableTypes.ARKTS_LANG_D_ETS || !this.tsUtils.isSendableClassOrInterfaceEntity(element.name)) {
        this.incrementCounters(element, _Problems.FaultID.NoTsImportEts);
      }
    }
  }
  checkImportClause(node, resolvedModule) {
    var _this2 = this;
    const checkAndIncrement = function checkAndIncrement(identifier) {
      _newArrowCheck(this, _this2);
      if (identifier && !this.tsUtils.isSendableClassOrInterfaceEntity(identifier)) {
        this.incrementCounters(identifier, _Problems.FaultID.NoTsImportEts);
      }
    }.bind(this);
    if (node.name) {
      if (this.allowInSdkImportSendable(resolvedModule)) {
        return;
      }
      checkAndIncrement(node.name);
    }
    if (!node.namedBindings) {
      return;
    }
    if (ts.isNamespaceImport(node.namedBindings)) {
      this.incrementCounters(node.namedBindings, _Problems.FaultID.NoNameSpaceImportEtsToTs);
    } else if (ts.isNamedImports(node.namedBindings)) {
      node.namedBindings.elements.forEach(function (element) {
        _newArrowCheck(this, _this2);
        checkAndIncrement(element.name);
      }.bind(this));
    }
  }
  allowInSdkImportSendable(resolvedModule) {
    const resolvedModuleIsInSdk = InteropTypescriptLinter.etsLoaderPath ? path.normalize(resolvedModule.resolvedFileName).startsWith(InteropTypescriptLinter.sdkPath) : false;
    return !!this.isInSdk && resolvedModuleIsInSdk && path.basename(resolvedModule.resolvedFileName).indexOf('sendable') !== -1;
  }
  handleClassDeclaration(node) {
    const tsClassDecl = node;
    if (!tsClassDecl.heritageClauses) {
      return;
    }
    for (const hClause of tsClassDecl.heritageClauses) {
      if (hClause) {
        this.checkClassOrInterfaceDeclarationHeritageClause(hClause);
      }
    }
  }

  // In ts files, sendable classes and sendable interfaces can not be extended or implemented.
  checkClassOrInterfaceDeclarationHeritageClause(hClause) {
    for (const tsTypeExpr of hClause.types) {
      /*
       * Always resolve type from 'tsTypeExpr' node, not from 'tsTypeExpr.expression' node,
       * as for the latter, type checker will return incorrect type result for classes in
       * 'extends' clause. Additionally, reduce reference, as mostly type checker returns
       * the TypeReference type objects for classes and interfaces.
       */
      const tsExprType = _TsUtils.TsUtils.reduceReference(this.tsTypeChecker.getTypeAtLocation(tsTypeExpr));
      const isSendableBaseType = this.tsUtils.isSendableClassOrInterface(tsExprType);
      if (isSendableBaseType) {
        this.incrementCounters(tsTypeExpr, _Problems.FaultID.SendableTypeInheritance);
      }
    }
  }
  handleInterfaceDeclaration(node) {
    const interfaceNode = node;
    const iSymbol = this.tsUtils.trueSymbolAtLocation(interfaceNode.name);
    const iDecls = iSymbol ? iSymbol.getDeclarations() : null;
    if (!iDecls) {
      return;
    }
    if (!interfaceNode.heritageClauses) {
      return;
    }
    for (const hClause of interfaceNode.heritageClauses) {
      if (hClause) {
        this.checkClassOrInterfaceDeclarationHeritageClause(hClause);
      }
    }
  }
  handleNewExpression(node) {
    const tsNewExpr = node;
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
  handleObjectLiteralExpression(node) {
    const objectLiteralExpr = node;
    const objectLiteralType = this.tsTypeChecker.getContextualType(objectLiteralExpr);
    if (objectLiteralType && this.tsUtils.typeContainsSendableClassOrInterface(objectLiteralType)) {
      this.incrementCounters(node, _Problems.FaultID.SendableObjectInitialization);
    }
  }
  handleArrayLiteralExpression(node) {
    const arrayLitNode = node;
    const arrayLitType = this.tsTypeChecker.getContextualType(arrayLitNode);
    if (arrayLitType && this.tsUtils.typeContainsSendableClassOrInterface(arrayLitType)) {
      this.incrementCounters(node, _Problems.FaultID.SendableObjectInitialization);
    }
  }
  handleAsExpression(node) {
    const tsAsExpr = node;
    const targetType = this.tsTypeChecker.getTypeAtLocation(tsAsExpr.type).getNonNullableType();
    const exprType = this.tsTypeChecker.getTypeAtLocation(tsAsExpr.expression).getNonNullableType();
    if (!this.tsUtils.isSendableClassOrInterface(exprType) && !this.tsUtils.isObject(exprType) && !_TsUtils.TsUtils.isAnyType(exprType) && this.tsUtils.isSendableClassOrInterface(targetType)) {
      this.incrementCounters(tsAsExpr, _Problems.FaultID.SendableAsExpr);
    }
  }
  handleExportDeclaration(node) {
    const exportDecl = node;
    const currentSourceFile = exportDecl.getSourceFile();
    const contextSpecifier = exportDecl.moduleSpecifier;
    if (contextSpecifier && ts.isStringLiteralLike(contextSpecifier)) {
      const resolvedModule = this.getResolveModule(contextSpecifier.text, currentSourceFile.fileName);
      if (!resolvedModule) {
        return;
      }
      if (this.isKitModule(resolvedModule.resolvedFileName, exportDecl)) {
        return;
      }
      if (InteropTypescriptLinter.isEtsFile(resolvedModule.extension)) {
        this.incrementCounters(contextSpecifier, _Problems.FaultID.NoTsReExportEts);
      }
      return;
    }
    if (!this.isInSdk) {
      return;
    }
    this.handleSdkExport(exportDecl);
  }
  isKitModule(resolvedFileName, exportDecl) {
    const baseFileName = path.basename(resolvedFileName);
    if (baseFileName.startsWith(_TsSuffix.KIT) && baseFileName.endsWith(_TsSuffix.D_TS)) {
      if (!InteropTypescriptLinter.etsLoaderPath) {
        return true;
      }
      InteropTypescriptLinter.initKitInfos(baseFileName);
      const exportClause = exportDecl.exportClause;
      if (exportClause && ts.isNamedExports(exportClause)) {
        this.checkKitImportClause(exportClause, baseFileName);
      }
      return true;
    }
    return false;
  }
  static isEtsFile(extension) {
    return extension === _TsSuffix.ETS || extension === _TsSuffix.D_ETS;
  }
  handleSdkExport(exportDecl) {
    if (!exportDecl.exportClause || !ts.isNamedExports(exportDecl.exportClause)) {
      return;
    }
    for (const exportSpecifier of exportDecl.exportClause.elements) {
      if (this.tsUtils.isSendableClassOrInterfaceEntity(exportSpecifier.name)) {
        this.incrementCounters(exportSpecifier.name, _Problems.FaultID.SendableTypeExported);
      }
    }
  }
  handleExportAssignment(node) {
    if (!this.isInSdk) {
      return;
    }

    // In sdk .d.ts files, sendable classes and sendable interfaces can not be "default" exported.
    const exportAssignment = node;
    if (this.tsUtils.isSendableClassOrInterfaceEntity(exportAssignment.expression)) {
      this.incrementCounters(exportAssignment.expression, _Problems.FaultID.SendableTypeExported);
    }
  }
  static initKitInfos(fileName) {
    var _this3 = this;
    if (InteropTypescriptLinter.kitInfos.has(fileName)) {
      return;
    }
    const JSON_SUFFIX = '.json';
    const KIT_CONFIGS = '../ets-loader/kit_configs';
    const KIT_CONFIG_PATH = './build-tools/ets-loader/kit_configs';
    const kitConfigs = [path.resolve(InteropTypescriptLinter.etsLoaderPath, KIT_CONFIGS)];
    if (process.env.externalApiPaths) {
      const externalApiPaths = process.env.externalApiPaths.split(path.delimiter);
      externalApiPaths.forEach(function (sdkPath) {
        _newArrowCheck(this, _this3);
        kitConfigs.push(path.resolve(sdkPath, KIT_CONFIG_PATH));
      }.bind(this));
    }
    for (const kitConfig of kitConfigs) {
      const kitModuleConfigJson = path.resolve(kitConfig, './' + fileName.replace(_TsSuffix.D_TS, JSON_SUFFIX));
      if (fs.existsSync(kitModuleConfigJson)) {
        InteropTypescriptLinter.kitInfos.set(fileName, JSON.parse(fs.readFileSync(kitModuleConfigJson, 'utf-8')));
      }
    }
  }
  static getKitModuleFileNames(fileName, node, index) {
    if (!InteropTypescriptLinter.kitInfos.has(fileName)) {
      return '';
    }
    const kitInfo = InteropTypescriptLinter.kitInfos.get(fileName);
    if (!kitInfo?.symbols) {
      return '';
    }
    const element = node.elements[index];
    return element.propertyName ? kitInfo.symbols[element.propertyName.text].source : kitInfo.symbols[element.name.text].source;
  }
  lint(sourceFile) {
    this.sourceFile = sourceFile;
    this.isInSdk = InteropTypescriptLinter.etsLoaderPath ? path.normalize(this.sourceFile.fileName).indexOf(InteropTypescriptLinter.sdkPath) === 0 : false;
    this.visitSourceFile(this.sourceFile);
  }
}
exports.InteropTypescriptLinter = InteropTypescriptLinter;
_defineProperty(InteropTypescriptLinter, "ideMode", false);
_defineProperty(InteropTypescriptLinter, "testMode", false);
_defineProperty(InteropTypescriptLinter, "kitInfos", new Map());
_defineProperty(InteropTypescriptLinter, "etsLoaderPath", void 0);
_defineProperty(InteropTypescriptLinter, "sdkPath", void 0);
_defineProperty(InteropTypescriptLinter, "advancedClassChecks", false);
//# sourceMappingURL=InteropTypescriptLinter.js.map