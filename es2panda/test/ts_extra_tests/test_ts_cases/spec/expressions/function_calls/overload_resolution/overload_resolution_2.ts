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
    The compile-time processing of a typed function call consists,
    a list of candidate signatures is constructed from the call signatures in the function type in declaration order.
    A generic signature is a candidate in a function call without type arguments when type inference succeeds for each type parameter,
    once the inferred type arguments are substituted for their associated type parameters, 
    the signature is applicable with respect to the argument list of the function call.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

interface IFnCall<S> {
  <T>(fn: (name: S) => T, age: number): T
}
interface IFnCall<S> {
  <T>(func: (str: S) => T, name: string): T
}
const foo: IFnCall<string> = function (fn:any) {
  return fn('xiao')
}
const res = foo((name) => {
  return name
}, 10)
Assert.isString(res);