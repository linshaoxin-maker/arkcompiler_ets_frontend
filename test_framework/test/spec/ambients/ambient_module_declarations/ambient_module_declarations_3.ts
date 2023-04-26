///<reference path="ambient_module_declarations_3.d.ts"/>
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
    if an ambient module declaration includes an export assignment, it is an error for any of the declarations within the module to specify an export modifier. 
    if an ambient module declaration contains no export assignment, entities declared in the module are exported regardless of whether their declarations include the optional export modifier.
 module: ESNext
 isCurrent: true
 ---*/


import { AMD3_1, AMD3_2 } from "./ambient_module_declarations_3.d";

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

var am3_1: AMD3_1.AMD3_1IF = { a3_1: false, a3_2: 0 };
var am3_2: AMD3_2.AMD3_2IF = { a3_1: true, a3_2: "T" };
equal(JSON.stringify(am3_1), '{"a3_1":false,"a3_2":0}');
equal(JSON.stringify(am3_2), '{"a3_1":true,"a3_2":"T"}');
