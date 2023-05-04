///<reference path="ambient_module_declarations_4.d.ts"/>
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
    ambient modules are "open-ended" and ambient module declarations with the same string literal name contribute to a single module.
 module: ESNext
 isCurrent: true
 ---*/


import * as dmname from "dmname"
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

var am4_1: dmname.AMD4_1IF = { a4_1: true, a4_2: false };
equal(JSON.stringify(am4_1), '{"a4_1":true,"a4_2":false}')

var am4_2: dmname.AMD4_2IF = { a4_1: 1, a4_2: 0 };
equal(JSON.stringify(am4_2), '{"a4_1":1,"a4_2":0}')
