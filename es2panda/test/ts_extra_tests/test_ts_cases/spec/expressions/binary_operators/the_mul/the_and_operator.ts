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
   The '&' operator require its operands to be of type Any, the Number primitive type, or an enum type.
   Operands of an enum type are treated as having the primitive type Number.
   If one operand is the null or undefined value, it is treated as having the type of the other operand.
   The result is always of the Number primitive type.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

var a: any = '10';
var b: any = true;
var c: number = 9;
var x = a & b;
var y = a & c;
var bb = b & b;
var cc = c & c;
Assert.isNumber(x);
Assert.equal(x, 0);
Assert.isNumber(y);
Assert.equal(y, 8);
Assert.isNumber(bb);
Assert.equal(bb, 1);
Assert.isNumber(cc);
Assert.equal(cc, 9);
enum e {
  A,
  B,
  C
}
var d = e.C;
var z = a & d;
Assert.isNumber(z);
Assert.equal(z, 2);