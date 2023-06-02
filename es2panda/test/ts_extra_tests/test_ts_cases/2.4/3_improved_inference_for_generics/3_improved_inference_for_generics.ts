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
 description: TypeScript 2.4 introduces a few wonderful changes around the way generics are inferred.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

function func<T, U>(f: (x: T) => U): (a: T[]) => U[] {
    return a => a.map(f);
}

const len: (a: string[]) => number[] = func(s => s.length);
Assert.equal(1, len(["s", "ss", "sss"])[0]);
Assert.equal(2, len(["s", "ss", "sss"])[1]);
Assert.equal(3, len(["s", "ss", "sss"])[2]);


type A = <T, U>(x: T, y: U) => [T, U];
type B = <S>(x: S, y: S) => [S, S];

function f(a: A, b: B) {
    b = a;
    Assert.isTrue(b == a);
}

let a: A = function funA<T, U>(x: T, y: U): [T, U] {
    return [x, y];
}
let b: B = function funB<S>(x: S, y: S): [S, S] {
    return [x, x];
}
f(a, b);