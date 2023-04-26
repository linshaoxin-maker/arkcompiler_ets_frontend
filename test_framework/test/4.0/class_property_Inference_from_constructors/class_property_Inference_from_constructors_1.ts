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
    TypeScript 4.0 can now use control flow analysis to determine the types of properties in classes when noImplicitAny is enabled.
 ---*/

 
class personW {
  name: string;
  age: number | undefined;
  constructor(name: string) {
    this.name = name;
  }
  setAge(age: number) {
    if (age >= 0) {
      this.age = age;
    }
  }
}
const W = new personW("tom");
Assert.equal(W.name, "tom");

class squareD {
  area: number | undefined;

  sideLength: number | undefined;
  constructor(sideLength: number) {
    this.sideLength = sideLength;
    this.area = sideLength * 2;
  }
}
const D = new squareD(6);
Assert.equal(D.area, 12);

class Square {
  sideLength!: number;
  constructor(sideLength: number) {
    this.initialize(sideLength);
  }

  initialize(sideLength: number) {
    this.sideLength = sideLength;
  }

  get area() {
    return this.sideLength ** 2;
  }
}
const b = new Square(8);
Assert.equal(b.area, 64);
