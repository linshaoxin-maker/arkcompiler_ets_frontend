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
    In the body of a static member function declaration, the type of this is the constructor function type.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class Point {
  constructor(public x: number = 3, public y: number = 3) { }
  static z: number;
  static pro() {
    return this.prototype;
  }
  static returnz() {
    this.z;
  }
}
Assert.equal(Point.pro(), "[object Object]");
Assert.equal(Point.returnz(), undefined);