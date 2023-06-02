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

const enum Enum {
  A,
  B,
  C,
}

const ets = {
  [Enum.A]: "Name A",
  [Enum.B]: "Name B",
  [Enum.C]: "Name C",
};

const sym1 = Symbol();
const sym2 = Symbol();
const sym3 = Symbol();

const etn = {
  [sym1]: 1,
  [sym2]: 2,
  [sym3]: 3,
};

type KE = keyof typeof ets;
type KS = keyof typeof etn;

function getValue<T, K extends keyof T>(obj: T, key: K): T[K] {
  return obj[key];
}

let x1 = getValue(ets, Enum.C);
let x2 = getValue(etn, sym3);
Assert.equal("Name C", x1);
Assert.equal(3, x2);



function useKey1<T, K extends Extract<keyof T, string>>(o: T, k: K) {
  var name: string = k;
  return name;
}

const stringMap = {
  a: "mystring"
};
let x3 = useKey1(stringMap, "a");
Assert.equal("a", x3);


function useKey2<T, K extends keyof T>(o: T, k: K) {
  var name: string | number | symbol = k;
  return name;
}
const uniteMap = {
  a: "mystring",
  b: 1
};
let x4 = useKey2(uniteMap, "b");
Assert.equal("b", x4);