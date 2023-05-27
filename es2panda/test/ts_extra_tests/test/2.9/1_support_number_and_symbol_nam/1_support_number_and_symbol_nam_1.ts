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

const str = "c";
const num = 10;
const sym = Symbol();
const bool = true;

const enum ABC { A, B, C }
const enum ABCStr { A = "A", B = "B", C = "C" }


type TypeObj = {
    a: string;
    5: string;

    [str]: string;
    [num]: string;
    [sym]: string;

    [ABC.A]: string;
    [ABCStr.A]: string;
}


type KEY1 = keyof TypeObj;
type KEY2 = Extract<keyof TypeObj, string>;
type KEY3 = Extract<keyof TypeObj, number>;
type KEY4 = Extract<keyof TypeObj, symbol>;


let key1: KEY1 = 'c';
let key2: KEY2 = ABCStr.A;
let key3: KEY3 = 10;
Assert.equal("c", key1);
Assert.equal(ABCStr.A, key2);
Assert.equal(10, key3);