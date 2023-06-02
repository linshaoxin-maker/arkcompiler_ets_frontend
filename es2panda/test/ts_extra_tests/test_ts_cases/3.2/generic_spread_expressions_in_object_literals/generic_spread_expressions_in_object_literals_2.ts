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
    Property assignments and non-generic spread expressions are merged to the greatest extent possible on either side of a generic spread expression.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../../suite/assert.js"

function func<T>(arg1: T, arg2: { str: string }, arg3:{num: number}) {
    let obj = { n: 5, ...arg2, ...arg1, s: 's', ...arg3 };
    return obj;
}

let o1 = {
    str: 'a'
};
let o2 = {
    num: 10
};
var f = func({ s: "string" }, o1, o2);

Assert.isObject(f);
