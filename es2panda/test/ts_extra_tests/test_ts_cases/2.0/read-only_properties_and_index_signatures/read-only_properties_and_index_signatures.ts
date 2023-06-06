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
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

interface Demo {
  readonly x: number;
  readonly y: number;
}
let d2 = { x: 1, y: 1 };

let d3: Demo = d2;
Assert.equal(JSON.stringify(d3), '{"x":1,"y":1}');

d2.x = 5;

Assert.equal(d3.x, 5)

let x: Array<number> = [0, 1, 2];
let y: ReadonlyArray<number> = x;
Assert.equal(JSON.stringify(y), '[0,1,2]');

class Demo2 {
  readonly x = 1;
  readonly y: string;
  constructor() {
    this.y = "demo";
  }

}

Assert.equal(new Demo2().y, "demo");
