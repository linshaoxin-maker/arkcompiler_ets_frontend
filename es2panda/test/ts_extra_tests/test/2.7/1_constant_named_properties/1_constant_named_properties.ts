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
description: TypeScript 2.7 adds support for declaring const-named properties on types including ECMAScript symbols.
module: ESNext
isCurrent: true
---*/


import { Assert } from "../../../suite/assert.js"

const SERIALIZE = Symbol("serialize-method-key");
interface Test {
  [SERIALIZE](obj: {}): string;
}

class JSONTest implements Test {
  [SERIALIZE](obj: {}) {
    return JSON.stringify(obj);
  }
}
var obj = new JSONTest();
Assert.equal('"test open harmony"', obj[SERIALIZE]("test open harmony"));
Assert.equal(123456, obj[SERIALIZE](123456));


// This also applies to numeric and string literals.
const test01 = "test01";
const test02 = "test02";

let x = {
  [test01]: 100,
  [test02]: "hello"
};


let a = x[test01];

let b = x[test02];
Assert.equal(100, a);
Assert.equal("hello", b);
