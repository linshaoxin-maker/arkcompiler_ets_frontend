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
 ---*/


interface A {
  a: number;
}
interface B {
  b: string;
}
var ab: A & B = { a: 1, b: "b" };
Assert.equal(ab.a, 1);
Assert.equal(ab.b, "b");
enum Color {
  Red1 = 1,
  Green1,
  Blue1,
}
interface A2 {
  a2: [string, number];
}
interface B2 {
  b2: Color;
}
var ab2: A2 & B2 = { a2: ["a2", 1], b2: Color.Red1 };
Assert.equal(ab2.a2[0], "a2");
Assert.equal(ab2.b2, 1);
interface A3 {
  a3: number[];
}
interface B3 {
  b3: boolean;
}
var ab3: A3 & B3 = { a3: [1, 2, 3], b3: true };
Assert.equal(ab3.b3, true);
interface A4 {
  a4: number;
}
interface B4 {
  b4: string;
}
interface C4 {
  c: any;
}
var ab4: A4 & B4 & C4 = { a4: 1, b4: "b4", c: 3 };
Assert.equal(ab4.a4, 1);
Assert.equal(ab4.b4, "b4");
Assert.equal(ab4.c, 3);
interface X {
  p: A;
}
interface Y {
  p: B;
}
var xy: X & Y = { p: ab };
Assert.equal(xy.p.a, 1);
type F1 = (a: string, b: string) => void;
type F2 = (a: number, b: number) => void;
var f: F1 & F2 = (a: string | number, b: string | number) => { };
f("hello", "world");
f(1, 2);
type F3 = (a: any, b: boolean) => void;
var f2: F1 & F2 & F3 = (
  a: string | number | any,
  b: string | number | boolean
) => { };