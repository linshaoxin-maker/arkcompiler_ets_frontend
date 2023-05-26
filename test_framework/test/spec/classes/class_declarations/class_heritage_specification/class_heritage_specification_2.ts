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
   The heritage specification of a class consists of optional extends and implements clauses. 
   The extends clause specifies the base class of the class 
   and the implements clause specifies a set of interfaces for which to validate the class provides an implementation.
 ---*/


class Point {
  constructor(public x: number, public y: number) { }
  public length() {
    return Math.sqrt(this.x * this.x + this.y * this.y);
  }
  static origin = new Point(0, 0);
}

var p: Point = new Point(10, 20);
Assert.equal(10, p.x);
Assert.equal(20, p.y);
// the extends
class ChildPoint extends Point {
  constructor(public x: number, public y: number) {
    super(x, y);
  }
  public move() {
    this.x += 1;
    this.y += 1;
  }
}
var pChild: ChildPoint = new ChildPoint(10, 20);
pChild.move();
Assert.equal(11, pChild.x);
Assert.equal(21, pChild.y);
// the implements
interface InterPoint {
  x: number;
  y: number;
}
class Point2 implements InterPoint {
  x = 1;
  y = 1;
  setarea(x: number, y: number) {
    return x * y;
  }
}
var ipoint = new Point2();
Assert.equal(1, ipoint.x);
Assert.equal(1, ipoint.y);
