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
    Super calls (section 4.9.1) are used to call the constructor of the base class.
 ---*/


class Point {
  public x: number;
  public y: number;
  constructor(x: number, y: number) {
    this.x = x;
    this.y = y;
  }
}
class ColoredPoint extends Point {
  constructor(x: number, y: number, public color: string) {
    super(x, y);
  }
}
let p1 = new ColoredPoint(1, 2, "red");
Assert.equal(p1.x, 1);
Assert.equal(p1.y, 2);
Assert.equal(p1.color, "red");
