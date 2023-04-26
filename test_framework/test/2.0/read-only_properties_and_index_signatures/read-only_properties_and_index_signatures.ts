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
  A property or index signature can now be declared with the readonly modifier is considered read-only.
  Read-only properties may have initializers and may be assigned to in constructors within the same class declaration, but otherwise assignments to read-only properties are disallowed 
 ---*/


interface Point {
  readonly x: number;
  readonly y: number;
}
var p1: Point = { x: 10, y: 20 };
var p2 = { x: 1, y: 1 };
// Ok, read-only alias for p2
var p3: Point = p2;
Assert.equal(JSON.stringify(p3), '{"x":1,"y":1}');
// Ok, but also changes p3.x because of aliasing
p2.x = 5;

class Foo {
  readonly a = 1;
  readonly b: string;
  constructor() {
    // Assignment permitted in constructor
    this.b = "hello";
  }
}

let a: Array<number> = [0, 1, 2, 3, 4];
let b: ReadonlyArray<number> = a;
Assert.equal(JSON.stringify(b), '[0,1,2,3,4]');
