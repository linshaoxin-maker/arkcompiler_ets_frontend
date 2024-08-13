"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.parseCommandLine = parseCommandLine;
var _Logger = require("../lib/Logger");
var _LogTscDiagnostic = require("../lib/utils/functions/LogTscDiagnostic");
var _commander = require("commander");
var ts = _interopRequireWildcard(require("typescript"));
var fs = _interopRequireWildcard(require("node:fs"));
var path = _interopRequireWildcard(require("node:path"));
var _this = void 0;
/*
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
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
function _newArrowCheck(n, r) { if (n !== r) throw new TypeError("Cannot instantiate an arrow function"); }
const TS_EXT = '.ts';
const TSX_EXT = '.tsx';
const ETS_EXT = '.ets';
let inputFiles;
let responseFile = '';
function addSrcFile(value) {
  if (value.startsWith('@')) {
    responseFile = value;
  } else {
    inputFiles.push(value);
  }
}
const _getFiles = function getFiles(dir) {
  _newArrowCheck(this, _this);
  const resultFiles = [];
  const files = fs.readdirSync(dir);
  for (let i = 0; i < files.length; ++i) {
    const name = path.join(dir, files[i]);
    if (fs.statSync(name).isDirectory()) {
      resultFiles.push(..._getFiles(name));
    } else {
      const extension = path.extname(name);
      if (extension === TS_EXT || extension === TSX_EXT || extension === ETS_EXT) {
        resultFiles.push(name);
      }
    }
  }
  return resultFiles;
}.bind(void 0);
function addProjectFolder(projectFolder, previous) {
  return previous.concat([projectFolder]);
}
function formCommandLineOptions(program) {
  const opts = {
    inputFiles: inputFiles,
    warningsAsErrors: false,
    enableAutofix: false,
    enableUseRtLogic: true
  };
  const options = program.opts();
  if (options.TSC_Errors) {
    opts.logTscErrors = true;
  }
  if (options.devecoPluginMode) {
    opts.ideMode = true;
  }
  if (options.testMode) {
    opts.testMode = true;
  }
  if (options.projectFolder) {
    doProjectFolderArg(options.projectFolder, opts);
  }
  if (options.project) {
    doProjectArg(options.project, opts);
  }
  if (options.autofix) {
    opts.enableAutofix = true;
  }
  if (options.warningsAsErrors) {
    opts.warningsAsErrors = true;
  }
  if (!options.enableUseRtLogic) {
    opts.enableUseRtLogic = false;
  }
  return opts;
}
function parseCommandLine(commandLineArgs) {
  var _this2 = this;
  const program = new _commander.Command();
  program.name('tslinter').description('Linter for TypeScript sources').version('0.0.1');
  program.option('-E, --TSC_Errors', 'show error messages from Tsc').option('--test-mode', 'run linter as if running TS test files').option('--deveco-plugin-mode', 'run as IDE plugin').option('-p, --project <project_file>', 'path to TS project config file').option('--project-folder <project_folder>', 'path to folder containig TS files to verify', addProjectFolder, []).option('--autofix', 'automatically fix problems found by linter').option('--enableUseRtLogic', 'linter with RT').addOption(new _commander.Option('--warnings-as-errors', 'treat warnings as errors').hideHelp(true));
  program.argument('[srcFile...]', 'files to be verified', addSrcFile);
  inputFiles = [];
  // method parse() eats two first args, so make them dummy
  let cmdArgs = ['dummy', 'dummy'];
  cmdArgs.push(...commandLineArgs);
  program.parse(cmdArgs);
  if (responseFile !== '') {
    try {
      commandLineArgs = fs.readFileSync(responseFile.slice(1)).toString().split('\n').filter(function (e) {
        _newArrowCheck(this, _this2);
        return e.trimEnd();
      }.bind(this));
      cmdArgs = ['dummy', 'dummy'];
      cmdArgs.push(...commandLineArgs);
      program.parse(cmdArgs);
    } catch (error) {
      _Logger.Logger.error('Failed to read response file: ' + error);
      process.exit(-1);
    }
  }
  return formCommandLineOptions(program);
}
function doProjectFolderArg(prjFolders, opts) {
  for (let i = 0; i < prjFolders.length; i++) {
    const prjFolderPath = prjFolders[i];
    try {
      opts.inputFiles.push(..._getFiles(prjFolderPath));
    } catch (error) {
      _Logger.Logger.error('Failed to read folder: ' + error);
      process.exit(-1);
    }
  }
}
function doProjectArg(cfgPath, opts) {
  var _this3 = this;
  // Process project file (tsconfig.json) and retrieve config arguments.
  const configFile = cfgPath;
  const host = ts.sys;
  const diagnostics = [];
  try {
    const oldUnrecoverableDiagnostic = host.onUnRecoverableConfigFileDiagnostic;
    host.onUnRecoverableConfigFileDiagnostic = function (diagnostic) {
      _newArrowCheck(this, _this3);
      diagnostics.push(diagnostic);
    }.bind(this);
    opts.parsedConfigFile = ts.getParsedCommandLineOfConfigFile(configFile, {}, host);
    host.onUnRecoverableConfigFileDiagnostic = oldUnrecoverableDiagnostic;
    if (opts.parsedConfigFile) {
      diagnostics.push(...ts.getConfigFileParsingDiagnostics(opts.parsedConfigFile));
    }
    if (diagnostics.length > 0) {
      // Log all diagnostic messages and exit program.
      _Logger.Logger.error('Failed to read config file.');
      (0, _LogTscDiagnostic.logTscDiagnostic)(diagnostics, _Logger.Logger.info);
      process.exit(-1);
    }
  } catch (error) {
    _Logger.Logger.error('Failed to read config file: ' + error);
    process.exit(-1);
  }
}
//# sourceMappingURL=CommandLineParser.js.map