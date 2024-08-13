"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.TimePhase = exports.ArkTSLinterTimePrinter = void 0;
var _ArkTSLinterTimePrinter;
function _newArrowCheck(n, r) { if (n !== r) throw new TypeError("Cannot instantiate an arrow function"); }
function _defineProperty(e, r, t) { return (r = _toPropertyKey(r)) in e ? Object.defineProperty(e, r, { value: t, enumerable: !0, configurable: !0, writable: !0 }) : e[r] = t, e; }
function _toPropertyKey(t) { var i = _toPrimitive(t, "string"); return "symbol" == typeof i ? i : i + ""; }
function _toPrimitive(t, r) { if ("object" != typeof t || !t) return t; var e = t[Symbol.toPrimitive]; if (void 0 !== e) { var i = e.call(t, r || "default"); if ("object" != typeof i) return i; throw new TypeError("@@toPrimitive must return a primitive value."); } return ("string" === r ? String : Number)(t); }
/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
let TimePhase = exports.TimePhase = /*#__PURE__*/function (TimePhase) {
  TimePhase["START"] = "start";
  TimePhase["GET_PROGRAM"] = "getProgram(not ArkTSLinter)";
  TimePhase["GET_REVERSE_STRICT_BUILDER_PROGRAM"] = "getReverseStrictBuilderProgram";
  TimePhase["UPDATE_ERROR_FILE"] = "updateErrorFile";
  TimePhase["INIT"] = "init";
  TimePhase["STRICT_PROGRAM_GET_SEMANTIC_DIAGNOSTICS"] = "strictProgramGetSemanticDiagnostics";
  TimePhase["STRICT_PROGRAM_GET_SYNTACTIC_DIAGNOSTICS"] = "strictProgramGetSyntacticDiagnostics";
  TimePhase["NON_STRICT_PROGRAM_GET_SEMANTIC_DIAGNOSTICS"] = "nonStrictProgramGetSemanticDiagnostics";
  TimePhase["NON_STRICT_PROGRAM_GET_SYNTACTIC_DIAGNOSTICS"] = "nonStrictProgramGetSyntacticDiagnostics";
  TimePhase["GET_TSC_DIAGNOSTICS"] = "getTscDiagnostics";
  TimePhase["EMIT_BUILD_INFO"] = "emitBuildInfo";
  TimePhase["LINT"] = "lint";
  return TimePhase;
}({});
class ArkTSLinterTimePrinter {
  constructor(ArkTSTimePrintSwitch = false) {
    _defineProperty(this, "arkTSTimePrintSwitch", void 0);
    _defineProperty(this, "timeMap", new Map());
    this.arkTSTimePrintSwitch = ArkTSTimePrintSwitch;
  }
  static getInstance() {
    if (!ArkTSLinterTimePrinter.instance) {
      ArkTSLinterTimePrinter.instance = new ArkTSLinterTimePrinter();
    }
    return ArkTSLinterTimePrinter.instance;
  }
  static destroyInstance() {
    ArkTSLinterTimePrinter.instance = undefined;
  }
  setArkTSTimePrintSwitch(arkTSTimePrintSwitch) {
    this.arkTSTimePrintSwitch = arkTSTimePrintSwitch;
  }
  appendTime(key) {
    if (this.arkTSTimePrintSwitch) {
      this.timeMap.set(key, Date.now());
    }
  }

  // eslint-disable-next-line class-methods-use-this
  formatMapAsTable(map) {
    var _this = this;
    if (!map.has(TimePhase.START)) {
      return 'ArkTSLinterTimePrinter: START phase is not exist!';
    }
    let maxKeyLength = 0;
    let lastVal = 0;
    let sum = 0;
    const printMap = new Map(map);
    printMap.forEach(function (value, key) {
      _newArrowCheck(this, _this);
      maxKeyLength = Math.max(maxKeyLength, key.toString().length);
      if (key !== TimePhase.START) {
        const relativeVal = value - lastVal;
        if (key !== TimePhase.GET_PROGRAM) {
          sum += relativeVal;
        }
        printMap.set(key, relativeVal / 1000.0);
      }
      lastVal = value;
    }.bind(this));
    printMap.set('total', sum / 1000.0);
    let table = '';
    printMap.forEach(function (value, key) {
      _newArrowCheck(this, _this);
      if (key !== TimePhase.START) {
        const paddedKey = key.toString().padEnd(maxKeyLength, ' ');
        table += `${paddedKey} | ${value.toString()}\n`;
      }
    }.bind(this));
    const header = `${'Phase'.padEnd(maxKeyLength, ' ')} | Time(seconds)\n`;
    const FIXLENGTH = ('Phase' + 'Time').length;
    const separator = '-'.repeat(maxKeyLength + FIXLENGTH) + '\n';
    return `${header}${separator}${table}`;
  }
  printTimes() {
    if (this.arkTSTimePrintSwitch) {
      const formattedMap = this.formatMapAsTable(this.timeMap);
      console.log(formattedMap);
    }
  }
}
exports.ArkTSLinterTimePrinter = ArkTSLinterTimePrinter;
_ArkTSLinterTimePrinter = ArkTSLinterTimePrinter;
_defineProperty(ArkTSLinterTimePrinter, "instance", void 0);
//# sourceMappingURL=ArkTSTimePrinter.js.map