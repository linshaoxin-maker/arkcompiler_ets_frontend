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
   Higher order type inference from generic functions.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../suite/assert.js'

function hwfun01<A, B, C>(f: (arg: A) => B, g: (arg: B) => C): (arg: A) => C {
    return (x) => g(f(x));
}

interface A<T> {
    value: T;
}

function hwfun02<T>(x: T): T[] {
    return [x];
}

function hwfun03<U>(value: U): A<U> {
    return { value };
}

// has type '(arg: {}) => Box<{}[]>'
const hw00 = hwfun01(
    hwfun02,
    hwfun03,
)


Assert.equal(hw00("hello").value[0].toUpperCase(), 'HELLO');