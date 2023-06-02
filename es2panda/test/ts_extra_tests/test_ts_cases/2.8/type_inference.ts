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
   Type inference in conditional types
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../suite/assert.js'

{
  type Type00<T> =
    T extends (infer U)[] ? U :
    T extends (...args: any[]) => infer U ? U :
    T extends Promise<infer U> ? U :
    T;

  // string
  type Type0 = Type00<string>;
  // string
  type Type1 = Type00<string[]>;
  // string
  type Type2 = Type00<() => string>;
  // string
  type Type3 = Type00<Promise<string>>;
  // string
  type Type4 = Type00<Type00<Promise<string>[]>>;

  let a: Type0 = 's';
  let b: Type1 = 's';
  let c: Type2 = 's';
  let d: Type3 = 's';
  let e: Type4 = 's';

  Assert.equal(typeof a, 'string');
  Assert.equal(typeof b, 'string');
  Assert.equal(typeof c, 'string');
  Assert.equal(typeof d, 'string');
  Assert.equal(typeof e, 'string');

  type F<T> = T extends { a: infer U, b: infer U } ? U : never;
  // string
  type Type5 = F<{ a: string, b: string }>;
  // string | number
  type Type6 = F<{ a: string, b: number }>;

  let f: Type5 = 's';
  let g: Type6 = 's';
  let h: Type6 = 1;

  Assert.equal(typeof f, 'string');
  Assert.equal(typeof g, 'string');
  Assert.equal(typeof h, 'number');
};