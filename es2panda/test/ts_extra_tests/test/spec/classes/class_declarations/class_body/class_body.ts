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
    The class body consists of zero or more constructor or member declarations. 
    Statements are not allowed in the body of a class—they must be placed in the constructor or in members.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class P {
  num1: number;
  num2: number;
  constructor(num1: number, num2: number) {
    this.num1 = num1;
    this.num2 = num2;
  }
  public hypot() {
    return Math.sqrt(this.num1 * this.num1 + this.num2 * this.num2);
  }
  static initial = new P(0, 0);
}
var p: P = new P(10, 20);
Assert.equal(10, p.num1);
Assert.equal(20, p.num2);

// zero constructor
class Circle {
  radius: number = 1;
}
const c = new Circle();
Assert.equal(c.radius, 1);

// more constructor
type TypeSummation = {
  width?: number;
  height?: number;
};
class summation {
  public width: number;
  public height: number;
  constructor(width: number, height: number);
  constructor(ParamObje_: TypeSummation);
  constructor(ParamObje_Obj_: any, height_ = 0) {
    if (typeof ParamObje_Obj_ === "object") {
      const { width, height } = ParamObje_Obj_;
      this.width = width;
      this.height = height;
    } else {
      this.width = ParamObje_Obj_;
      this.height = height_;
    }
  }
  sunArea(): number {
    return this.width * this.height;
  }
}
const sun = new summation(4, 5);
Assert.equal(sun.sunArea(), 20);
const obj: TypeSummation = { width: 10, height: 2 };
const sun2 = new summation(obj);
Assert.equal(sun2.sunArea(), 20);
