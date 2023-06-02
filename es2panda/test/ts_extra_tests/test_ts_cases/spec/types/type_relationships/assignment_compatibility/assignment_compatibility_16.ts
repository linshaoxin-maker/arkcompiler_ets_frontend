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
  S is assignable to a type T, and T is assignable from S, 
  if S has no excess properties with respect to T, 
  and S is an object type, an intersection type, an enum type, or the Number, Boolean, or String primitive type, T is an object type, and for each member M in T,
  and M is a numeric index signature of type U, 
  and U is the Any type or S has an apparent string or numeric index signature of a type that is assignable to U.
 module: ESNext
 isCurrent: true
 ---*/


import {Assert} from '../../../../../suite/assert.js'

interface U {
  anything: string
}
interface T {
  [key: number]: U
}
interface S {
  [keyname: number]: U
}
let t: T = {}
let s: S = {}
s[10] = { anything: "thing" }
t = s
Assert.equal(t, s);