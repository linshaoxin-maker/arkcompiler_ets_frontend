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
  The inclusion of type arguments in the Arguments production gives rise to certain ambiguities in the grammar for expressions. 
 ---*/


// a call to 'f' with two arguments, 'g < A' and 'B > (7)'
var g: number = 5
var A: number = 3
var B: number = 6
function f(a: any, b?: any) {
  return a
}

Assert.isFalse(f(g < A, B > 7))
Assert.isFalse(f(g < A, B > +(7)))

// a call to a generic function 'g' with two type arguments and one regular argument
type A1 = number
type B1 = number
function g1<T, U>(a: T) {
  return a;
}
function f1(a: any, b?: any) {
  return a
}

Assert.equal(f1(g1<A1, B1>(7)), 7)