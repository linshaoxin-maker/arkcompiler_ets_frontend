"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.LinterConfig = void 0;
var ts = _interopRequireWildcard(require("typescript"));
var _Problems = require("./Problems");
var _LinterConfig;
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
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
class LinterConfig {
  static initTsSyntaxKindNames() {
    const keys = Object.keys(ts.SyntaxKind);
    const values = Object.values(ts.SyntaxKind);
    for (let i = 0; i < values.length; i++) {
      const val = values[i];
      const kindNum = typeof val === 'string' ? parseInt(val) : val;
      if (kindNum && !LinterConfig.tsSyntaxKindNames[kindNum]) {
        LinterConfig.tsSyntaxKindNames[kindNum] = keys[i];
      }
    }
  }

  // must detect terminals during parsing
}
exports.LinterConfig = LinterConfig;
_LinterConfig = LinterConfig;
/*
 * The SyntaxKind enum defines additional elements at the end of the enum
 * that serve as markers (FirstX/LastX). Those elements are initialized
 * with indices of the previously defined elements. As result, the enum
 * may return incorrect name for a certain kind index (e.g. 'FirstStatement'
 * instead of 'VariableStatement').
 * The following code creates a map with correct syntax kind names.
 * It can be used when need to print name of syntax kind of certain
 * AST node in diagnostic messages.
 */
_defineProperty(LinterConfig, "tsSyntaxKindNames", []);
_LinterConfig.initTsSyntaxKindNames();
_defineProperty(LinterConfig, "terminalTokens", new Set([ts.SyntaxKind.CloseBracketToken, ts.SyntaxKind.OpenBracketToken, ts.SyntaxKind.CloseParenToken, ts.SyntaxKind.OpenParenToken, ts.SyntaxKind.CloseBraceToken, ts.SyntaxKind.OpenBraceToken, ts.SyntaxKind.CommaToken, ts.SyntaxKind.SemicolonToken, ts.SyntaxKind.QuestionDotToken, ts.SyntaxKind.DotDotDotToken, ts.SyntaxKind.DotToken, ts.SyntaxKind.MinusMinusToken, ts.SyntaxKind.PlusPlusToken, ts.SyntaxKind.PercentToken, ts.SyntaxKind.SlashToken, ts.SyntaxKind.AsteriskAsteriskToken, ts.SyntaxKind.AsteriskToken, ts.SyntaxKind.MinusToken, ts.SyntaxKind.PlusToken, ts.SyntaxKind.EqualsGreaterThanToken, ts.SyntaxKind.ExclamationEqualsEqualsToken, ts.SyntaxKind.ExclamationEqualsToken, ts.SyntaxKind.EqualsEqualsToken, ts.SyntaxKind.EqualsEqualsEqualsToken, ts.SyntaxKind.GreaterThanEqualsToken, ts.SyntaxKind.LessThanEqualsToken, ts.SyntaxKind.GreaterThanGreaterThanGreaterThanToken, ts.SyntaxKind.GreaterThanGreaterThanToken, ts.SyntaxKind.GreaterThanToken, ts.SyntaxKind.LessThanSlashToken, ts.SyntaxKind.LessThanToken, ts.SyntaxKind.LessThanLessThanToken, ts.SyntaxKind.HashToken, ts.SyntaxKind.BacktickToken, ts.SyntaxKind.AtToken, ts.SyntaxKind.ColonToken, ts.SyntaxKind.QuestionToken, ts.SyntaxKind.QuestionQuestionToken, ts.SyntaxKind.BarBarToken, ts.SyntaxKind.AmpersandAmpersandToken, ts.SyntaxKind.TildeToken, ts.SyntaxKind.ExclamationToken, ts.SyntaxKind.CaretToken, ts.SyntaxKind.BarToken, ts.SyntaxKind.AmpersandToken, ts.SyntaxKind.CaretEqualsToken, ts.SyntaxKind.BarEqualsToken, ts.SyntaxKind.AmpersandEqualsToken, ts.SyntaxKind.GreaterThanGreaterThanGreaterThanEqualsToken, ts.SyntaxKind.GreaterThanGreaterThanEqualsToken, ts.SyntaxKind.LessThanLessThanEqualsToken, ts.SyntaxKind.PercentEqualsToken, ts.SyntaxKind.SlashEqualsToken, ts.SyntaxKind.AsteriskAsteriskEqualsToken, ts.SyntaxKind.AsteriskEqualsToken, ts.SyntaxKind.MinusEqualsToken, ts.SyntaxKind.PlusEqualsToken, ts.SyntaxKind.EqualsToken, ts.SyntaxKind.MultiLineCommentTrivia, ts.SyntaxKind.SingleLineCommentTrivia, ts.SyntaxKind.WhitespaceTrivia, ts.SyntaxKind.NewLineTrivia, ts.SyntaxKind.EndOfFileToken, ts.SyntaxKind.ConflictMarkerTrivia, ts.SyntaxKind.ShebangTrivia]));
// tokens which can be reported without additional parsing
_defineProperty(LinterConfig, "incrementOnlyTokens", new Map([[ts.SyntaxKind.AnyKeyword, _Problems.FaultID.AnyType], [ts.SyntaxKind.SymbolKeyword, _Problems.FaultID.SymbolType], [ts.SyntaxKind.ThisType, _Problems.FaultID.ThisType], [ts.SyntaxKind.TypeQuery, _Problems.FaultID.TypeQuery], [ts.SyntaxKind.DeleteExpression, _Problems.FaultID.DeleteOperator], [ts.SyntaxKind.TypePredicate, _Problems.FaultID.IsOperator], [ts.SyntaxKind.YieldExpression, _Problems.FaultID.YieldExpression], [ts.SyntaxKind.WithStatement, _Problems.FaultID.WithStatement], [ts.SyntaxKind.IndexedAccessType, _Problems.FaultID.IndexedAccessType], [ts.SyntaxKind.UnknownKeyword, _Problems.FaultID.UnknownType], [ts.SyntaxKind.CallSignature, _Problems.FaultID.CallSignature], [ts.SyntaxKind.IntersectionType, _Problems.FaultID.IntersectionType], [ts.SyntaxKind.ConstructorType, _Problems.FaultID.ConstructorFuncs], [ts.SyntaxKind.ConditionalType, _Problems.FaultID.ConditionalType], [ts.SyntaxKind.MappedType, _Problems.FaultID.MappedType], [ts.SyntaxKind.JsxElement, _Problems.FaultID.JsxElement], [ts.SyntaxKind.JsxSelfClosingElement, _Problems.FaultID.JsxElement], [ts.SyntaxKind.ImportEqualsDeclaration, _Problems.FaultID.ImportAssignment], [ts.SyntaxKind.NamespaceExportDeclaration, _Problems.FaultID.UMDModuleDefinition], [ts.SyntaxKind.ClassExpression, _Problems.FaultID.ClassExpression]]));
//# sourceMappingURL=TypeScriptLinterConfig.js.map