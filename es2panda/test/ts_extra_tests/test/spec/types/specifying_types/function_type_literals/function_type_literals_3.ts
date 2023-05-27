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
  Function types with multiple call or construct signatures cannot be written as function type literals 
  but must instead be written as object type literals.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

interface h_inter {
    (h_x: string): number;
    (h_x: number): string
}

var h_i: h_inter = Object.assign(function (h_x: any) {
    if (typeof h_x === 'string') {
        return h_x.toString();
    } else {
        return h_x.toString();
    }
})
Assert.equal(h_i('a'), 'a')
Assert.equal(h_i(2), 2);