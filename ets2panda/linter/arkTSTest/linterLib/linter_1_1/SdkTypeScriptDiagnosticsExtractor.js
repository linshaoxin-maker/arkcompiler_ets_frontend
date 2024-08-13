"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.getStrictDiagnostics = getStrictDiagnostics;
function _newArrowCheck(n, r) { if (n !== r) throw new TypeError("Cannot instantiate an arrow function"); }
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

/**
 * Returns diagnostics which appear in strict compilation mode only
 */
function getStrictDiagnostics(builderProgram, fileName, cancellationToken) {
  var _this = this;
  // applying filter is a workaround for tsc bug
  const strict = getAllDiagnostics(builderProgram, fileName, cancellationToken, true).filter(function (diag) {
    _newArrowCheck(this, _this);
    return !(diag.length === 0 && diag.start === 0);
  }.bind(this));
  const nonStrict = getAllDiagnostics(builderProgram, fileName, cancellationToken, false);

  // collect hashes for later easier comparison
  const nonStrictHashes = nonStrict.reduce(function (result, value) {
    _newArrowCheck(this, _this);
    const hash = hashDiagnostic(value);
    if (hash) {
      result.add(hash);
    }
    return result;
  }.bind(this), new Set());
  // return diagnostics which weren't detected in non-strict mode
  return strict.filter(function (value) {
    _newArrowCheck(this, _this);
    const hash = hashDiagnostic(value);
    return hash && !nonStrictHashes.has(hash);
  }.bind(this));
}
function getAllDiagnostics(builderProgram, fileName, cancellationToken, isStrict = false) {
  var _this2 = this;
  const sourceFile = builderProgram.getSourceFile(fileName);
  const syntacticDiagnostics = builderProgram.getSyntacticDiagnostics(sourceFile, cancellationToken);
  if (isStrict) {
    return !builderProgram.builderProgramForLinter ? [] : builderProgram.builderProgramForLinter.getSemanticDiagnostics(sourceFile, cancellationToken).concat(syntacticDiagnostics).filter(function (diag) {
      _newArrowCheck(this, _this2);
      return diag.file === sourceFile;
    }.bind(this));
  }
  return builderProgram.getSemanticDiagnostics(sourceFile, cancellationToken).concat(syntacticDiagnostics).filter(function (diag) {
    _newArrowCheck(this, _this2);
    return diag.file === sourceFile;
  }.bind(this));
}
function hashDiagnostic(diagnostic) {
  if (diagnostic.start === undefined || diagnostic.length === undefined) {
    return undefined;
  }
  return `${diagnostic.code}%${diagnostic.start}%${diagnostic.length}`;
}
//# sourceMappingURL=SdkTypeScriptDiagnosticsExtractor.js.map