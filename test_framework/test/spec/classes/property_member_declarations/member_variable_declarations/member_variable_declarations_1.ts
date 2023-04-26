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
    Member declarations with a static modifier are called static member declarations.
    it is possible to have instance and static property members with the same name.
 ---*/


class Point {
  constructor(public x: number, public y: number) { }
  public distance(p: Point) {
    var dx = this.x - p.x;
    var dy = this.y - p.y;
    return Math.sqrt(dx * dx + dy * dy);
  }
  static origin = new Point(0, 0);
  static distance(p1: Point, p2: Point) {
    return p1.distance(p2);
  }
  static x: number = 10;
}
var p1: Point = new Point(2, 2);
var p2: Point = new Point(1, 1);
Assert.equal(p1.distance(p2), Math.sqrt(2));
Assert.equal(Point.distance(p1, p2), Math.sqrt(2));
Assert.equal(p1.distance(Point.origin), Math.sqrt(8));
Assert.equal(Point.x, 10);
