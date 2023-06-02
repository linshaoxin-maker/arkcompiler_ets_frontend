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
    list of candidate signatures is constructed from the call signatures in the function type in declaration order.
    A non-generic signature is a candidate when
    the function call has no type arguments, and
    the signature is applicable with respect to the argument list of the function call.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

interface IfnCall {
  (name: string, age: number): string
}
interface IfnCall {
  (str: string, num: string): string
}
const foo: IfnCall = function (name, age) {
  return name + ":" + age
}
var f = foo("xiao", 18)
Assert.isString(f);