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
   In TypeScript 3.4, we improved inference for when generic functions that return functions.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../suite/assert.js'

{
    class A<T> {
        value: T;

        constructor(value: T) {
            this.value = value;
        }
    }

    class B<U> {
        value: U;

        constructor(value: U) {
            this.value = value;
        }
    }

    function hwfun<T, U, V>(
        F: new (x: T) => U,
        G: new (y: U) => V
    ): (x: T) => V {
        return x => new G(new F(x));
    }


    let f = hwfun(A, B);

    let a = f(1024);

    Assert.equal(a.value.value, 1024);
};