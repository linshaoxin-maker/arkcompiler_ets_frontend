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
    The second change is that rest elements can occur anywhere in a tuple - not just at the end!
    TypeScript 4.0 improves the inference process for rest parameters and rest tuple elements so that we can type this and have it “just work”.
 ---*/


type Strings = [string, string];
type Numbers = number[];
type Unbounded = [...Strings, ...Numbers, boolean];
const myVariable: Unbounded = ["hello", "world", 1, 2, true];
Assert.equal(myVariable.length, 5);
type Arr = readonly any[];
function concat<T extends Arr, U extends Arr>(arr1: T, arr2: U): [...T, ...U] {
  return [...arr1, ...arr2];
}
const arr1 = [1, 2, 3];
const arr2 = ["a", "b", "c"];
const arr3 = concat(arr1, arr2);
Assert.equal(arr3.length, 6);

// add
type Arry = readonly unknown[];

function partialCall<T extends Arry, U extends Arry, R>(
  f: (...args: [...T, ...U]) => R,
  ...headArgs: T
) {
  return (...tailArgs: U) => f(...headArgs, ...tailArgs);
}
function foo(a: number, b: number, c: number): number {
  return a + b + c;
}
const add2And3 = partialCall(foo, 2, 3);
const result = add2And3(4);
Assert.equal(result, 9);
