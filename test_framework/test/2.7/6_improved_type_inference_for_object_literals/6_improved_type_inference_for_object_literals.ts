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
---*/


let obj = [{ a: 1, b: 2 }, { a: "abc" }, {}][0];

// Multiple object literal type inferences for the same type parameter are similarly collapsed into a single normalized union type
function f<T>(...items: T[]): T {
   return items[1];
};
let obj2 = f({ a: 1, b: 2 }, { a: "abc", b: "cde" }, {});

Assert.equal(1, obj.a);
Assert.equal(2, obj.b);
Assert.equal("abc", obj2.a);
Assert.equal("cde", obj2.b);