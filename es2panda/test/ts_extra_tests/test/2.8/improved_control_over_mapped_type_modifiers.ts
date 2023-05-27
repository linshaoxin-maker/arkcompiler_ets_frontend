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
   Improved control over mapped type modifiers
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../suite/assert.js'

{
  type OA = { a: string };
  type OB = { b: string };


  type T1 = keyof (OA & OB);

  type T2<T> = keyof (T & OB);

  type T3<U> = keyof (OA & U);

  type T4<T, U> = keyof (T & U);

  type T5 = T2<OA>;

  type T6 = T3<OB>;

  type T7 = T4<OA, OB>;

  let a: T1 = 'a';
  let b: T1 = "b";
  let c: T5 = "a";
  let d: T6 = "a";
  let e: T7 = "a";

  Assert.equal(a, 'a');
  Assert.equal(b, 'b');
  Assert.equal(c, 'a');
  Assert.equal(d, 'a');
  Assert.equal(e, 'a');
};
