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
    A class declaration declares a new named type  called a class type. 
    Within the constructor and instance member functions of a class, the type of this is the this-type  of that class type
 ---*/


class A {
  public x: number;
  public f() { }
  public g(a: any): any {
    return undefined;
  }
  constructor(x: number) {
    this.x = x;
  }
  static s: string;
}
let a = new A(1);
class B extends A {
  public y: number;
  public g(b: boolean) { return false; }
  constructor(x: number, y: number) {
    super(x);
    this.y = y;
  }
}
let b = new B(1, 2);
interface C {
  x: number;
  f: () => void;
  g: (a: any) => any;
}
let c: C = a;
Assert.equal(c.g(1), undefined)
interface D {
  x: number;
  y: number;
  f: () => void;
  g: (b: boolean) => boolean;
}
let d: D = b;
Assert.isFalse(d.g(true))
