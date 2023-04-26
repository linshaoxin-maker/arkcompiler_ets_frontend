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
 ---*/


{
  type A = { a: string };
  type B = { b: string };

  // "a" | "b"
  type T1 = keyof (A & B);
  // keyof T | "b"
  type T2<T> = keyof (T & B);
  // "a" | keyof U
  type T3<U> = keyof (A & U);
  // keyof T | keyof U
  type T4<T, U> = keyof (T & U);
  // "a" | "b"
  type T5 = T2<A>;
  // "a" | "b"
  type T6 = T3<B>;
  // "a" | "b"
  type T7 = T4<A, B>;

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
}

