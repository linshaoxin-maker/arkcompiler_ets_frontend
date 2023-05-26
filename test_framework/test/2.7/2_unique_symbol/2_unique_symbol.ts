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
---*/


// Works
declare const Foo: unique symbol;

// unique symbol is a subtype of symbol, and are produced only from calling Symbol() or Symbol.for(), or from explicit type annotations. 
const Bar: unique symbol = Symbol();
const Bar2: unique symbol = Symbol.for("Bar2");

// Works - refers to a unique symbol, but its identity is tied to 'Foo'.
// in order to reference a specific unique symbol, you’ll have to use the typeof operator. 
let Baz: typeof Bar = Bar;
let Baz2: typeof Bar2 = Bar2;

// Also works.
class C {
  static readonly StaticSymbol: unique symbol = Symbol();
}

Assert.isTrue('symbol' === typeof Baz);
Assert.isTrue('symbol' === typeof Baz2);
Assert.isTrue('symbol' === typeof C.StaticSymbol);