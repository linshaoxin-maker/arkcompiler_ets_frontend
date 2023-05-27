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

function h_zip<S, T, U>(x: S[], y: T[], group: (x: S) => (y: T) => U): U[] {
    var length = Math.max(x.length, y.length);
    var cons: U[] = [];
    for (var i = 0; i < length; i++) cons.push(group(x[i])(y[i]));
    return cons;
}

var h_names = ["Peter", "Paul", "Mary"];
var h_ages = [7, 9, 12];
var h_pairs = h_zip(h_names, h_ages, str => num => ({ name: str, age: num }));
Assert.equal(typeof h_pairs, 'object');