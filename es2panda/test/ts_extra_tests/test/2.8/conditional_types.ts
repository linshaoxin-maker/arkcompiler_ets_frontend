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
  Conditional Types.
module: ESNext
isCurrent: true
 ---*/


import { Assert } from "../../suite/assert.js"
type TypeGather<T> =
  T extends string ? string :
  T extends number ? number :
  T extends boolean ? boolean :
  T extends undefined ? undefined :
  T extends Function ? Function :
  object;

type T0 = TypeGather<string>;
type T1 = TypeGather<"a">;
type T2 = TypeGather<true>;
type T3 = TypeGather<() => void>;
type T4 = TypeGather<string[]>;


let a: T0 = "string";
let b: T1 = 'string';
let c: T2 = true;
let d: T3 = (() => { });
let e: T4 = {};


Assert.equal(typeof a, 'string');
Assert.equal(typeof b, 'string');
Assert.equal(typeof c, 'boolean');
Assert.equal(typeof d, 'function');
Assert.equal(typeof e, 'object');
