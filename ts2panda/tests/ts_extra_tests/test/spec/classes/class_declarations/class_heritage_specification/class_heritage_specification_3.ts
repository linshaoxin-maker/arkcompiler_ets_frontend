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
   The this-type (section 3.6.3) of the declared class 
   must be assignable (section 3.11.4) to the base type reference 
   and each of the type references listed in the implements clause.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class Point {
  constructor(public x: number = 0) { }
  public add(): this {
    this.x++;
    return this;
  }
}
var p: Point = new Point(10);
Assert.equal(11, p.add().x);
// the extends
class ChildPoint extends Point {
  constructor(public x: number = 0) {
    super(x);
  }
  public move(): this {
    this.x++;
    return this;
  }
}
var pChild: ChildPoint = new ChildPoint(10);
Assert.equal(11, pChild.move().x);
var basePoint: Point = pChild.move();
Assert.equal(13, basePoint.add().x);
// the implements
interface InterPoint {
  x: number;
}
class Point2 implements InterPoint {
  x = 1;
  setadd(): this {
    this.x++;
    return this;
  }
}
var ipoint = new Point2();
Assert.equal(2, ipoint.setadd().x);
var interpoint: InterPoint = ipoint.setadd();
Assert.equal(3, interpoint.x);
