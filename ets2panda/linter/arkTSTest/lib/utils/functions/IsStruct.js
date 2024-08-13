"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.isStruct = isStruct;
exports.isStructDeclaration = isStructDeclaration;
exports.isStructDeclarationKind = isStructDeclarationKind;
var _TypeScriptLinterConfig = require("../../TypeScriptLinterConfig");
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

function isStruct(symbol) {
  if (!symbol.declarations) {
    return false;
  }
  for (const decl of symbol.declarations) {
    if (isStructDeclaration(decl)) {
      return true;
    }
  }
  return false;
}
function isStructDeclarationKind(kind) {
  return _TypeScriptLinterConfig.LinterConfig.tsSyntaxKindNames[kind] === 'StructDeclaration';
}
function isStructDeclaration(node) {
  return isStructDeclarationKind(node.kind);
}
//# sourceMappingURL=IsStruct.js.map