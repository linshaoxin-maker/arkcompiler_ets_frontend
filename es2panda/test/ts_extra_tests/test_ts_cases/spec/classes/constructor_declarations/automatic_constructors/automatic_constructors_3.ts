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
    In a class with no extends clause, the automatic constructor has no parameters 
    and performs no action other than executing the instance member variable initializers (section 8.4.1), if any.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class Foo {
  constructor(public x: number = 1, public y: string = "go") {
    this.x = x;
    this.y = y;
  }
}
let foo = new Foo();
Assert.equal(foo.x, 1);
Assert.equal(foo.y, "go");
class Foo2 {
  x: number;
  y: string;
  constructor(x: number, y: string) {
    this.x = x;
    this.y = y;
  }
}
let foo2 = new Foo2(2, "go");
Assert.equal(foo2.x, 2);
Assert.equal(foo2.y, "go");
