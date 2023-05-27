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
     Common properties of unions are now considered discriminants as long as they contain some singleton type, 
     and they contain no generics.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../../suite/assert.js"

type R<T> = { error: Error; data: null } | { error: null; data: T };

function hwfun<T>(result: R<T>) {
    if (result.error) {
        throw result.error;
    }
    return result.data;
}

var a = {
    error: null,
    data: null,
};
var b = hwfun(a);
let f = false;
if (b == null) {
    f = true;
}
Assert.isTrue(f);

var c = {
    error: null,
    data: 10,
};
var d = hwfun(c);
Assert.equal(d, "10");
