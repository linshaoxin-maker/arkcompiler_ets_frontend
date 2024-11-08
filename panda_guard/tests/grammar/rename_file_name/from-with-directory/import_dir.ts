/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import * as t1 from './custom-ts-dir';
import {CustomClassJs} from './custom-js-dir';
import type {Teacher} from './custom-dts-dir';
import {funcPlus, TempInter} from './file1.Y.Z'

import path from 'path'

let ins = new t1.CustomClass();
print(ins.propName === 'ts-default-name', 'success')
print(ins.propAge === 'ts-default-age', 'success')
let insJs = new CustomClassJs();
print(insJs.propNameJs === 'js-default-name', 'success')
print(insJs.propAgeJs === 'js-default-age', 'success')
let Moli: Teacher;
print(funcPlus(1, 2) === 3, 'success');

const fileName: string = path.basename(__filename);
print(fileName !== 'import_dir.ts', 'success');

class TempClass implements TempInter {
  method(): void {
    throw new Error('Method not implemented.');
  }
}