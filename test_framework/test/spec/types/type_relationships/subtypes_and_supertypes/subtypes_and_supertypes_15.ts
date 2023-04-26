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
  and for each member M in T, M is a numeric index signature of type U, 
  and U is the Any type or S has an apparent string or numeric index signature of a type that is a subtype of U.
 ---*/


interface T {
    age: number
}
interface S {
    age: number
    height: number
}
interface U {
    [age: number]: any
}
let obj: U = {}
obj[5] = { age: 18, height: 180 }
Assert.equal(JSON.stringify(obj[5]), '{"age":18,"height":180}')