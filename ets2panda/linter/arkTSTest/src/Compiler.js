"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.compileLintOptions = compileLintOptions;
var ts = _interopRequireWildcard(require("typescript"));
var _FormTscOptions = require("./ts-compiler/FormTscOptions");
var _LogTscDiagnostic = require("../lib/utils/functions/LogTscDiagnostic");
var _TypeScriptLinter = require("../lib/TypeScriptLinter");
var _TSCCompiledProgram = require("../lib/ts-diagnostics/TSCCompiledProgram");
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
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

function compile(cmdOptions, overrideCompilerOptions) {
  const createProgramOptions = (0, _FormTscOptions.formTscOptions)(cmdOptions, overrideCompilerOptions);
  const program = ts.createProgram(createProgramOptions);
  // Log Tsc errors if needed
  if (cmdOptions.logTscErrors) {
    const diagnostics = ts.getPreEmitDiagnostics(program);
    (0, _LogTscDiagnostic.logTscDiagnostic)(diagnostics, _TypeScriptLinter.consoleLog);
  }
  return program;
}
function compileLintOptions(cmdOptions, enableCheckTsFile) {
  const strict = compile(cmdOptions, getOverrideCompilerOptions(true));
  const nonStrict = compile(cmdOptions, getOverrideCompilerOptions(false));
  return {
    cmdOptions: cmdOptions,
    tscCompiledProgram: new _TSCCompiledProgram.TSCCompiledProgramWithDiagnostics(strict, nonStrict, cmdOptions.inputFiles),
    isEtsFile: !enableCheckTsFile
  };
}
function getOverrideCompilerOptions(strict) {
  return {
    strict: false,
    alwaysStrict: false,
    noImplicitAny: false,
    noImplicitThis: false,
    strictBindCallApply: false,
    useUnknownInCatchVariables: false,
    strictNullChecks: strict,
    strictFunctionTypes: strict,
    strictPropertyInitialization: strict,
    noImplicitReturns: strict
  };
}
//# sourceMappingURL=Compiler.js.map