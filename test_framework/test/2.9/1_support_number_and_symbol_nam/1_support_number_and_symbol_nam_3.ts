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
description: with the keyof operator's support for number and symbol named keys, it is now possible to abstract over access to properties of objects that are indexed by numeric literals (such as numeric enum types) and unique symbols.
---*/


const enum Enum {
  A,
  B,
  C,
}

const enumToStringMap = {
  [Enum.A]: "Name A",
  [Enum.B]: "Name B",
  [Enum.C]: "Name C",
};

const sym1 = Symbol();
const sym2 = Symbol();
const sym3 = Symbol();

const symbolToNumberMap = {
  [sym1]: 1,
  [sym2]: 2,
  [sym3]: 3,
};

type KE = keyof typeof enumToStringMap;
type KS = keyof typeof symbolToNumberMap;

function getValue<T, K extends keyof T>(obj: T, key: K): T[K] {
  return obj[key];
}

let x1 = getValue(enumToStringMap, Enum.C);
let x2 = getValue(symbolToNumberMap, sym3);
Assert.equal("Name C", x1);
Assert.equal(3, x2);


// If your functions are only able to handle string named property keys, use Extract<keyof T, string> in the declaration
function useKey1<T, K extends Extract<keyof T, string>>(o: T, k: K) {
  var name: string = k;
  return name;
}

const stringMap = {
  a: "mystring"
};
let x3 = useKey1(stringMap, "a");
Assert.equal("a", x3);

// your functions are open to handling all property keys, then the changes should be done down-stream
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