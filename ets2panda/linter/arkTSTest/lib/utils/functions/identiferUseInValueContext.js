"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.identiferUseInValueContext = identiferUseInValueContext;
var ts = _interopRequireWildcard(require("typescript"));
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
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

function isInstanceofContext(tsIdentStart) {
  return ts.isBinaryExpression(tsIdentStart.parent) && tsIdentStart.parent.operatorToken.kind === ts.SyntaxKind.InstanceOfKeyword;
}
function isNewExpressionContext(tsIdentStart) {
  return ts.isNewExpression(tsIdentStart.parent) && tsIdentStart === tsIdentStart.parent.expression;
}

/*
 * If identifier is the right-most name of Property Access chain or Qualified name,
 * or it's a separate identifier expression, then identifier is being referenced as an value.
 */
function isQualifiedNameContext(tsIdentStart, tsIdentifier) {
  // rightmost in AST is rightmost in qualified name chain
  return ts.isQualifiedName(tsIdentStart) && tsIdentifier !== tsIdentStart.right;
}
function isPropertyAccessContext(tsIdentStart, tsIdentifier) {
  // rightmost in AST is rightmost in qualified name chain
  return ts.isPropertyAccessExpression(tsIdentStart) && tsIdentifier !== tsIdentStart.name;
}
function getQualifiedStart(ident) {
  let qualifiedStart = ident;
  while (ts.isPropertyAccessExpression(qualifiedStart.parent) || ts.isQualifiedName(qualifiedStart.parent)) {
    qualifiedStart = qualifiedStart.parent;
  }
  return qualifiedStart;
}
function isEnumPropAccess(ident, tsSym, context) {
  return ts.isElementAccessExpression(context) && !!(tsSym.flags & ts.SymbolFlags.Enum) && (context.expression === ident || ts.isPropertyAccessExpression(context.expression) && context.expression.name === ident);
}
function isValidParent(parent) {
  // treat TypeQuery as valid because it's already forbidden (FaultID.TypeQuery)
  return ts.isTypeNode(parent) && !ts.isTypeOfExpression(parent) || ts.isExpressionWithTypeArguments(parent) || ts.isExportAssignment(parent) || ts.isExportSpecifier(parent) || ts.isMetaProperty(parent) || ts.isImportClause(parent) || ts.isClassLike(parent) || ts.isInterfaceDeclaration(parent) || ts.isModuleDeclaration(parent) || ts.isEnumDeclaration(parent) || ts.isNamespaceImport(parent) || ts.isImportSpecifier(parent) || ts.isImportEqualsDeclaration(parent);
}
function identiferUseInValueContext(ident, tsSym) {
  const qualifiedStart = getQualifiedStart(ident);
  const parent = qualifiedStart.parent;
  const isValidUse = isValidParent(parent) || isEnumPropAccess(ident, tsSym, parent) || isQualifiedNameContext(qualifiedStart, ident) || isPropertyAccessContext(qualifiedStart, ident) || isNewExpressionContext(qualifiedStart) || isInstanceofContext(qualifiedStart);
  return !isValidUse;
}
//# sourceMappingURL=identiferUseInValueContext.js.map