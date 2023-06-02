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
     a union type encompasses an ordered set of constituent types. 
     while it is generally true that A | B is equivalent to B | A, the order of the constituent types may matter when determining the call and construct signatures of the union type.
 module: ESNext
 isCurrent: true
 ---*/


import {Assert} from '../../../../suite/assert.js'

var ab: number | string;
var ba: string | number;
ab = 1408;
ba = 1408;
Assert.equal(ab, ba);
Assert.equal(typeof ab, typeof ba);
ab = "Shift";
ba = "Shift";
Assert.equal(ab, ba);
Assert.equal(typeof ab, typeof ba);
