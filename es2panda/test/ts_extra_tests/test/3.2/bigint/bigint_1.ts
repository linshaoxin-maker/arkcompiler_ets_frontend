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
    BigInts are part of an upcoming proposal in ECMAScript that allow us to model theoretically arbitrarily large integers. 
    Brings type-checking for BigInts, as well as support for emitting BigInt literals when targeting esnext.
    BigInt support in TypeScript introduces a new primitive type called the 'bigint' (all lowercase). 
    You can get a bigint by calling the BigInt() function or by writing out a BigInt literal by adding an n to the end of any integer numeric litera.
 options: 
    target: es2020
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

let bi01: bigint = BigInt(100);
Assert.equal(bi01, 100n)

let bi02: bigint = 100n;
Assert.equal(bi02, 100n)

function hwfun(n: bigint) {
    let a = 1n;
    for (let x = 0n, i = 0n; i < n; i++) {
        const y = a;
        a += x;
        x = y;
    }
    return a;
}

let a = hwfun(100n);
Assert.equal(a, 573147844013817084101n);