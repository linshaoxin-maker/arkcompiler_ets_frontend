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
 description: TypeScript 2.9 adds support for number and symbol named properties in index types and mapped types.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

const str = "x";
const num = 1;
const sym = Symbol();
const enum NUM { ONE, TWO, THREE }
const enum XYZStr { X = "X", Y = "Y", Z = "Z" }
type TypeObj = {
    y: string;
    2: string;
    [str]: string;
    [num]: string;
    [sym]: string;
    [NUM.ONE]: string;
    [XYZStr.X]: string;
}
type X1 = keyof TypeObj;
type X2 = Extract<keyof TypeObj, string>;
type X3 = Extract<keyof TypeObj, number>;
let x1: X1 = "x";
let x2: X2 = XYZStr.X;
let x3: X3 = 1;
Assert.equal("x", x1);
Assert.equal(XYZStr.X, x2);
Assert.equal(1, x3);