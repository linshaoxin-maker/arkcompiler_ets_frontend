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
 description: the difference between private and #
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../suite/assert.js'

class C1 {
    private x = 10;
}

Assert.equal(10, new C1()["x"]);

class C2 {
    #x: number;
    constructor(x: number) {
        this.#x = x;
    }
    getX(): number {
        return this.#x;
    }
}

let c = new C2(5);
Assert.equal(c.getX(), 5);
