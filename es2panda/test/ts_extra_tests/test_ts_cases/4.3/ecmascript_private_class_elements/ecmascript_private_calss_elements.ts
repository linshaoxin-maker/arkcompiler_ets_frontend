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
   TypeScript 4.3 expands which elements in a class can be given #private #names to make them truly private at run-time.
   In addition to properties, methods and accessors can also be given private names.
   Even more broadly, static members can now also have private names.
 options:
   target: es2015
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../../suite/assert.js"

class HWC {
    #someMethod() {
        return 1;
    }
    test1() {
        return this.#someMethod();
    }
    static #something(num: number) {
        return num;
    }
    test2() {
        return HWC.#something(10);
    }
    get #someValue() {
        return 100;
    }
    publicMethod() {
        this.#someMethod();
        return this.#someValue;
    }
}

let c = new HWC();
Assert.equal(c.test1(), 1);
Assert.equal(c.test2(), 10);
Assert.equal(c.publicMethod(), 100);
