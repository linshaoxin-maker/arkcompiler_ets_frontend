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
    A property member in a derived class is said to override a property member in a base class 
    when the derived class property member has the same name and kind (instance or static) as the base class property member. 
 ---*/


class Shape {
  color: string = "black";
  switchColor() {
    this.color = this.color === "black" ? "white" : "black";
  }
}
class Circle extends Shape {
  color: string = "red";

  switchColor() {
    this.color = this.color === "red" ? "green" : "red";
  }
}
const circle = new Circle();
let a = circle.color;
Assert.equal(a, "red");
circle.switchColor();
let b = circle.color;
Assert.equal(b, "green");
