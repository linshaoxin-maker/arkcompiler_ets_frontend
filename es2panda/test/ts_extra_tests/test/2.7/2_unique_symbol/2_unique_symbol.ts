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
 description: To enable treating symbols as unique literals a new type unique symbol is available.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'


declare const usym: unique symbol;

const test03: unique symbol = Symbol();
const test04: unique symbol = Symbol.for("Bar2");

// Works - refers to a unique symbol, but its identity is tied to 'Foo'.
// in order to reference a specific unique symbol, you’ll have to use the typeof operator. 
let t01: typeof test03 = test03;
let t02: typeof test04 = test04;

// Also works.
class C {
  static readonly StaticSymbol: unique symbol = Symbol();
}

Assert.isTrue('symbol' === typeof t01);
Assert.isTrue('symbol' === typeof t02);
Assert.isTrue('symbol' === typeof C.StaticSymbol);