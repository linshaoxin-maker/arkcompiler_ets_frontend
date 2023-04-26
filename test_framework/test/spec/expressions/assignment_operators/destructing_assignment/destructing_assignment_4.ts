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
  In a destructuring assignment expression, the type of the expression on the right must be assignable to the assignment target on the left.
  An expression of type S is considered assignable to an assignment target V if V is an object assignment pattern and,
  for each assignment property P in V, P specifies a numeric property name and S has a numeric index signature of a type that is assignable to the target given in P.
 ---*/


interface Foo {
  [key: number]: string
  18: string
  180: string
}
let t: Foo = {
  18: 'age',
  180: 'height'
}
let v = {
  18: 'Age',
  180: 'Height'
}
v = t
Assert.equal(v[18], 'age');
Assert.equal(v[180], 'height');