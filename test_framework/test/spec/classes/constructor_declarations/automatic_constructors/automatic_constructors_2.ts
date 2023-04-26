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
    In a derived class, the automatic constructor has the same parameter list (and possibly overloads) as the base class constructor.
 ---*/


class Point1 {
  x: string;
  y: string;
  constructor(x: string, y: string) {
    this.x = x;
    this.y = y;
  }
  toString() {
    return this.x + " " + this.y;
  }
}
class ColorPoint extends Point1 {
  color: string;
  constructor(x: string, y: string, color: string) {
    super(x, y);
    this.color = color;
  }
  toString() {
    return this.color + " " + super.toString();
  }
}
let co = new ColorPoint("A", "B", "blue");
Assert.equal(co.x, "A");
Assert.equal(co.y, "B");
Assert.equal(co.color, "blue");
Assert.equal(co.toString(), "blue A B");
class ColorPoint2 extends Point1 {
  constructor(x: string, y: string) {
    super(x, y);
  }
}
let co2 = new ColorPoint2("a", "b");
Assert.equal(co2.x, "a");
Assert.equal(co2.y, "b");
Assert.equal(co2.toString(), "a b");
