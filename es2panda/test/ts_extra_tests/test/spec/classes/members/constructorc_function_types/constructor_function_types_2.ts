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
   If the class contains no constructor declaration and has a base class,
   a set of construct signatures with the same parameters as those of the base class constructor function type 
   following substitution of type parameters with the type arguments specified in the base class type reference, 
   all having the same type parameters as the class.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class A1 {
  public num: number;
  constructor(a: number) {
    this.num = a;
  }
}
class B1 extends A1 {
  public numb: number;
  constructor(a: number, b: number) {
    super(a);
    this.numb = b;
  }
}
const b = new B1(2, 2);
Assert.equal(b.num, 2);
