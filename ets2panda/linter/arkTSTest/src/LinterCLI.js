"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.run = run;
var _TypeScriptLinter = require("../lib/TypeScriptLinter");
var _CommandLineParser = require("./CommandLineParser");
var _Logger = require("../lib/Logger");
var fs = _interopRequireWildcard(require("node:fs"));
var os = _interopRequireWildcard(require("node:os"));
var readline = _interopRequireWildcard(require("node:readline"));
var path = _interopRequireWildcard(require("node:path"));
var _LinterRunner = require("../lib/LinterRunner");
var _Compiler = require("./Compiler");
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
function run() {
  const commandLineArgs = process.argv.slice(2);
  if (commandLineArgs.length === 0) {
    _Logger.Logger.info('Command line error: no arguments');
    process.exit(-1);
  }
  const cmdOptions = (0, _CommandLineParser.parseCommandLine)(commandLineArgs);
  if (cmdOptions.testMode) {
    _TypeScriptLinter.TypeScriptLinter.testMode = true;
  }
  _TypeScriptLinter.TypeScriptLinter.initGlobals();
  if (!cmdOptions.ideMode) {
    const result = (0, _LinterRunner.lint)((0, _Compiler.compileLintOptions)(cmdOptions));
    process.exit(result.errorNodes > 0 ? 1 : 0);
  } else {
    runIDEMode(cmdOptions);
  }
}
function getTempFileName() {
  return path.join(os.tmpdir(), Math.floor(Math.random() * 10000000).toString() + '_linter_tmp_file.ts');
}
function runIDEMode(cmdOptions) {
  var _this = this;
  _TypeScriptLinter.TypeScriptLinter.ideMode = true;
  const tmpFileName = getTempFileName();
  // read data from stdin
  const writeStream = fs.createWriteStream(tmpFileName, {
    flags: 'w'
  });
  const rl = readline.createInterface({
    input: process.stdin,
    output: writeStream,
    terminal: false
  });
  rl.on('line', function (line) {
    _newArrowCheck(this, _this);
    fs.appendFileSync(tmpFileName, line + '\n');
  }.bind(this));
  rl.once('close', function () {
    var _this2 = this;
    _newArrowCheck(this, _this);
    // end of input
    writeStream.close();
    cmdOptions.inputFiles = [tmpFileName];
    if (cmdOptions.parsedConfigFile) {
      cmdOptions.parsedConfigFile.fileNames.push(tmpFileName);
    }
    const result = (0, _LinterRunner.lint)((0, _Compiler.compileLintOptions)(cmdOptions));
    const problems = Array.from(result.problemsInfos.values());
    if (problems.length === 1) {
      const jsonMessage = problems[0].map(function (x) {
        _newArrowCheck(this, _this2);
        return {
          line: x.line,
          column: x.column,
          start: x.start,
          end: x.end,
          type: x.type,
          suggest: x.suggest,
          rule: x.rule,
          severity: x.severity,
          autofix: x.autofix
        };
      }.bind(this));
      _Logger.Logger.info(`{"linter messages":${JSON.stringify(jsonMessage)}}`);
    } else {
      _Logger.Logger.error('Unexpected error: could not lint file');
    }
    fs.unlinkSync(tmpFileName);
  }.bind(this));
}
//# sourceMappingURL=LinterCLI.js.map