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
  When a function expression is inferentially typed and a type assigned to a parameter in that expression references type parameters for which inferences are being made,
  the corresponding inferred type arguments to become fixed and no further candidate inferences are made for them.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

function h_map<T, U>(h_a: T[], h_f: (h_x: T) => U): U[] {
    var cons: U[] = [];
    for (var i = 0; i < h_a.length; i++) cons.push(h_f(h_a[i]));
    return cons;
}

var h_names = ["xiao", "xi", "xin"];
var h_lengths = h_map(h_names, str => str.length);
Assert.equal(typeof h_lengths, 'object');