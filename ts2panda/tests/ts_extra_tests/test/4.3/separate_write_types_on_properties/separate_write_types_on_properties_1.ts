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
    TypeScript 4.3 allows to specify types for reading and writing to properties.
    When considering how two properties with the same name relate to each other, TypeScript will only use the "reading" type. 
    "Writing" types are only considered when directly writing to a property.
 options: 
    target: es2015
    lib: es2015
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../../suite/assert.js"

class HWC {
    #size = 0;
    get size(): number {
        return this.#size;
    }
    set size(value: string | number | boolean) {
        let num = Number(value);
        if (!Number.isFinite(num)) {
            this.#size = 0;
            return;
        }
        this.#size = num;
    }
}

let c = new HWC();

c.size = "hello";
c.size = 42;
c.size = true;

let n: number = c.size;
Assert.equal(typeof n, "number");
