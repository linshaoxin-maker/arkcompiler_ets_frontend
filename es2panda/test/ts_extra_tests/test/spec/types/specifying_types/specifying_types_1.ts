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
    Types are specified either by referencing their keyword or name, or by writing object type literals, 
    array type literals, tuple type literals, function type literals, constructor type literals, or type queries.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../suite/assert.js'

let h_num: number = 5;
Assert.isNumber(h_num);

let h_obj = {
  name: 'xiao',
  age: 18
}
Assert.equal(typeof h_obj, 'object');

let h_arr = [10, 5, 7, 20];
Assert.equal(typeof h_arr, 'object');

let h_tup = ['str', 5, true];
Assert.equal(typeof h_tup, 'object');

let h_func = (h_x: number, h_y: number) => {
  return h_x + h_y
}
Assert.equal(typeof h_func, 'function');