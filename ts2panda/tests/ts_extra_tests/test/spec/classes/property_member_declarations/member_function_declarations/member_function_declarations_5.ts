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
    In a static member function,a call to 'new this()' may actually invoke a derived class constructor
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class A {
  constructor(public a: number = 1) { }
  static create() {
    return new this();
  }
}
class B extends A {
  constructor(public b: number = 2) {
    super();
  }
}
// new A()
var x = A.create();
// new B()
var y = B.create();
Assert.equal(x.a, 1);
Assert.equal(y.a, 1);
