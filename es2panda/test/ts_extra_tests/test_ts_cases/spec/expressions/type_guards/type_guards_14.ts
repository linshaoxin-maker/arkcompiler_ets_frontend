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
  A type guard of the form expr1 || expr2,
  when true, narrows the type of x to T1 | T2, where T1 is the type of x narrowed by expr1 when true,
  and T2 is the type of x narrowed by expr1 when false and then by expr2 when true, or
  when false, narrows the type of x by expr1 when false and then by expr2 when false.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../suite/assert.js'

function h_func(h_x: string | number | undefined) {
    if (typeof h_x === "string" || typeof h_x === "number") {
        return h_x
    }
    else {
        return undefined
    }
}
var h_a = h_func(10)
Assert.isNumber(h_a)
var h_b = h_func('s')
Assert.isString(h_b)
var h_c = h_func(undefined)
Assert.isUndefined(h_c);