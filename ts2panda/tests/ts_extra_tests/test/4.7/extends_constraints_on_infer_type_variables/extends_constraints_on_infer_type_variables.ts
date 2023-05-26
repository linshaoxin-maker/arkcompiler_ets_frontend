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
   Conditional types are a bit of a power-user feature.
   They allow us to match and infer against the shape of types, and make decisions based on them.
   To avoid that second level of nesting, TypeScript 4.7 now allows to place a constraint on any infer type.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../../suite/assert.js"

type FStr1<T> = T extends [infer S, ...unknown[]]
  ? S extends string
  ? S
  : boolean
  : boolean;

type FStr2<T> = T extends [infer S extends string, ...unknown[]] ? S : boolean;

let A1: FStr1<[string, number, number]> = "A1";
let A2: FStr2<[string, number, number]> = "A2";
Assert.isString(A1);
Assert.isString(A2);

let B1: FStr1<["hello", number, number]> = "hello";
let B2: FStr2<["hello", number, number]> = "hello";
Assert.equal(B1, "hello");
Assert.equal(B2, "hello");

let C1: FStr1<["hello" | "world", boolean]> = "hello";
Assert.equal(C1, "hello");
let C2: FStr2<["hello" | "world", boolean]> = "world";
Assert.equal(C2, "world");
C1 = "world";
Assert.equal(C1, "world");
C2 = "hello";
Assert.equal(C2, "hello");

let D1: FStr1<[object, number, string]> = false;
Assert.isBoolean(D1);
let D2: FStr2<[object, number, string]> = false;
Assert.isBoolean(D2);
