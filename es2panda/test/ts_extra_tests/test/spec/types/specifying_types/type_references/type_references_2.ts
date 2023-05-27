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
   A type reference to a generic type is required to specify exactly one type argument for each type parameter 
   of the referenced generic type, and each type argument must be assignable to the constraint of the corresponding type parameter. 
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

interface h_A {
    h_a: string;
}
interface h_B extends h_A {
    h_b: string;
}
interface h_C extends h_B {
    h_c: string;
}
interface h_G<T, U extends h_B> {
    h_x: T;
    h_y: U;
}

var h_v: h_G<h_A, h_C> = {
    h_x: { h_a: 'h_a' },
    h_y: {
        h_a: 'h_a',
        h_b: 'h_b',
        h_c: 'h_c'
    }
}
Assert.equal(h_v.h_x.h_a, 'h_a');
Assert.equal(h_v.h_y.h_a, 'h_a');
Assert.equal(h_v.h_y.h_b, 'h_b');
Assert.equal(h_v.h_y.h_c, 'h_c');