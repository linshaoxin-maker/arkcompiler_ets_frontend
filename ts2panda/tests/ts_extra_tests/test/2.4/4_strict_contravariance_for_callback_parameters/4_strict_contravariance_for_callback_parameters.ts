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

interface TestI<T> {
    add_10(t: T): T;
}

class CA implements TestI<number> {
    add_10(n: number) {
        return n += 10;
    }
}

let a: TestI<number>;
let b: TestI<string | number>;

a = new CA();

b = a;

Assert.equal(20, b.add_10(10));