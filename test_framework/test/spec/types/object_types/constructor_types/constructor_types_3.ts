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
  An object type containing one or more construct signatures is said to be a constructor type. 
 ---*/


interface Point {
  x: number;
  y: number;
}
interface PointConstructor {
  new(x: number, y: number): Point;
}

class Point2D implements Point {
  readonly x: number;
  readonly y: number;
  constructor(x: number, y: number) {
    this.x = x;
    this.y = y;
  }
}

function newPoint(
  pointConstructor: PointConstructor,
  x: number,
  y: number
): Point {
  return new pointConstructor(x, y);
}

const point1: Point = new Point2D(1, 2);
Assert.equal(point1.x, 1);
Assert.equal(point1.y, 2);

const point: Point = newPoint(Point2D, 2, 2);
Assert.equal(point.x, 2);
Assert.equal(point.y, 2);
