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
    A member variable declaration declares an instance member variable or a static member variable.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class Point {
  public x: number;
  public y: number;
  constructor(x: number, y: number) {
    this.x = x;
    this.y = y;
  }
  static z: number = 3;
}
class ColoredPoint extends Point {
  constructor(x: number, y: number, public color: string) {
    super(x, y);
  }
  static r: number = 10;
}
let p = new Point(1, 2);
Assert.equal(p.x, 1);
Assert.equal(p.y, 2);
Assert.equal(Point.z, 3);
let cp = new ColoredPoint(4, 5, "red");
Assert.equal(cp.x, 4);
Assert.equal(cp.y, 5);
Assert.equal(ColoredPoint.r, 10);
