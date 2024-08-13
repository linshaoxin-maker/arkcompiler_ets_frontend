"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.ARKTS_IGNORE_FILES = exports.ARKTS_IGNORE_DIRS_OH_MODULES = exports.ARKTS_IGNORE_DIRS_NO_OH_MODULES = exports.ARKTS_IGNORE_DIRS = void 0;
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

const ARKTS_IGNORE_DIRS_NO_OH_MODULES = exports.ARKTS_IGNORE_DIRS_NO_OH_MODULES = ['node_modules', 'build', '.preview'];
const ARKTS_IGNORE_DIRS_OH_MODULES = exports.ARKTS_IGNORE_DIRS_OH_MODULES = 'oh_modules';
const ARKTS_IGNORE_DIRS = exports.ARKTS_IGNORE_DIRS = [...ARKTS_IGNORE_DIRS_NO_OH_MODULES, ARKTS_IGNORE_DIRS_OH_MODULES];
const ARKTS_IGNORE_FILES = exports.ARKTS_IGNORE_FILES = ['hvigorfile.ts'];
//# sourceMappingURL=ArktsIgnorePaths.js.map