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
    conditional types can now immediately reference themselves within their branches
 ---*/


type ElementType<T> = T extends ReadonlyArray<infer U> ? ElementType<U> : T;

function deepFlatten<T extends readonly unknown[]>(x: T): ElementType<T>[] {
  return [];
}
let deepx = deepFlatten([1, 2, 3]);
type typeofx = typeof deepx;
let nx: typeofx = [1, 2, 3];
Assert.isNumber(nx[0]);
let deepx2 = deepFlatten([[1], [2, 3]]);
type typeofx2 = typeof deepx2;
let nx2: typeofx2 = [4, 5, 6];
Assert.isNumber(nx2[0]);
let deepx3 = deepFlatten([[1], [[2]], [[[3]]]]);
type typeofx3 = typeof deepx3;
let nx3: typeofx3 = [7, 8, 9];
Assert.isNumber(nx3[0]);