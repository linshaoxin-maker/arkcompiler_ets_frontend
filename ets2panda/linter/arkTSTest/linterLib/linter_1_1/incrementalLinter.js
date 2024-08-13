"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.IncrementalLinterState = void 0;
var ts = _interopRequireWildcard(require("typescript"));
function _getRequireWildcardCache(e) { if ("function" != typeof WeakMap) return null; var r = new WeakMap(), t = new WeakMap(); return (_getRequireWildcardCache = function (e) { return e ? t : r; })(e); }
function _interopRequireWildcard(e, r) { if (!r && e && e.__esModule) return e; if (null === e || "object" != typeof e && "function" != typeof e) return { default: e }; var t = _getRequireWildcardCache(r); if (t && t.has(e)) return t.get(e); var n = { __proto__: null }, a = Object.defineProperty && Object.getOwnPropertyDescriptor; for (var u in e) if ("default" !== u && {}.hasOwnProperty.call(e, u)) { var i = a ? Object.getOwnPropertyDescriptor(e, u) : null; i && (i.get || i.set) ? Object.defineProperty(n, u, i) : n[u] = e[u]; } return n.default = e, t && t.set(e, n), n; }
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
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
class IncrementalLinterState {
  constructor(builderProgram, arkTSVersion) {
    // @ts-ignore
    _defineProperty(this, "changedFiles", new ts.Set());
    // @ts-ignore
    _defineProperty(this, "programState", void 0);
    _defineProperty(this, "oldDiagnostics", void 0);
    // @ts-ignore
    this.programState = builderProgram.getState();
    this.oldDiagnostics = this.programState.arktsLinterDiagnosticsPerFile;
    this.programState.arktsLinterDiagnosticsPerFile = new Map();
    this.changedFiles = IncrementalLinterState.collectChangedFilesFromProgramState(this.programState, arkTSVersion);
  }
  isFileChanged(srcFile) {
    // @ts-ignore
    return this.changedFiles.has(srcFile.resolvedPath);
  }
  getOldDiagnostics(srcFile) {
    // @ts-ignore
    return this.oldDiagnostics?.get(srcFile.resolvedPath) ?? [];
  }
  updateDiagnostics(srcFile, newDiagnostics) {
    // @ts-ignore
    this.programState.arktsLinterDiagnosticsPerFile?.set(srcFile.resolvedPath, newDiagnostics);
  }
  updateProgramStateArkTSVersion(arkTSVersion) {
    this.programState.arkTSVersion = arkTSVersion;
  }
  static emitBuildInfo(buildInfoWriteFile, builderProgram) {
    // @ts-ignore
    builderProgram.emitBuildInfo(buildInfoWriteFile);
  }
  static collectChangedFilesFromProgramState(
  // @ts-ignore
  state, arkTSVersion) {
    /*
     * If old arkTSVersion from last run is not same current arkTSVersion from ets_loader,
     * then process all files in project.
     */
    if (state.arkTSVersion !== arkTSVersion) {
      // @ts-ignore
      return new ts.Set(IncrementalLinterState.arrayFrom(state.fileInfos.keys()));
    }

    // @ts-ignore
    const changedFiles = new ts.Set(state.changedFilesSet);

    /*
     * If any source file that affects global scope has been changed,
     * then process all files in project.
     */
    for (const changedFile of IncrementalLinterState.arrayFrom(changedFiles.keys())) {
      const fileInfo = state.fileInfos.get(changedFile);
      if (fileInfo?.affectsGlobalScope) {
        // @ts-ignore
        return new ts.Set(IncrementalLinterState.arrayFrom(state.fileInfos.keys()));
      }
    }
    if (!state.referencedMap) {
      return changedFiles;
    }

    // @ts-ignore
    const seenPaths = new ts.Set();
    const queue = IncrementalLinterState.arrayFrom(changedFiles.keys());
    while (queue.length) {
      const path = queue.pop();
      if (!seenPaths.has(path)) {
        seenPaths.add(path);

        // Collect all files that import this file
        // @ts-ignore
        queue.push(...ts.BuilderState.getReferencedByPaths(state, path));
      }
    }
    return seenPaths;
  }
  static arrayFrom(iterator) {
    const result = [];
    for (let iterResult = iterator.next(); !iterResult.done; iterResult = iterator.next()) {
      result.push(iterResult.value);
    }
    return result;
  }
}
exports.IncrementalLinterState = IncrementalLinterState;
//# sourceMappingURL=incrementalLinter.js.map