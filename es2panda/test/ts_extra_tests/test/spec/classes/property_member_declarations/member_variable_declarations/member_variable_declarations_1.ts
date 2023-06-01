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
    Initializers in static member variable declarations are executed once when the containing script or module is loaded.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class Point {
  constructor(public x: number, public y: number) { }
  public addc() {
    Point.c++;
  }
  static origin = new Point(0, 0);
  static c: number = 10;
}
let a: Point = new Point(1, 1);
Assert.equal(Point.c, 10);
a.addc();
Assert.equal(Point.c, 11);
let b: Point = new Point(1, 1);
b.addc();
Assert.equal(Point.c, 12);