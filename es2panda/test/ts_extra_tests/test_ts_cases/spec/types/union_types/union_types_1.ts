/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
/**---
 description: >
    union types represent values that may have one of several distinct representations. 
    a value of a union type A | B is a value that is either of type A or type B. Union types are written using union type literals
 module: ESNext
 isCurrent: true
 ---*/


import {Assert} from '../../../../suite/assert.js'

var n_s: number | string;
var b_s: boolean | string;
var o_na: object | number[];
n_s = 2048;
Assert.isNumber(n_s);
b_s = false;
Assert.isFalse(b_s);
n_s = "NARC";
Assert.isString(n_s);
b_s = "1";
Assert.equal(b_s, "1");


