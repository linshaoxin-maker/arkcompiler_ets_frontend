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
  How would we type either of these in TypeScript?
  For concat, the only valid thing we could do in older versions of the language was to try and write some overloads.
  function concat<A, B, C, D, E, F>(arr1: [A, B, C, D, E, F], arr2: []): [A, B, C, D, E, F];
 ---*/

 
function concatA<T1, T2>(arr1: [T1, T2], arr2: []): [T1, T2];
function concatA<T>(arr1: T[], arr2: T[]): T[] {
  return arr1.concat(arr2);
}
const tuple: [] = [];
const combinedTuple: [number, string] = concatA([1, "a"], tuple);
Assert.equal(combinedTuple[0], 1);
Assert.equal(combinedTuple[1], "a");

const arr1: [number, string, boolean, number[], object, string] = [
  1,
  "hello",
  true,
  [2, 3],
  { name: "john" },
  "world",
];
const arr2: [] = [];
function concat<A, B, C, D, E, F>(
  arr1: [A, B, C, D, E, F],
  arr2: []
): [A, B, C, D, E, F] {
  return [...arr1, ...arr2];
}
const result = concat(arr1, arr2);
Assert.equal(result.length, 6);
