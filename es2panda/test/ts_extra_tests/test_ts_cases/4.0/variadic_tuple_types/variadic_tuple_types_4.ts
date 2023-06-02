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
  Uh…okay, that’s…seven overloads for when the second array is always empty. Let’s add some for when arr2 has one argument.
  function concat<A1, B1, C1, D1, E1, F1, A2>(arr1: [A1, B1, C1, D1, E1, F1], arr2: [A2]): [A1, B1, C1, D1, E1, F1, A2];
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../../suite/assert.js"

function hwtest01<A1, B1, C1, D1, E1, F1, A2, A3>(
  arr1: [A1, B1, C1, D1, E1, F1, A2],
  arr2: [A3]
): [A1, B1, C1, D1, E1, F1, A2, A3] {
  return [...arr1, ...arr2];
}
const arr1: [string, number, boolean, number, null, undefined, string] = [
  "hello",
  12,
  true,
  45,
  null,
  undefined,
  "tom",
];
const arr2: [string] = ["world"];
const result = hwtest01(arr1, arr2);
Assert.equal(result.length, 8);
