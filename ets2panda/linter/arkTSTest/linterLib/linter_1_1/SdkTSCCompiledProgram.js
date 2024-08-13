"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.SdkTSCCompiledProgram = void 0;
var _ArkTSTimePrinter = require("../ArkTSTimePrinter");
var _SdkTypeScriptDiagnosticsExtractor = require("./SdkTypeScriptDiagnosticsExtractor");
function _defineProperty(e, r, t) { return (r = _toPropertyKey(r)) in e ? Object.defineProperty(e, r, { value: t, enumerable: !0, configurable: !0, writable: !0 }) : e[r] = t, e; }
function _toPropertyKey(t) { var i = _toPrimitive(t, "string"); return "symbol" == typeof i ? i : i + ""; }
function _toPrimitive(t, r) { if ("object" != typeof t || !t) return t; var e = t[Symbol.toPrimitive]; if (void 0 !== e) { var i = e.call(t, r || "default"); if ("object" != typeof i) return i; throw new TypeError("@@toPrimitive must return a primitive value."); } return ("string" === r ? String : Number)(t); } /*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, softwareP
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
class SdkTSCCompiledProgram {
  constructor(builerProgram) {
    _defineProperty(this, "builerProgram", void 0);
    this.builerProgram = builerProgram;
  }
  getProgram() {
    return this.builerProgram.getProgram();
  }
  getBuilderProgram() {
    return this.builerProgram;
  }
  getStrictDiagnostics(fileName) {
    return (0, _SdkTypeScriptDiagnosticsExtractor.getStrictDiagnostics)(this.getBuilderProgram(), fileName);
  }

  /**
   * Updates all diagnostics in TSC compilation program after the incremental build.
   */
  updateCompilationDiagnostics() {
    this.builerProgram.getSemanticDiagnostics();
    const timePrinterInstance = _ArkTSTimePrinter.ArkTSLinterTimePrinter.getInstance();
    timePrinterInstance.appendTime(_ArkTSTimePrinter.TimePhase.STRICT_PROGRAM_GET_SEMANTIC_DIAGNOSTICS);
    this.builerProgram.getSyntacticDiagnostics();
    timePrinterInstance.appendTime(_ArkTSTimePrinter.TimePhase.STRICT_PROGRAM_GET_SYNTACTIC_DIAGNOSTICS);
  }
}
exports.SdkTSCCompiledProgram = SdkTSCCompiledProgram;
//# sourceMappingURL=SdkTSCCompiledProgram.js.map