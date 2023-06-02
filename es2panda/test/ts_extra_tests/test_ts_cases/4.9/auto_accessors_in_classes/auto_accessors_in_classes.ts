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
    typescript 4.9 makes the in operator a little bit more powerful when narrowing types that don’t list the property at all.
 options:
    target: es2015
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../../suite/assert.js"

class NormalClass {
    #_str: string;
    get str(): string {
        return this.#_str;
    }
    set str(value: string) {
        this.#_str = value;
    }
    constructor(str: string) {
        this.#_str = str;
    }
}
class AutoClass {
    str: string;
    constructor(str: string) {
        this.str = str;
    }
}

let c1 = new NormalClass("0");
let c2 = new AutoClass("0");
c1.str = "NormalClass";
Assert.equal(c1.str, "NormalClass");
c2.str = "AutoClass";
Assert.equal(c2.str, "AutoClass");
