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
  TypeScript’s uncalled function checks now apply within && and || expressions.
 module: ESNext
 isCurrent: true
 error: 
  code: TS2774
  message: This condition will always return true since this function is always defined. Did you mean to call it instead?
---*/

import { Assert } from "../../../suite/assert.js";

function defined(u: unknown) {
  if (u !== undefined) {
    return u;
  } else {
    return undefined;
  }
}

function test(f: boolean) {
  let fa: [boolean?, boolean?, boolean?] = [];
  if (defined) {
    fa[0] = true;
  } else {
    fa[0] = false;
  }
  if (defined && f) {
    fa[1] = true;
  } else {
    fa[1] = false;
  }
  if (defined || f) {
    fa[2] = true;
  } else {
    fa[2] = false;
  }
  return fa;
}

Assert.equal(JSON.stringify(test(true)), "[true,true,true]");
Assert.equal(JSON.stringify(test(false)), "[true,false,true]");
