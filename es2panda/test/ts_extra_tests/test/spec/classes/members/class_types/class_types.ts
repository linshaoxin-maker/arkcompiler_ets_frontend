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
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class h_A {
  public h_x: number;
  public h_f() { }
  public h_g(h_a: any): any {
    return undefined;
  }
  constructor(h_x: number) {
    this.h_x = h_x;
  }
  static h_s: string;
}
let h_a = new h_A(1);
class h_B extends h_A {
  public h_y: number;
  public h_g(h_b: boolean) { return false; }
  constructor(h_x: number, h_y: number) {
    super(h_x);
    this.h_y = h_y;
  }
}
let h_b = new h_B(1, 2);
interface h_C {
  h_x: number;
  h_f: () => void;
  h_g: (h_a: any) => any;
}
let h_c: h_C = h_a;
Assert.equal(h_c.h_g(1), undefined);
interface h_D {
  h_x: number;
  h_y: number;
  h_f: () => void;
  h_g: (h_b: boolean) => boolean;
}
let h_d: h_D = h_b;
Assert.isFalse(h_d.h_g(true));
