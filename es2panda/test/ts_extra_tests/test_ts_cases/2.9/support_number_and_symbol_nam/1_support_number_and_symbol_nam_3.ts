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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implie d.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**---
 description: with the keyof operator's support for number and symbol named keys, it is now possible to abstract over access to properties of objects that are indexed by numeric literals (such as numeric enum types) and unique symbols.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

const enum E1 {
  ONE,
  TWO,
  THREE,
}
const ets = {
  [E1.ONE]: "ONE",
  [E1.TWO]: "TWO",
  [E1.THREE]: "THREE",
};
const x = Symbol();
const y = Symbol();
const z = Symbol();
const etn = {
  [x]: 1,
  [y]: 2,
  [z]: 3,
};
function getValue<T, K extends keyof T>(obj: T, key: K): T[K] {
  return obj[key];
}
let x1 = getValue(ets, E1.THREE);
let x2 = getValue(etn, z);
Assert.equal("THREE", x1);
Assert.equal(3, x2);
function fun1<T, K extends Extract<keyof T, string>>(o: T, k: K) {
  let m: string = k;
  return m;
}
const stringMap = {
  a: "mystring"
};
let x3 = fun1(stringMap, "a");
Assert.equal("a", x3);
function fun2<T, K extends keyof T>(o: T, k: K) {
  let m: string | number | symbol = k;
  return m;
}
const uniteMap = {
  a: "mystring",
  b: 1
};
let x4 = fun2(uniteMap, "b");
Assert.equal("b", x4);