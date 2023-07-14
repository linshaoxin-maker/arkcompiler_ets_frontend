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
   One neat feature here is that this analysis works transitively.
   TypeScript will hop through constants to understand what sorts of checks you’ve already performed.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

function isBool(a: unknown) {
    if (typeof a === "string") {
        Assert.equal(a[0], "f");
    }
    if (typeof a === "number") {
        Assert.equal(a, 12);
    }
    if (typeof a === "boolean") {
        Assert.isBoolean(a);
    }
    if (typeof a === "undefined") {
        Assert.isUndefined(a);
    }
    if (typeof a === "function") {
        Assert.isFunction(a);
    }
    if (typeof a === "object") {
        Assert.isObject(a);
    }
    if (typeof a === "symbol") {
        Assert.isSymbol(a);
    }
    const x = typeof a === "string";
    const y = typeof a === "number";
    const z = x || y;
    if (z) {
        return 0;
    } else {
        return 1;
    }
}
Assert.equal(isBool(false), 1);
Assert.equal(isBool("false"), 0);
Assert.equal(isBool(12), 0);
let x: undefined;
Assert.equal(isBool(x), 1);
let f = (a: number) => { a = 0; return a; };
Assert.equal(isBool(f), 1);
let obj: object = {
    x: 1
}
Assert.equal(isBool(obj), 1);
let sym: symbol = Symbol('a');
Assert.equal(isBool(sym), 1);