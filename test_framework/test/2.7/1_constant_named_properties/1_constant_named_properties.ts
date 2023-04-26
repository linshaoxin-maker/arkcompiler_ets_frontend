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


import { SERIALIZE, Serializable } from "./lib.js";

// consumer
class JSONSerializableItem implements Serializable {
  [SERIALIZE](obj: {}) {
    return JSON.stringify(obj);
  }
}
var obj = new JSONSerializableItem();
equal('"test open harmony"', obj[SERIALIZE]("test open harmony"));
equal(123456, obj[SERIALIZE](123456));


// This also applies to numeric and string literals.
const Foo = "Foo";
const Bar = "Bar";

let x = {
  [Foo]: 100,
  [Bar]: "hello"
};

// has type 'number'
let a = x[Foo];
// has type 'string'
let b = x[Bar];
equal(100, a);
equal("hello", b);

class AssertionError extends Error {
  constructor(public msg: string) {
    super();
    this.msg = "";
    this.msg = msg;
  }
}

function defaultMessage(actual: any, expect: any, flag: boolean = true) {
  if (flag == true) {
    return "expected '" + expect + "' ,but was '" + actual + "'.";
  } else {
    return "expected not '" + expect + "' ,but was '" + actual + "'.";
  }

}

function equal(actual: any, expect: any, msg?: string) {
  if (actual != expect) {
    throw new AssertionError(msg ? msg : defaultMessage(actual, expect));
  }
}