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
   A property name can be any identifier (including a reserved word), a string literal, a numeric literal,
   or a computed property name. String literals may be used to give properties names that are not valid identifiers,
   such as names containing blanks. Numeric literal property names are equivalent to string literal property names
   with the string representation of the numeric literal, as defined in the ECMAScript specification.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class Property {
  // reserved word as property name
  break: string;
  // string literal as property name
  "with blank": number;
  // numeric literal as property name
  1: boolean;
  constructor(x: string, y: number, z: boolean) {
    this.break = x;
    this["with blank"] = y;
    this[1] = z;
  }
}

var p: Property = new Property("abc", 12, false);
Assert.equal("abc", p.break);
Assert.equal(12, p["with blank"]);
Assert.equal(false, p[1]);