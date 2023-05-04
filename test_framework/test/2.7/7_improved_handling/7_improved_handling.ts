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
---*/


class A { }
class B extends A { }
class C extends A { }
class D extends A { }
class E extends D { }

// A
let x1 = !true ? new A() : new B();
// B | C (previously B)
let x2 = !true ? new B() : new C();
// C | D (previously C)
let x3 = !true ? new C() : new D();
Assert.isTrue(x1 instanceof B);
Assert.isTrue(x2 instanceof C);
Assert.isTrue(x3 instanceof D);

// A[]
let a1 = [new A(), new B(), new C(), new D(), new E()];
// (B | C | D)[] (previously B[])
let a2 = [new B(), new C(), new D(), new E()];

Assert.isTrue(a1[0] instanceof A);
Assert.isTrue(a1[1] instanceof B);
Assert.isTrue(a1[2] instanceof C);
Assert.isTrue(a1[3] instanceof D);
Assert.isTrue(a1[4] instanceof E);


Assert.isTrue(a2[0] instanceof B);
Assert.isTrue(a2[1] instanceof C);
Assert.isTrue(a2[2] instanceof D);
Assert.isTrue(a2[3] instanceof E);


function f1(x: B | C | D) {
  if (x instanceof B) {
    Assert.isTrue(x instanceof B);
  } else if (x instanceof C) {
    Assert.isTrue(x instanceof C);
  } else {
    Assert.isFalse(x instanceof B);
    Assert.isFalse(x instanceof C);
  }
}

f1(new A());
f1(new B());
f1(new C());
f1(new D());