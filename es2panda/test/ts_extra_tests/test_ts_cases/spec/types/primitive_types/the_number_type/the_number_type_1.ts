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
    Test "Number primitive type corresponds to the similarly named JavaScript primitive type
    and represents double-precision 64-bit format IEEE 754 floating point values."
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

Assert.notEqual(0.1 + 0.2, 0.3);
Assert.notEqual(0.7 + 0.1, 0.8);
Assert.notEqual(0.2 + 0.4, 0.6);
Assert.notEqual(1.5 - 1.2, 0.3);
Assert.notEqual(0.3 - 0.2, 0.1);
Assert.notEqual(19.9 * 100, 1990);
Assert.notEqual(0.8 * 3, 2.4);
Assert.notEqual(35.41 * 100, 3541);
Assert.notEqual(0.3 / 0.1, 3);
Assert.notEqual(0.69 / 10, 0.069);
Assert.equal((1.335).toFixed(2), 1.33);
Assert.equal((1.3335).toFixed(3), 1.333);
Assert.equal((1.33335).toFixed(4), 1.3334);
Assert.equal((1.333335).toFixed(5), 1.33333);
Assert.equal((1.3333335).toFixed(6), 1.333333);