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
    Intersection types represent values that simultaneously have multiple types.
    A value of an intersection type A & B is a value that is both of type A and type B. 
    Intersection types are written using intersection type literals.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../suite/assert.js'

interface A1 {
  num1: number;
}
interface B1 {
  str1: string;
}
let ns1: A1 & B1 = { num1: 1, str1: "b" };
Assert.equal(ns1.num1, 1);
Assert.equal(ns1.str1, "b");
enum Color {
  Red1 = 1,
  Green1,
  Blue1,
}
interface A2 {
  num2: [string, number];
}
interface B2 {
  str2: Color;
}
let ns2: A2 & B2 = { num2: ["a2", 1], str2: Color.Red1 };
Assert.equal(ns2.num2[0], "a2");
Assert.equal(ns2.str2, 1);
interface A3 {
  num3: number[];
}
interface B3 {
  str3: boolean;
}
let ns3: A3 & B3 = { num3: [1, 2, 3], str3: true };
Assert.equal(ns3.str3, true);
interface A4 {
  num4: number;
}
interface B4 {
  str4: string;
}
interface C4 {
  cm: any;
}
let ns4: A4 & B4 & C4 = { num4: 1, str4: "b4", cm: 3 };
Assert.equal(ns4.num4, 1);
Assert.equal(ns4.str4, "b4");
Assert.equal(ns4.cm, 3);
interface XX {
  obj: A1;
}
interface YY {
  obj: B1;
}
let xxyy: XX & YY = { obj: ns1 };
Assert.equal(xxyy.obj.num1, 1);