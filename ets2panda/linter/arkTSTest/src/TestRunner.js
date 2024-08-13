"use strict";

var _Logger = require("../lib/Logger");
var _LoggerImpl = require("./LoggerImpl");
var _TypeScriptLinter = require("../lib/TypeScriptLinter");
var _InteropTypescriptLinter = require("../lib/InteropTypescriptLinter");
var _LinterRunner = require("../lib/LinterRunner");
var _CommandLineParser = require("./CommandLineParser");
var fs = _interopRequireWildcard(require("node:fs"));
var path = _interopRequireWildcard(require("node:path"));
var ts = _interopRequireWildcard(require("typescript"));
var _Compiler = require("./Compiler");
var _RunArkTSLinter = require("../sdk/linter_1_1/RunArkTSLinter");
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
_Logger.Logger.init(new _LoggerImpl.LoggerImpl());
let enableUseRtLogic = true;
let enableCheckTsFile = false;
let checkArkTSRunner = false;
const TEST_DIR = 'test';
const TAB = '    ';
// initial configuration
const options = ts.readConfigFile('./arkTSTest/tsconfig.json', ts.sys.readFile).config.compilerOptions;
const allPath = ['*'];
Object.assign(options, {
  emitNodeModulesFiles: true,
  importsNotUsedAsValues: ts.ImportsNotUsedAsValues.Preserve,
  module: ts.ModuleKind.ES2020,
  moduleResolution: ts.ModuleResolutionKind.NodeJs,
  noEmit: true,
  target: ts.ScriptTarget.ES2021,
  baseUrl: '/',
  paths: {
    '*': allPath
  },
  lib: ['lib.es2021.d.ts'],
  types: [],
  etsLoaderPath: 'null_sdkPath'
});
var Mode = /*#__PURE__*/function (Mode) {
  Mode[Mode["DEFAULT"] = 0] = "DEFAULT";
  Mode[Mode["AUTOFIX"] = 1] = "AUTOFIX";
  return Mode;
}(Mode || {});
const RESULT_EXT = [];
RESULT_EXT[Mode.DEFAULT] = '.json';
RESULT_EXT[Mode.AUTOFIX] = '.autofix.json';
const AUTOFIX_SKIP_EXT = '.autofix.skip';
const ARGS_CONFIG_EXT = '.args.json';
const DIFF_EXT = '.diff';
function runTests() {
  var _this = this;
  /*
   * Set the IDE mode manually to enable storing information
   * about found bad nodes and also disable the log output.
   */
  _TypeScriptLinter.TypeScriptLinter.ideMode = true;
  _TypeScriptLinter.TypeScriptLinter.testMode = true;
  _InteropTypescriptLinter.InteropTypescriptLinter.ideMode = true;
  _InteropTypescriptLinter.InteropTypescriptLinter.testMode = true;
  let hasComparisonFailures = false;
  let passed = 0;
  let failed = 0;
  const testDirs = getParam();
  // Get tests from test directory
  for (const testDir of testDirs) {
    const testFiles = fs.readdirSync(testDir).filter(function (x) {
      _newArrowCheck(this, _this);
      return x.trimEnd().endsWith(ts.Extension.Ts) && !x.trimEnd().endsWith(ts.Extension.Dts) || x.trimEnd().endsWith(ts.Extension.Tsx) || x.trimEnd().endsWith(ts.Extension.Ets);
    }.bind(this));
    _Logger.Logger.info(`\nProcessing "${testDir}" directory:\n`);
    // Run each test in Default and Autofix mode:
    for (const testFile of testFiles) {
      if (runTest(testDir, testFile, Mode.DEFAULT)) {
        failed++;
        hasComparisonFailures = true;
      } else {
        passed++;
      }
      if (!checkArkTSRunner && runTest(testDir, testFile, Mode.AUTOFIX)) {
        failed++;
        hasComparisonFailures = true;
      } else {
        passed++;
      }
    }
  }
  _Logger.Logger.info(`\nSUMMARY: ${passed + failed} total, ${passed} passed or skipped, ${failed} failed.`);
  _Logger.Logger.info(failed > 0 ? '\nTEST FAILED' : '\nTEST SUCCESSFUL');
  process.exit(hasComparisonFailures ? -1 : 0);
}
function getParam() {
  let pathArg = null;
  for (const key of process.argv) {
    if (key.includes('-P:')) {
      pathArg = key.replace('-P:', '').split(',');
    }
    if (key.includes('-SDK')) {
      enableUseRtLogic = false;
    }
    if (key.includes('-CheckTs')) {
      enableCheckTsFile = true;
    }
    if (key.includes('-CheckArkTsRunner')) {
      checkArkTSRunner = true;
    }
  }
  if (!pathArg?.length) {
    pathArg = [TEST_DIR];
  }
  return pathArg;
}
function parseArgs(testDir, testFile, mode) {
  // Configure test parameters and run linter.
  const args = [path.join(testDir, testFile)];
  const argsFileName = path.join(testDir, testFile + ARGS_CONFIG_EXT);
  if (fs.existsSync(argsFileName)) {
    const data = fs.readFileSync(argsFileName).toString();
    const args = JSON.parse(data);
    if (args.testMode !== undefined) {
      _TypeScriptLinter.TypeScriptLinter.testMode = args.testMode;
      _InteropTypescriptLinter.InteropTypescriptLinter.testMode = args.testMode;
    }
  }
  if (mode === Mode.AUTOFIX) {
    args.push('--autofix');
  }
  if (enableUseRtLogic) {
    args.push('--enableUseRtLogic');
  }
  return (0, _CommandLineParser.parseCommandLine)(args);
}
function compareExpectedAndActual(testDir, testFile, mode, resultNodes) {
  var _this2 = this;
  // Read file with expected test result.
  let expectedResult;
  let diff = '';
  const resultExt = RESULT_EXT[mode];
  const testResultFileName = testFile + resultExt;
  try {
    const expectedResultFile = fs.readFileSync(path.join(testDir, testResultFileName)).toString();
    expectedResult = JSON.parse(expectedResultFile);

    /**
     * The exclusive field is added to identify whether the use case is exclusive to the RT or SDK
     * RT means the RT exclusive
     * SDK means the SDK exclusive
     * undefined means shared
     */
    expectedResult.nodes = expectedResult?.nodes.filter(function (x) {
      _newArrowCheck(this, _this2);
      return !x?.exclusive || x.exclusive === (enableUseRtLogic ? 'RT' : 'SDK');
    }.bind(this));
    if (!expectedResult?.nodes || expectedResult.nodes.length !== resultNodes.length) {
      const expectedResultCount = expectedResult?.nodes ? expectedResult.nodes.length : 0;
      diff = `Expected count: ${expectedResultCount} vs actual count: ${resultNodes.length}`;
      _Logger.Logger.info(`${TAB}${diff}`);
    } else {
      diff = expectedAndActualMatch(expectedResult.nodes, resultNodes);
    }
    if (diff) {
      _Logger.Logger.info(`${TAB}Test failed. Expected and actual results differ.`);
    }
  } catch (error) {
    _Logger.Logger.info(`${TAB}Test failed. ` + error);
  }
  return diff;
}
function runTest(testDir, testFile, mode) {
  var _this3 = this;
  if (mode === Mode.AUTOFIX && fs.existsSync(path.join(testDir, testFile + AUTOFIX_SKIP_EXT))) {
    _Logger.Logger.info(`Skipping test ${testFile} (${Mode[mode]} mode)`);
    return false;
  }
  _Logger.Logger.info(`Running test ${testFile} (${Mode[mode]} mode)`);
  _TypeScriptLinter.TypeScriptLinter.initGlobals();
  const currentTestMode = _TypeScriptLinter.TypeScriptLinter.testMode;
  const currentInteropTestMode = _InteropTypescriptLinter.InteropTypescriptLinter.testMode;
  const cmdOptions = parseArgs(testDir, testFile, mode);
  let resultNodes = [];
  _Logger.Logger.info(`checkArkTSRunner===========${checkArkTSRunner}`);
  if (checkArkTSRunner) {
    const result = runLinter(testDir, testFile, mode).filter(function (x) {
      _newArrowCheck(this, _this3);
      return !!x.file && !!x.start;
    }.bind(this));
    _Logger.Logger.info(`result===========${result.length}`);
    if (!result || result.length === 0) {
      _Logger.Logger.info(`return===========${resultNodes.length}`);
      return true;
    }
    resultNodes = result.map(function (x) {
      _newArrowCheck(this, _this3);
      const lineAndCharacter = x.file?.getLineAndCharacterOfPosition(x.start ? x.start : 0);
      _Logger.Logger.info(`start=${x.start}; messageText=${x.messageText}; `);
      _Logger.Logger.info(`line=${lineAndCharacter?.line}; column=${!!lineAndCharacter?.character}; `);
      return {
        line: !!lineAndCharacter?.line ? lineAndCharacter.line + 1 : 1,
        column: !!lineAndCharacter?.character ? lineAndCharacter.character + 1 : 1,
        problem: undefined,
        autofix: undefined,
        suggest: undefined,
        rule: typeof x.messageText === 'string' ? x.messageText : x.messageText.messageText
      };
    }.bind(this));
    _Logger.Logger.info(`resultNodes===========${resultNodes.length}`);
  } else {
    const result = (0, _LinterRunner.lint)((0, _Compiler.compileLintOptions)(cmdOptions, enableCheckTsFile));
    const fileProblems = result.problemsInfos.get(path.normalize(cmdOptions.inputFiles[0]));
    if (fileProblems === undefined) {
      return true;
    }
    _TypeScriptLinter.TypeScriptLinter.testMode = currentTestMode;
    _InteropTypescriptLinter.InteropTypescriptLinter.testMode = currentInteropTestMode;

    // Get list of bad nodes from the current run.
    resultNodes = fileProblems.map(function (x) {
      _newArrowCheck(this, _this3);
      _Logger.Logger.info(`start=${x.start}; messageText=${x.rule}; `);
      _Logger.Logger.info(`line=${x.line}; column=${x.column}; `);
      return {
        line: x.line,
        column: x.column,
        problem: x.problem,
        autofix: mode === Mode.AUTOFIX ? x.autofix : undefined,
        suggest: x.suggest,
        rule: x.rule
      };
    }.bind(this));
  }

  // Read file with expected test result.
  const testResult = compareExpectedAndActual(testDir, testFile, mode, resultNodes);

  // Write file with actual test results.
  writeActualResultFile(testDir, testFile, mode, resultNodes, testResult);
  return !!testResult;
}
function runLinter(rootName, testFile, mode) {
  const nonStrictCheckParam = {
    allowJS: true,
    checkJs: false
  };
  Object.assign(options, nonStrictCheckParam);
  const builderProgram = ts.createIncrementalProgramForArkTs({
    rootNames: [path.join(process.cwd(), rootName, testFile)],
    options: options
  });
  _Logger.Logger.info(`builderProgram===========${path.join(process.cwd(), rootName, testFile)}`);
  _Logger.Logger.info(`mode===========${mode}`);
  _Logger.Logger.info(`mode===========${mode === Mode.AUTOFIX}`);
  const result = (0, _RunArkTSLinter.runArkTSLinter)(builderProgram, undefined, undefined, undefined, mode === Mode.AUTOFIX, enableUseRtLogic);
  _Logger.Logger.info(`runLinter===========${result.length}`);
  return result;
}
function expectedAndActualMatch(expectedNodes, actualNodes) {
  // Compare expected and actual results.
  for (let i = 0; i < actualNodes.length; i++) {
    const actual = actualNodes[i];
    const expect = expectedNodes[i];
    if (actual.line !== expect.line || actual.column !== expect.column || !checkArkTSRunner && actual.problem !== expect.problem) {
      _Logger.Logger.info(`reportDiff===========11111`);
      return reportDiff(expect, actual);
    }
    if (!autofixArraysMatch(expect.autofix, actual.autofix)) {
      _Logger.Logger.info(`reportDiff===========22222`);
      return reportDiff(expect, actual);
    }
    if (!checkArkTSRunner && expect.suggest && actual.suggest !== expect.suggest) {
      _Logger.Logger.info(`reportDiff===========33333`);
      return reportDiff(expect, actual);
    }
    if (expect.rule && actual.rule !== expect.rule) {
      _Logger.Logger.info(`reportDiff===========44444`);
      return reportDiff(expect, actual);
    }
  }
  return '';
}
function autofixArraysMatch(expected, actual) {
  if (!expected && !actual) {
    return true;
  }
  if (!(expected && actual) || expected.length !== actual.length) {
    return false;
  }
  for (let i = 0; i < actual.length; ++i) {
    if (actual[i].start !== expected[i].start || actual[i].end !== expected[i].end || actual[i].replacementText !== expected[i].replacementText) {
      return false;
    }
  }
  return true;
}
function writeActualResultFile(testDir, testFile, mode, resultNodes, diff) {
  const actualResultsDir = path.join(testDir, enableUseRtLogic ? 'results_rt' : 'results_sdk');
  const resultExt = RESULT_EXT[mode];
  if (!fs.existsSync(actualResultsDir)) {
    fs.mkdirSync(actualResultsDir);
  }
  const actualResultJSON = JSON.stringify({
    nodes: resultNodes
  }, null, 4);
  fs.writeFileSync(path.join(actualResultsDir, testFile + resultExt), actualResultJSON);
  if (diff) {
    fs.writeFileSync(path.join(actualResultsDir, testFile + resultExt + DIFF_EXT), diff);
  }
}
function reportDiff(expected, actual) {
  const expectedNode = JSON.stringify({
    nodes: [expected]
  }, null, 4);
  const actualNode = JSON.stringify({
    nodes: [actual]
  }, null, 4);
  const diff = `Expected:
${expectedNode}
Actual:
${actualNode}`;
  _Logger.Logger.info(diff);
  return diff;
}
runTests();
//# sourceMappingURL=TestRunner.js.map