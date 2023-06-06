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
 description: TypeScript 2.4 introduces tightens this up when relating two callback types.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

interface I<T> {
    add(t: T): T;
}
class C implements I<number> {
    add(n: number) {
        return n += 10;
    }
}

let a: I<number>;
let b: I<string | number>;
a = new C();

b = a;
Assert.equal(20, b.add(10));

function func<T>(arg: T): T{
    return arg;
}
let f1 = func<number>(5);
let f2 = func<number | string>('a');

f2 = f1;
Assert.equal(f1, f2);