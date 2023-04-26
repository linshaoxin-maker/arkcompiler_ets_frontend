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
    this appendix contains a summary of the grammar found in the main document.
    typescript grammar is a superset of the grammar defined in the ECMAScript 2015 Language Specification (specifically, the ECMA-262 Standard, 6th Edition) and this appendix lists only productions that are new or modified from the ECMAScript grammar.
 module: ESNext
 isCurrent: true
 ---*/


import * as sam from "./scripts_and_modules_cp.js"
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
equal(sam.pi, 3.14);

let a100 = sam.area(10);
equal(a100, 314);

let cr = new sam.Color(0, 255, 0);
equal(cr.toColorJSON(), "[0,255,0]");

let w: sam.Weapon = { Damage: 12500, DamageType: "XO" };
let wjson = JSON.stringify(w);
equal(wjson, "{\"Damage\":12500,\"DamageType\":\"XO\"}");

equal(sam.CommandE.end, -1);
equal(sam.CommandE.stop, 0);
equal(sam.CommandE.run, 1);

let p: sam.AE.PointXYZ = { x: -1, y: -1, z: -1 };
let pjson = JSON.stringify(p);
equal(pjson, "{\"x\":-1,\"y\":-1,\"z\":-1}");

