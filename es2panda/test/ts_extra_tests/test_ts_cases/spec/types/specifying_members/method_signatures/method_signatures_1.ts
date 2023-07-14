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
     If the PropertyName is followed by a question mark, the property is optional. 
     Only object type literals and interfaces can declare optional properties.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

interface point {
  point(x: number, y: number, z?: number): number;
}
class P implements point {
  point(x: number, y: number, z?: number | undefined): number {
    if (z) return x + y + z;
    return x + y;
  }
}
let p1 = new P();
let x: number = 1;
let y: number = 2;
let z: number = 3;
let sum = p1.point(x, y, z);
Assert.equal(sum, 6);
sum = p1.point(x, y);
Assert.equal(sum, 3);

let point2: { x: number; y: number; z?: number };
point2 = { x: 1, y: 2, z: 3 };
let pp1 = point2;
Assert.equal(pp1.x, 1);
Assert.equal(pp1.y, 2);
point2 = { x: 0, y: 0, z: 0 };
let pp2 = point2;
Assert.equal(pp2.x, 0);
Assert.equal(pp2.y, 0);
Assert.equal(pp2.z, 0);