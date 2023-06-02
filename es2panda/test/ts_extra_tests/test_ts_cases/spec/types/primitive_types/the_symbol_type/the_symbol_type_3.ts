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
    The Symbol primitive type behaves as an object type with the same properties as the global interface type 'Symbol'.
 options: 
    lib: es2015
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

let h_x: symbol = Symbol();
let h_obj: any = {};
h_obj[h_x] = "primitive type";
Assert.equal(h_obj[h_x], "primitive type");
h_obj[Symbol.toStringTag] = "project";
Assert.equal(h_obj[Symbol.toStringTag], "project");