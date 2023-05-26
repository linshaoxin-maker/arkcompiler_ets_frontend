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
    Protected property members can be accessed only within their declaring class and classes derived from their declaring class, 
    and a protected instance property member must be accessed through an instance of the enclosing class or a subclass thereof.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class Base {
  protected x: number = 1;
  add() {
    this.x++;
  }
  get foo() {
    return this.x;
  }
}
class Derived extends Base {
  get foo() {
    return this.x;
  }
}
const a: Base = new Base();
a.add();
Assert.equal(a.foo, 2);
const b: Derived = new Derived();
b.add();
Assert.equal(b.foo, 2);
