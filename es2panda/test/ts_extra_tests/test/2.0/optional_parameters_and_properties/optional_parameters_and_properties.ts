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
 description: Optional parameters and properties automatically have undefined added to their types, even when their type annotations don’t specifically include undefined.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../../suite/assert.js";

function printPoint(a: number, b: number, c?: number): string {
    let obj: { a: number, b: number, c?: number };
    if (c !== undefined) {
        obj = { a, b, c };
        Assert.isNumber(c);
    } else {
        obj = { a, b };
        Assert.isUndefined(c);
    }
    return JSON.stringify(obj);
}
Assert.equal(printPoint(1, 3, 5), '{"a":1,"b":3,"c":5}');
Assert.equal(printPoint(1, 1), '{"a":1,"b":1}');
