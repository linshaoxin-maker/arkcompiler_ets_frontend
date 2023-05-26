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
   the type checker analyses all possible flows of control in statements and expressions to produce the most specific type possible (the narrowed type) at any given location for a local variable or parameter that is declared to have a union type.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from "../../../suite/assert.js"

function fun01(x: string | number | boolean): void {
    if (typeof x === "string") {
        x;
        Assert.equal(typeof x, "string");
        x = 1;
        x;
    }
    x;
}

function fun02(x: string | number): void {
    if (typeof x === "number") {
        Assert.equal(typeof x, "number");
        return;
    }
    x;
}

fun01("hello");
fun02(1024);
