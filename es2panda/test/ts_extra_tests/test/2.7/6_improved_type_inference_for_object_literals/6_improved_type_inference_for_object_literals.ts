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
 description: TypeScript 2.7 improves type inference for multiple object literals occurring in the same context. When multiple object literal types contribute to a union type, we now normalize the object literal types such that all properties are present in each constituent of the union type.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

let obj1 = [{ a: 1, b: 2 }, { a: "abc" }, {}][0];


function fun<T>(...items: T[]): T {
   return items[1];
};
let obj2 = fun({ a: 1, b: 2 }, { a: "abc", b: "ABC" }, {});

Assert.equal(1, obj1.a);
Assert.equal(2, obj1.b);
Assert.equal("abc", obj2.a);
Assert.equal("ABC", obj2.b);