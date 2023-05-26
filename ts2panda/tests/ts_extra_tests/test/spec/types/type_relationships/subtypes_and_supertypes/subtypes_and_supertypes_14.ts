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
  S is a subtype of a type T, and T is a supertype of S,
  if S has no excess properties with respect to T, 
  and S is an object type, an intersection type, an enum type, or the Number, Boolean, or String primitive type, T is an object type, 
  and for each member M in T, M is a string index signature of type U, 
  and U is the Any type or S has an apparent string index signature of a type that is a subtype of U.
 module: ESNext
 isCurrent: true
 ---*/


import {Assert} from '../../../../../suite/assert.js'

interface T {
  name: string
}
interface S {
  name: string
  age: number
}
interface U {
  [name: string]: any
}
let obj: U = { name: "xiao", age: 18 }
Assert.equal(JSON.stringify(obj), '{"name":"xiao","age":18}');