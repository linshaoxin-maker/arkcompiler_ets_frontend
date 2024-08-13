"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.lint = lint;
var _TypeScriptLinter = require("./TypeScriptLinter");
var _InteropTypescriptLinter = require("./InteropTypescriptLinter");
var _Problems = require("./Problems");
var _FaultDesc = require("./FaultDesc");
var _FaultAttrs = require("./FaultAttrs");
var path = _interopRequireWildcard(require("node:path"));
var _MergeArrayMaps = require("./utils/functions/MergeArrayMaps");
var _GetTscDiagnostics = require("./ts-diagnostics/GetTscDiagnostics");
var _TransformTscDiagnostics = require("./ts-diagnostics/TransformTscDiagnostics");
var _ArktsIgnorePaths = require("./utils/consts/ArktsIgnorePaths");
var _PathHelper = require("./utils/functions/PathHelper");
var _ProblemSeverity = require("./ProblemSeverity");
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
function _newArrowCheck(n, r) { if (n !== r) throw new TypeError("Cannot instantiate an arrow function"); } /*
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
function prepareInputFilesList(cmdOptions) {
  var _this = this;
  let inputFiles = cmdOptions.inputFiles;
  if (cmdOptions.parsedConfigFile) {
    inputFiles = cmdOptions.parsedConfigFile.fileNames;
    if (cmdOptions.inputFiles.length > 0) {
      /*
       * Apply linter only to the project source files that are specified
       * as a command-line arguments. Other source files will be discarded.
       */
      const cmdInputsResolvedPaths = cmdOptions.inputFiles.map(function (x) {
        _newArrowCheck(this, _this);
        return path.resolve(x);
      }.bind(this));
      const configInputsResolvedPaths = inputFiles.map(function (x) {
        _newArrowCheck(this, _this);
        return path.resolve(x);
      }.bind(this));
      inputFiles = configInputsResolvedPaths.filter(function (x) {
        var _this2 = this;
        _newArrowCheck(this, _this);
        return cmdInputsResolvedPaths.some(function (y) {
          _newArrowCheck(this, _this2);
          return x === y;
        }.bind(this));
      }.bind(this));
    }
  }
  return inputFiles;
}
function countProblems(linter) {
  let errorNodesTotal = 0;
  let warningNodes = 0;
  for (let i = 0; i < _Problems.FaultID.LAST_ID; i++) {
    switch (_FaultAttrs.faultsAttrs[i].severity) {
      case _ProblemSeverity.ProblemSeverity.ERROR:
        errorNodesTotal += linter.nodeCounters[i];
        break;
      case _ProblemSeverity.ProblemSeverity.WARNING:
        warningNodes += linter.nodeCounters[i];
        break;
    }
  }
  return [errorNodesTotal, warningNodes];
}
function lint(options) {
  var _this3 = this;
  const cmdOptions = options.cmdOptions;
  const cancellationToken = options.cancellationToken;
  const tscCompiledProgram = options.tscCompiledProgram;
  const tsProgram = tscCompiledProgram.getProgram();

  // Prepare list of input files for linter and retrieve AST for those files.
  let inputFiles = prepareInputFilesList(cmdOptions);
  inputFiles = inputFiles.filter(function (input) {
    _newArrowCheck(this, _this3);
    return shouldProcessFile(options, input);
  }.bind(this));
  const srcFiles = [];
  for (const inputFile of inputFiles) {
    const srcFile = tsProgram.getSourceFile(inputFile);
    if (srcFile) {
      srcFiles.push(srcFile);
    }
  }
  const tscStrictDiagnostics = (0, _GetTscDiagnostics.getTscDiagnostics)(tscCompiledProgram, srcFiles);
  const linter = options.isEtsFile ? new _TypeScriptLinter.TypeScriptLinter(tsProgram.getTypeChecker(), cmdOptions.enableAutofix, !!cmdOptions.enableUseRtLogic, cancellationToken, options.incrementalLintInfo, tscStrictDiagnostics, options.reportAutofixCb, options.isEtsFileCb) : new _InteropTypescriptLinter.InteropTypescriptLinter(tsProgram.getTypeChecker(), tsProgram.getCompilerOptions(), options.incrementalLintInfo, !!cmdOptions.enableUseRtLogic);
  const {
    errorNodes,
    problemsInfos
  } = lintFiles(srcFiles, linter);
  (0, _TypeScriptLinter.consoleLog)('\n\n\nFiles scanned: ', srcFiles.length);
  (0, _TypeScriptLinter.consoleLog)('\nFiles with problems: ', errorNodes);
  const [errorNodesTotal, warningNodes] = countProblems(linter);
  logTotalProblemsInfo(errorNodesTotal, warningNodes, linter);
  logProblemsPercentageByFeatures(linter);
  return {
    errorNodes: errorNodesTotal,
    problemsInfos: (0, _MergeArrayMaps.mergeArrayMaps)(problemsInfos, (0, _TransformTscDiagnostics.transformTscDiagnostics)(tscStrictDiagnostics))
  };
}
function lintFiles(srcFiles, linter) {
  let problemFiles = 0;
  const problemsInfos = new Map();
  for (const srcFile of srcFiles) {
    const prevVisitedNodes = linter.totalVisitedNodes;
    const prevErrorLines = linter.totalErrorLines;
    const prevWarningLines = linter.totalWarningLines;
    linter.errorLineNumbersString = '';
    linter.warningLineNumbersString = '';
    const nodeCounters = [];
    for (let i = 0; i < _Problems.FaultID.LAST_ID; i++) {
      nodeCounters[i] = linter.nodeCounters[i];
    }
    linter.lint(srcFile);
    // save results and clear problems array
    problemsInfos.set(path.normalize(srcFile.fileName), [...linter.problemsInfos]);
    linter.problemsInfos.length = 0;

    // print results for current file
    const fileVisitedNodes = linter.totalVisitedNodes - prevVisitedNodes;
    const fileErrorLines = linter.totalErrorLines - prevErrorLines;
    const fileWarningLines = linter.totalWarningLines - prevWarningLines;
    problemFiles = countProblemFiles(nodeCounters, problemFiles, srcFile, fileVisitedNodes, fileErrorLines, fileWarningLines, linter);
  }
  return {
    errorNodes: problemFiles,
    problemsInfos: problemsInfos
  };
}
function countProblemFiles(nodeCounters, filesNumber, tsSrcFile, fileNodes, fileErrorLines, fileWarningLines, linter) {
  let errorNodes = 0;
  let warningNodes = 0;
  for (let i = 0; i < _Problems.FaultID.LAST_ID; i++) {
    const nodeCounterDiff = linter.nodeCounters[i] - nodeCounters[i];
    switch (_FaultAttrs.faultsAttrs[i].severity) {
      case _ProblemSeverity.ProblemSeverity.ERROR:
        errorNodes += nodeCounterDiff;
        break;
      case _ProblemSeverity.ProblemSeverity.WARNING:
        warningNodes += nodeCounterDiff;
        break;
    }
  }
  if (errorNodes > 0) {
    filesNumber++;
    const errorRate = (errorNodes / fileNodes * 100).toFixed(2);
    const warningRate = (warningNodes / fileNodes * 100).toFixed(2);
    (0, _TypeScriptLinter.consoleLog)(tsSrcFile.fileName, ': ', '\n\tError lines: ', linter.errorLineNumbersString);
    (0, _TypeScriptLinter.consoleLog)(tsSrcFile.fileName, ': ', '\n\tWarning lines: ', linter.warningLineNumbersString);
    (0, _TypeScriptLinter.consoleLog)('\n\tError constructs (%): ', errorRate, '\t[ of ', fileNodes, ' constructs ], \t', fileErrorLines, ' lines');
    (0, _TypeScriptLinter.consoleLog)('\n\tWarning constructs (%): ', warningRate, '\t[ of ', fileNodes, ' constructs ], \t', fileWarningLines, ' lines');
  }
  return filesNumber;
}
function logTotalProblemsInfo(errorNodes, warningNodes, linter) {
  const errorRate = (errorNodes / linter.totalVisitedNodes * 100).toFixed(2);
  const warningRate = (warningNodes / linter.totalVisitedNodes * 100).toFixed(2);
  (0, _TypeScriptLinter.consoleLog)('\nTotal error constructs (%): ', errorRate);
  (0, _TypeScriptLinter.consoleLog)('\nTotal warning constructs (%): ', warningRate);
  (0, _TypeScriptLinter.consoleLog)('\nTotal error lines:', linter.totalErrorLines, ' lines\n');
  (0, _TypeScriptLinter.consoleLog)('\nTotal warning lines:', linter.totalWarningLines, ' lines\n');
}
function logProblemsPercentageByFeatures(linter) {
  (0, _TypeScriptLinter.consoleLog)('\nPercent by features: ');
  for (let i = 0; i < _Problems.FaultID.LAST_ID; i++) {
    const nodes = linter.nodeCounters[i];
    const lines = linter.lineCounters[i];
    const pecentage = (nodes / linter.totalVisitedNodes * 100).toFixed(2).padEnd(7, ' ');
    (0, _TypeScriptLinter.consoleLog)(_FaultDesc.faultDesc[i].padEnd(55, ' '), pecentage, '[', nodes, ' constructs / ', lines, ' lines]');
  }
}
function shouldProcessFile(options, fileFsPath) {
  var _this4 = this;
  if (_ArktsIgnorePaths.ARKTS_IGNORE_FILES.some(function (ignore) {
    _newArrowCheck(this, _this4);
    return path.basename(fileFsPath) === ignore;
  }.bind(this))) {
    return false;
  }
  if (_ArktsIgnorePaths.ARKTS_IGNORE_DIRS_NO_OH_MODULES.some(function (ignore) {
    _newArrowCheck(this, _this4);
    return (0, _PathHelper.pathContainsDirectory)(path.resolve(fileFsPath), ignore);
  }.bind(this))) {
    return false;
  }
  return !(0, _PathHelper.pathContainsDirectory)(path.resolve(fileFsPath), _ArktsIgnorePaths.ARKTS_IGNORE_DIRS_OH_MODULES) || !!options.isFileFromModuleCb?.(fileFsPath);
}
//# sourceMappingURL=LinterRunner.js.map