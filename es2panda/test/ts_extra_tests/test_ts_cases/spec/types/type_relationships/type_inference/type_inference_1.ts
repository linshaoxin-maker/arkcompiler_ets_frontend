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
  In certain contexts, 
  inferences for a given set of type parameters are made from a type S, in which those type parameters do not occur, 
  to another type T, in which those type parameters do occur.
 module: ESNext
 isCurrent: true
 ---*/


import {Assert} from '../../../../../suite/assert.js'

interface Cb {
    (a: number): void
}
interface Fn {
    (cb: Cb): void
}
const fn: Fn = function (cb) { }
// The argument a to the anonymous function is of type number
fn(function (a) {
    Assert.isNumber(a);
    a = a + 1;
});