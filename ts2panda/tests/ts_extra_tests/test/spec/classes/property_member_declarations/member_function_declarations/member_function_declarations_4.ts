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
    A member function can access overridden base class members using a super property access (section 4.9.2).
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class Point {
  constructor(public x: number, public y: number) { }
  public toString() {
    return "x=" + this.x + " y=" + this.y;
  }
}
class ColoredPoint extends Point {
  constructor(x: number, y: number, public color: string) {
    super(x, y);
  }
  public toString() {
    return super.toString() + " color=" + this.color;
  }
}
let cp = new ColoredPoint(1, 2, "red");
Assert.equal(cp.color, "red");
Assert.equal(cp.x, 1);
Assert.equal(cp.y, 2);
