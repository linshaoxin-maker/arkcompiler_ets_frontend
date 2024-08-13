"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.TSCCompiledProgramWithDiagnostics = exports.TSCCompiledProgramSimple = void 0;
exports.transformDiagnostic = transformDiagnostic;
var ts = _interopRequireWildcard(require("typescript"));
var _ProblemSeverity = require("../ProblemSeverity");
var _TypeScriptDiagnosticsExtractor = require("./TypeScriptDiagnosticsExtractor");
var _Problems = require("../Problems");
var _FaultAttrs = require("../FaultAttrs");
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
function _newArrowCheck(n, r) { if (n !== r) throw new TypeError("Cannot instantiate an arrow function"); }
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
class TSCCompiledProgramSimple {
  constructor(program) {
    _defineProperty(this, "program", void 0);
    this.program = program;
  }
  getProgram() {
    return this.program;
  }
  getStrictDiagnostics(fileName) {
    void this;
    void fileName;
    return [];
  }
}
exports.TSCCompiledProgramSimple = TSCCompiledProgramSimple;
class TSCCompiledProgramWithDiagnostics {
  constructor(strict, nonStrict, inputFiles, cancellationToken, progressCb) {
    var _this = this;
    _defineProperty(this, "program", void 0);
    _defineProperty(this, "cachedDiagnostics", new Map());
    this.program = strict;
    inputFiles.forEach(function (fileName) {
      _newArrowCheck(this, _this);
      progressCb?.(fileName);
      const sourceFile = this.program.getSourceFile(fileName);
      if (sourceFile !== undefined) {
        this.cachedDiagnostics.set(sourceFile.fileName, (0, _TypeScriptDiagnosticsExtractor.getStrictDiagnostics)(strict, nonStrict, sourceFile.fileName, cancellationToken));
      }
    }.bind(this));
  }
  getProgram() {
    return this.program;
  }
  getStrictDiagnostics(fileName) {
    return this.cachedDiagnostics.get(fileName) ?? [];
  }
}
exports.TSCCompiledProgramWithDiagnostics = TSCCompiledProgramWithDiagnostics;
function transformDiagnostic(diagnostic) {
  const startPos = diagnostic.start;
  const start = getLineAndColumn(diagnostic.file, startPos);
  const endPos = startPos + diagnostic.length;
  const end = getLineAndColumn(diagnostic.file, endPos);
  const messageText = ts.flattenDiagnosticMessageText(diagnostic.messageText, '\n');
  const faultId = _Problems.FaultID.StrictDiagnostic;
  return {
    line: start.line,
    column: start.column,
    endLine: end.line,
    endColumn: end.column,
    start: startPos,
    end: endPos,
    type: 'StrictModeError',
    // expect strict options to always present
    severity: _ProblemSeverity.ProblemSeverity.ERROR,
    problem: _Problems.FaultID[faultId],
    suggest: messageText,
    rule: messageText,
    ruleTag: _FaultAttrs.faultsAttrs[faultId] ? _FaultAttrs.faultsAttrs[faultId].cookBookRef : 0
  };
}

/**
 * Returns line and column of the position, counts from 1
 */
function getLineAndColumn(file, position) {
  const {
    line,
    character
  } = file.getLineAndCharacterOfPosition(position);
  // TSC counts lines and columns from zero
  return {
    line: line + 1,
    column: character + 1
  };
}
//# sourceMappingURL=TSCCompiledProgram.js.map