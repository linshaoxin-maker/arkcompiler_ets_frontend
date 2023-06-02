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
    Private property members can be accessed only within their declaring class. Specifically,
    a private member M declared in a class C can be accessed only within the class body of C.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class Base {
  private x: number = 1;
  private y: number = 2;
  addx() {
    this.x++;
  }
  get foo() {
    return this.x;
  }
  public addxy(): number {
    return this.x + this.y;
  }
  static f(a: Base, b: Derived) {
    a.x = 1;
    b.x = 1;
    a.y = 1;
    b.y = 1;
  }
}
class Derived extends Base { }
let a: Base = new Base();
a.addx();
Assert.equal(a.foo, 2);
Assert.equal(a.addxy(), 4);