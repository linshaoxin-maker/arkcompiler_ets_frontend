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
  Consider a function in JavaScript called concat that takes two array or tuple types and concatenates them together to make a new array.
 ---*/

 
function concat(arr1: any, arr2: any) {
  return [...arr1, ...arr2];
}
const arrA: any = [1, 2, 3];
const arrB: any = [4, 5, 6];
let judgelength = concat(arrA, arrB).length;
Assert.equal(judgelength, 6);

function tail(arg: any) {
  const [_, ...result] = arg;
  return result;
}

const myArray: any = [1, 2, 3, 4, 5];
let taillength = tail(myArray).length;
Assert.equal(taillength, 4);
