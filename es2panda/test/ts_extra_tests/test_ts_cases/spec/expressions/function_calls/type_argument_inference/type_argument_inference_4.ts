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
  If e is an expression of a function type that contains exactly one generic call signature and no other members, 
  and T is a function type with exactly one non-generic call signature and no other members, 
  then any inferences made for type parameters referenced by the parameters of T's call signature are fixed, 
  and e's type is changed to a function type with e's call signature instantiated in the context of T's call signature.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

function fun<S, T, U>(a: S[], b: T[], group: (a: S) => (b: T) => U): U[] {
    let length = Math.max(a.length, b.length);
    let cons: U[] = [];
    for (let i = 0; i < length; i++) cons.push(group(a[i])(b[i]));
    return cons;
}
let xx = ["sum", "con", "say"];
let yy = [1, 2, 3];
let zz = fun(xx, yy, str => num => ({ name: str, age: num }));
Assert.isObject(zz);