"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.SymbolCache = void 0;
var ts = _interopRequireWildcard(require("typescript"));
var _TypeScriptLinterConfig = require("../TypeScriptLinterConfig");
var _ForEachNodeInSubtree = require("../utils/functions/ForEachNodeInSubtree");
var _IsStruct = require("../utils/functions/IsStruct");
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
class SymbolCache {
  constructor(typeChecker, utils, sourceFile, cancellationToken) {
    var _this = this;
    this.typeChecker = typeChecker;
    this.utils = utils;
    this.sourceFile = sourceFile;
    this.cancellationToken = cancellationToken;
    _defineProperty(this, "handlersMap", new Map([[ts.SyntaxKind.ElementAccessExpression, this.handleElementAccessExpression], [ts.SyntaxKind.EnumDeclaration, this.handleEnumDeclaration], [ts.SyntaxKind.PrivateIdentifier, this.handlePrivateIdentifier], [ts.SyntaxKind.PropertyAssignment, this.handlePropertyAssignment], [ts.SyntaxKind.PropertyDeclaration, this.handlePropertyDeclaration], [ts.SyntaxKind.PropertySignature, this.handlePropertySignature], [ts.SyntaxKind.FunctionDeclaration, this.handleFunctionDeclaration], [ts.SyntaxKind.CallExpression, this.handleCallExpression], [ts.SyntaxKind.Identifier, this.handleIdentifier]]));
    _defineProperty(this, "cache", new Map());
    const callback = function callback(node) {
      _newArrowCheck(this, _this);
      if ((0, _IsStruct.isStructDeclaration)(node)) {
        // early exit via exception if cancellation was requested
        this.cancellationToken?.throwIfCancellationRequested();
      }
      const symbol = this.handlersMap.get(node.kind)?.call(this, node);
      if (symbol !== undefined) {
        this.addReference(symbol, node);
      }
    }.bind(this);
    const stopCondition = function stopCondition(node) {
      _newArrowCheck(this, _this);
      return !node || _TypeScriptLinterConfig.LinterConfig.terminalTokens.has(node.kind);
    }.bind(this);
    (0, _ForEachNodeInSubtree.forEachNodeInSubtree)(sourceFile, callback, stopCondition);
  }
  getReferences(symbol) {
    return this.cache.get(symbol) ?? [];
  }
  handleElementAccessExpression(node) {
    const elementAccessExpr = node;
    return this.typeChecker.getSymbolAtLocation(elementAccessExpr.argumentExpression);
  }
  handleEnumDeclaration(node) {
    const enumDeclaration = node;
    return this.utils.trueSymbolAtLocation(enumDeclaration.name);
  }
  handlePrivateIdentifier(node) {
    const privateIdentifier = node;
    return this.typeChecker.getSymbolAtLocation(privateIdentifier);
  }
  handlePropertyAssignment(node) {
    const propertyAssignment = node;
    const contextualType = this.typeChecker.getContextualType(propertyAssignment.parent);
    return contextualType === undefined ? undefined : this.utils.getPropertySymbol(contextualType, propertyAssignment);
  }
  handlePropertyDeclaration(node) {
    const propertyDeclaration = node;
    return this.typeChecker.getSymbolAtLocation(propertyDeclaration.name);
  }
  handlePropertySignature(node) {
    const propertySignature = node;
    return this.typeChecker.getSymbolAtLocation(propertySignature.name);
  }
  handleFunctionDeclaration(node) {
    const functionDeclaration = node;
    return functionDeclaration.name ? this.typeChecker.getSymbolAtLocation(functionDeclaration.name) : undefined;
  }
  handleCallExpression(node) {
    const callExpression = node;
    return this.typeChecker.getSymbolAtLocation(callExpression.expression);
  }
  handleIdentifier(node) {
    const identifier = node;
    const symbol = this.typeChecker.getSymbolAtLocation(identifier);
    if (symbol?.flags) {
      return (symbol.flags & ts.SymbolFlags.Variable) !== 0 ? symbol : undefined;
    }
    return undefined;
  }
  addReference(symbol, node) {
    let nodes = this.cache.get(symbol);
    if (nodes === undefined) {
      nodes = [];
      this.cache.set(symbol, nodes);
    }
    nodes.push(node);
  }
}
exports.SymbolCache = SymbolCache;
//# sourceMappingURL=SymbolCache.js.map