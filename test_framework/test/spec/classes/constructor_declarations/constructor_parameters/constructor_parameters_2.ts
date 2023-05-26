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
    A parameter of a ConstructorImplementation may be prefixed with a public, private, or protected modifier. 
    This is called a parameter property declaration and is shorthand for declaring a property with the same name as the parameter 
    and initializing it with the value of the parameter.
 ---*/


class Point {
  constructor(public x: number, private y: number = 1) { }
  get foo(): number {
    return this.y;
  }
}
class Point2 {
  public x: number;
  protected y: number;
  constructor(x: number, y: number) {
    this.x = x;
    this.y = y;
  }
  get foo(): number {
    return this.y;
  }
}
let p1 = new Point(1, 2);
Assert.equal(p1.x, 1);
Assert.equal(p1.foo, 2);
let p2 = new Point2(3, 4);
Assert.equal(p2.foo, 4);
