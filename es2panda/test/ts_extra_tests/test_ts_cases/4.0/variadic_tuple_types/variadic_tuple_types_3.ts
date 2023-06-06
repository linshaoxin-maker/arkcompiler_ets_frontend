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
  This is another case of what we like to call “death by a thousand overloads”, and it doesn’t even solve the problem generally.
  It only gives correct types for as many overloads as we care to write. 
  If we wanted to make a catch-all case, we’d need an overload like the following:
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

function funVTT01<T, U>(arr1: T[], arr2: U[]): Array<T | U> {
  return [...arr1, ...arr2]
}
const arr1: string[] = ["hello", "world"];
const arr2: number[] = [1, 2, 3];
const arrsn: Array<string | number> = funVTT01(arr1, arr2);
function funVTT02(arg: any) {
  const [_, ...result] = arg;
  return result;
}
const myTuple: any = [1, 2, 3, 4] as const;
const newArr: any = ["hello", "world"];
const r1 = funVTT02(myTuple);
const r2 = funVTT02([...myTuple, ...newArr] as const);
const Arrlength = r2.length;
Assert.equal(Arrlength, 5);
