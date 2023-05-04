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
    function foo(...args: [string, number]): void {
      // ...}
 ---*/

 
function fooA(...args: [string, number]): void {
  const newArr = [args[0].toUpperCase(), args[1] * 2];
  Assert.equal(newArr[0], "HELLO");
  Assert.equal(newArr[1], 10);
}
fooA("hello", 5);
function fooB(arg0: string, arg1: number): void {
  const Arrb = [arg0.toUpperCase(), arg1 * 2];
  Assert.equal(Arrb[0], "HELLO");
  Assert.equal(Arrb[1], 10);
}
fooB("hello", 5);
