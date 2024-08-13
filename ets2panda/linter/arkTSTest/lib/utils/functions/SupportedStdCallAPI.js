"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.SupportedStdCallApiChecker = void 0;
var ts = _interopRequireWildcard(require("typescript"));
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
function _defineProperty(e, r, t) { return (r = _toPropertyKey(r)) in e ? Object.defineProperty(e, r, { value: t, enumerable: !0, configurable: !0, writable: !0 }) : e[r] = t, e; }
function _toPropertyKey(t) { var i = _toPrimitive(t, "string"); return "symbol" == typeof i ? i : i + ""; }
function _toPrimitive(t, r) { if ("object" != typeof t || !t) return t; var e = t[Symbol.toPrimitive]; if (void 0 !== e) { var i = e.call(t, r || "default"); if ("object" != typeof i) return i; throw new TypeError("@@toPrimitive must return a primitive value."); } return ("string" === r ? String : Number)(t); } /*
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
class SupportedStdCallApiChecker {
  constructor(tsUtils, typeChecker) {
    _defineProperty(this, "tsUtils", void 0);
    _defineProperty(this, "typeChecker", void 0);
    _defineProperty(this, "stdObjectEntry", new Map([['assign', this.checkObjectAssignCall]]));
    _defineProperty(this, "StdCallApi", new Map([['Object', this.stdObjectEntry], ['ObjectConstructor', this.stdObjectEntry]]));
    this.tsUtils = tsUtils;
    this.typeChecker = typeChecker;
  }
  isSupportedStdCallAPI(callExpr, parentSymName, symName) {
    const entry = this.StdCallApi.get(parentSymName);
    if (entry) {
      const stdCallApiCheckCb = entry.get(symName);
      return !!stdCallApiCheckCb && stdCallApiCheckCb.call(this, callExpr);
    }
    return false;
  }
  checkObjectAssignCall(callExpr) {
    /*
     * 'Object.assign' is allowed only with signature like following:
     *    assign(target: Record<string, V>, ...source: Object[]>): Record<String, V>
     *
     * Note: For 'return' type, check the contextual type of call expression, as the
     * return type of actual call signature will be deduced as an intersection of all
     * types of the 'target' and 'source' arguments.
     */

    if (callExpr.typeArguments || callExpr.arguments.length === 0) {
      return false;
    }
    const targetArgType = this.typeChecker.getTypeAtLocation(callExpr.arguments[0]);
    if (!this.isValidObjectAssignRecordType(targetArgType)) {
      return false;
    }
    const contextualType = this.typeChecker.getContextualType(callExpr);
    if (!contextualType || !this.isValidObjectAssignRecordType(contextualType)) {
      return false;
    }
    return true;
  }
  isValidObjectAssignRecordType(type) {
    if (this.tsUtils.isStdRecordType(type) && type.aliasTypeArguments?.length) {
      const typeArg = type.aliasTypeArguments[0];
      if (typeArg.getFlags() & (ts.TypeFlags.String | ts.TypeFlags.StringLiteral)) {
        return true;
      }
    }
    return false;
  }
}
exports.SupportedStdCallApiChecker = SupportedStdCallApiChecker;
//# sourceMappingURL=SupportedStdCallAPI.js.map