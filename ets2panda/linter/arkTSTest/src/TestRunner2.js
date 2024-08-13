"use strict";

var _Logger = require("../lib/Logger");
var _LoggerImpl = require("./LoggerImpl");
var ts = _interopRequireWildcard(require("typescript"));
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
/*
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
function runTests() {
  const options = ts.readConfigFile('./tsconfig.json', ts.sys.readFile).config.compilerOptions;
  _Logger.Logger.info("options===" + !!options);
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
  _Logger.Logger.info("options2===" + !!options);
}
runTests();
//# sourceMappingURL=TestRunner2.js.map