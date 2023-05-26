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
  The in operator now acts as a narrowing expression for types.
  For a n in x expression, where n is a string literal or string literal type and x is a union type, the “true” branch narrows to types which have an optional or required property n, and the “false” branch narrows to types which have an optional or missing property n.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

interface HWX {
  a: number;
}
interface HWY {
  b: string;
}

function fun(x: HWX | HWY) {
  if ("a" in x) {
    return x.a;
  }
  return x.b;
}

let a1: HWX = {
  a: 1
}

let b1: HWY = {
  b: 'this is a string'
}

Assert.equal(1, fun(a1));
Assert.equal('this is a string', fun(b1));