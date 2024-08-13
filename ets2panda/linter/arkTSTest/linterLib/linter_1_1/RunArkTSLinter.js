"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.runArkTSLinter = runArkTSLinter;
exports.translateDiag = translateDiag;
var ts = _interopRequireWildcard(require("typescript"));
var path = _interopRequireWildcard(require("node:path"));
var _ProblemSeverity = require("../../lib/ProblemSeverity");
var _TypeScriptLinter = require("../../lib/TypeScriptLinter");
var _GetTscDiagnostics = require("../../lib/ts-diagnostics/GetTscDiagnostics");
var _ArkTSTimePrinter = require("../ArkTSTimePrinter");
var _GetScriptKind = require("../../lib/utils/functions/GetScriptKind");
var _SdkTSCCompiledProgram = require("./SdkTSCCompiledProgram");
var _incrementalLinter = require("./incrementalLinter");
var _InteropTypescriptLinter = require("../../lib/InteropTypescriptLinter");
var _Logger = require("../../lib/Logger");
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
function _newArrowCheck(n, r) { if (n !== r) throw new TypeError("Cannot instantiate an arrow function"); } /*
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
function makeDiag(category, code, file, start, length, messageText) {
  return {
    category,
    code,
    file,
    start,
    length,
    messageText
  };
}
function translateDiag(srcFile, problemInfo) {
  const LINTER_MSG_CODE_START = -1;
  const severity = problemInfo.severity === _ProblemSeverity.ProblemSeverity.ERROR ? ts.DiagnosticCategory.Error : ts.DiagnosticCategory.Warning;
  return makeDiag(severity, LINTER_MSG_CODE_START /* + problemInfo.ruleTag */, srcFile, problemInfo.start, problemInfo.end - problemInfo.start + 1, problemInfo.rule);
}
function runArkTSLinter(tsBuilderProgram, srcFile, buildInfoWriteFile, arkTSVersion, needAutoFix, isUseRtLogic) {
  var _this = this;
  const diagnostics = [];
  const tscDiagnosticsLinter = new _SdkTSCCompiledProgram.SdkTSCCompiledProgram(tsBuilderProgram);
  const program = tscDiagnosticsLinter.getProgram();
  const incrementalLinterState = new _incrementalLinter.IncrementalLinterState(tsBuilderProgram, arkTSVersion);
  incrementalLinterState.updateProgramStateArkTSVersion(arkTSVersion);
  const timePrinterInstance = _ArkTSTimePrinter.ArkTSLinterTimePrinter.getInstance();
  timePrinterInstance.appendTime(_ArkTSTimePrinter.TimePhase.INIT);
  tscDiagnosticsLinter.updateCompilationDiagnostics();
  const srcFiles = getSrcFiles(program, srcFile);
  _Logger.Logger.info("getTscDiagnostics");
  const tscStrictDiagnostics = (0, _GetTscDiagnostics.getTscDiagnostics)(tscDiagnosticsLinter, srcFiles.filter(function (file) {
    _newArrowCheck(this, _this);
    return incrementalLinterState.isFileChanged(file);
  }.bind(this)));
  timePrinterInstance.appendTime(_ArkTSTimePrinter.TimePhase.GET_TSC_DIAGNOSTICS);
  const etsLoaderPath = program.getCompilerOptions().etsLoaderPath;
  const tsImportSendableEnable = program.getCompilerOptions().tsImportSendableEnable;
  _Logger.Logger.info("createTypeScriptLinter");
  const linter = createTypeScriptLinter(program, tscStrictDiagnostics, needAutoFix, isUseRtLogic);
  const interopTypescriptLinter = createInteropTypescriptLinter(program, tsBuilderProgram.getCompilerOptions(), isUseRtLogic);
  _Logger.Logger.info("processFiles");
  processFiles(incrementalLinterState, linter, interopTypescriptLinter, srcFiles, tscStrictDiagnostics, diagnostics, etsLoaderPath, tsImportSendableEnable);
  timePrinterInstance.appendTime(_ArkTSTimePrinter.TimePhase.LINT);
  if (buildInfoWriteFile) {
    _incrementalLinter.IncrementalLinterState.emitBuildInfo(buildInfoWriteFile, tscDiagnosticsLinter.getBuilderProgram());
    timePrinterInstance.appendTime(_ArkTSTimePrinter.TimePhase.EMIT_BUILD_INFO);
  }
  releaseResources();
  return diagnostics;
}
function processFiles(incrementalLinterState, linter, interopTypescriptLinter, srcFiles, tscStrictDiagnostics, diagnostics, etsLoaderPath, tsImportSendableEnable) {
  for (const fileToLint of srcFiles) {
    const scriptKind = (0, _GetScriptKind.getScriptKind)(fileToLint);
    if (scriptKind !== ts.ScriptKind.ETS && scriptKind !== ts.ScriptKind.TS) {
      continue;
    }
    const currentDiagnostics = getDiagnostic(incrementalLinterState, linter, interopTypescriptLinter, fileToLint, tscStrictDiagnostics, scriptKind, etsLoaderPath, tsImportSendableEnable);
    diagnostics.push(...currentDiagnostics);
    incrementalLinterState.updateDiagnostics(fileToLint, currentDiagnostics);
  }
}
function getDiagnostic(incrementalLinterState, linter, interopTypescriptLinter, fileToLint, tscStrictDiagnostics, scriptKind, etsLoaderPath, tsImportSendableEnable) {
  var _this2 = this;
  let currentDiagnostics = [];
  if (incrementalLinterState.isFileChanged(fileToLint)) {
    if (scriptKind === ts.ScriptKind.ETS) {
      _Logger.Logger.info("lint==  ets==fileToLint====" + fileToLint.fileName);
      linter.lint(fileToLint);

      // Get list of bad nodes from the current run.
      currentDiagnostics = tscStrictDiagnostics.get(path.normalize(fileToLint.fileName)) ?? [];
      linter.problemsInfos.forEach(function (x) {
        _newArrowCheck(this, _this2);
        return currentDiagnostics.push(translateDiag(fileToLint, x));
      }.bind(this));
    } else {
      _Logger.Logger.info("lint==  ts==fileToLint====" + fileToLint.fileName);
      const isKit = path.basename(fileToLint.fileName).toLowerCase().indexOf('@kit.') === 0;
      const isInOhModules = ts.isOHModules(fileToLint.fileName);
      if (isKit || isInOhModules) {
        _Logger.Logger.info("isKit || isInOhModules====");
        return currentDiagnostics;
      }
      let isInSdk = false;
      if (!!etsLoaderPath) {
        const a = path.normalize(fileToLint.fileName);
        // @ts-ignore
        const b = ts.resolvePath(etsLoaderPath, '../..');
        const c = a.indexOf(b);
        isInSdk = c === 0;
      }
      // const isInSdk = etsLoaderPath ?
      //   path.normalize(fileToLint.fileName).indexOf(path.resolve(etsLoaderPath, '../..')) === 0 :
      //   false;
      if (!tsImportSendableEnable && !isInSdk) {
        _Logger.Logger.info("isInSdk====" + isInSdk);
        _Logger.Logger.info("tsImportSendableEnable====" + tsImportSendableEnable);
        return currentDiagnostics;
      }
      interopTypescriptLinter.lint(fileToLint);
      interopTypescriptLinter.problemsInfos.forEach(function (x) {
        _newArrowCheck(this, _this2);
        return currentDiagnostics.push(translateDiag(fileToLint, x));
      }.bind(this));
    }
  } else {
    // Get diagnostics from old run.
    currentDiagnostics = incrementalLinterState.getOldDiagnostics(fileToLint);
  }
  return currentDiagnostics;
}
function getSrcFiles(program, srcFile) {
  let srcFiles = [];
  if (srcFile) {
    srcFiles.push(srcFile);
  } else {
    srcFiles = program.getSourceFiles();
  }
  return srcFiles;
}
function createTypeScriptLinter(program, tscStrictDiagnostics, needAutoFix, isUseRtLogic) {
  _TypeScriptLinter.TypeScriptLinter.initGlobals();
  _Logger.Logger.info("lint====needAutoFix====" + !!needAutoFix);
  return new _TypeScriptLinter.TypeScriptLinter(program.getLinterTypeChecker(), !!needAutoFix, !!isUseRtLogic, undefined, undefined, tscStrictDiagnostics);
}
function createInteropTypescriptLinter(program, compileOptions, isUseRtLogic) {
  return new _InteropTypescriptLinter.InteropTypescriptLinter(program.getLinterTypeChecker(), compileOptions, undefined, !!isUseRtLogic);
}

// Reclaim memory for Hvigor with "no-parallel" and "daemon".
function releaseResources() {}
//# sourceMappingURL=RunArkTSLinter.js.map