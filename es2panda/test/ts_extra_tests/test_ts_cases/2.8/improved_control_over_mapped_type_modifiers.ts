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
   With TypeScript 2.8 keyof applied to an intersection type is transformed to a union of keyof applied to each intersection constituent. 
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../suite/assert.js'

{
  type myType1 = 'a' | 4;
  type myType2 = { str: string };
  interface I{
    mem: string;
  }

  type T1 = keyof (myType1 & myType2);
  type T2<T> = keyof (T & myType2);
  type T3<U> = keyof (myType1 & U);
  type T4<T, U> = keyof (T & U);
  type T5 = T2<myType1>;
  type T6 = T3<myType2>;
  type T7 = T4<myType1, myType2>;

  // The type of  a,b,c,d is "toString" | "valueOf" | "str"
  let a: T1 = 'str';
  a = 'toString';
  a = 'valueOf';
  Assert.equal(a, 'valueOf');
  let b: T5 = "str";
  b = 'toString';
  b = 'valueOf';
  Assert.equal(b, 'valueOf');
  let c: T6 = "str";
  c = 'toString';
  c = 'valueOf';
  Assert.equal(c, 'valueOf');
  let d: T7 = "str";
  d = 'toString';
  d = 'valueOf';
  Assert.equal(d, 'valueOf');

  type T11 = keyof (I & myType2);
  type T22<T> = keyof (T & myType2);
  type T33<U> = keyof (I & U);
  type T44<T, U> = keyof (T & U);
  type T55 = T22<I>;
  type T66 = T33<myType2>;
  type T77 = T44<I, myType2>;

  let aa: T11 = 'mem';
  aa = 'str';
  Assert.equal(aa, 'str');
  let bb: T55  = 'mem';
  bb = 'str';
  Assert.equal(bb, 'str');
  let cc: T66 = 'mem';
  cc = 'str';
  Assert.equal(cc, 'str');
  let dd: T77 = 'mem';
  dd = 'str';
  Assert.equal(dd, 'str');
  
};
