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
  TypeScript 2.7 improves the handling of structurally identical classes in union types and instanceof expressions:
  Structurally identical, but distinct, class types are now preserved in union types (instead of eliminating all but one).
  Union type subtype reduction only removes a class type if it is a subclass of and derives from another class type in the union.
  Type checking of the instanceof operator is now based on whether the type of the left operand derives from the type indicated by the right operand (as opposed to a structural subtype check).
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

class HWA { }
class HWB extends HWA { }
class HWC extends HWA { }
class HWD extends HWA { }
class HWE extends HWD { }


let x1 = !true ? new HWA() : new HWB();

let x2 = !true ? new HWB() : new HWC();

let x3 = !true ? new HWC() : new HWD();
Assert.isTrue(x1 instanceof HWB);
Assert.isTrue(x2 instanceof HWC);
Assert.isTrue(x3 instanceof HWD);


let a1 = [new HWA(), new HWB(), new HWC(), new HWD(), new HWE()];

let a2 = [new HWB(), new HWC(), new HWD(), new HWE()];

Assert.isTrue(a1[0] instanceof HWA);
Assert.isTrue(a1[1] instanceof HWB);
Assert.isTrue(a1[2] instanceof HWC);
Assert.isTrue(a1[3] instanceof HWD);
Assert.isTrue(a1[4] instanceof HWE);


Assert.isTrue(a2[0] instanceof HWB);
Assert.isTrue(a2[1] instanceof HWC);
Assert.isTrue(a2[2] instanceof HWD);
Assert.isTrue(a2[3] instanceof HWE);


function fun(x: HWB | HWC | HWD) {
  if (x instanceof HWB) {
    Assert.isTrue(x instanceof HWB);
  } else if (x instanceof HWC) {
    Assert.isTrue(x instanceof HWC);
  } else {
    Assert.isFalse(x instanceof HWB);
    Assert.isFalse(x instanceof HWC);
  }
}

fun(new HWA());
fun(new HWB());
fun(new HWC());
fun(new HWD());