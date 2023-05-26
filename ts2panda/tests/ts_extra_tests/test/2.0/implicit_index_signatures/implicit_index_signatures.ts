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
   An object literal type is now assignable to a type with an index signature if all known properties in the
   object literal are assignable to that index signature. This makes it possible to pass a variable that was initialized with an object literal as parameter to a function that expects a map or dictionary:
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../../suite/assert.js"

function httpService(path: string, headers: { [x: string]: string }) {
  return JSON.stringify(headers);
}
const headers = {
  "Content-Type": "application/x-www-form-urlencoded",
};
let s1: string = httpService("", {
  "Content-Type": "application/x-www-form-urlencoded",
});
Assert.equal(s1, '{"Content-Type":"application/x-www-form-urlencoded"}');

// Now ok, previously wasn't
let s2: string = httpService("", headers);
Assert.equal(s2, '{"Content-Type":"application/x-www-form-urlencoded"}');
