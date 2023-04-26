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
   A class declaration declares a class type and a constructor function: class BindingIdentifieropt TypeParametersopt ClassHeritage { ClassBody }
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
class MyClass<T> {
  field: T;
  constructor(field: T) {
    this.field = field;
  }

  public getFieldName(): T {
    return this.field;
  }
}
let mc: MyClass<String> = new MyClass<String>("a");
Assert.equal("a", mc.field);
let mc2: MyClass<number> = new MyClass<number>(1);
Assert.equal(1, mc2.field);
let mc3: MyClass<boolean> = new MyClass<boolean>(false);
Assert.equal(false, mc3.field);
let mc4: MyClass<ChildPoint> = new MyClass<ChildPoint>(pChild);
Assert.isTrue(mc4.field instanceof ChildPoint);
