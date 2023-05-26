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
    string index signatures, specified using index type string, define type constraints for all properties and numeric index signatures in the containing type.
    numeric index signatures, specified using index type number, define type constraints for all numerically named properties in the containing type.
 module: ESNext
 isCurrent: true
 ---*/


import {Assert} from '../../../../../suite/assert.js'

var si: { [key: string]: number } = { a: 97, A: 65 };
si["b"] = 98;
Assert.isNumber(si["a"]);
Assert.equal(si["A"], 65);

var ni: { [key: number]: boolean } = { 0: false, 1: true };
ni[-1] = true;
Assert.isBoolean(ni[0]);
Assert.equal(ni[-1], true);

interface UnionKey {
  [key: string | number]: string | number;
}
var uk: UnionKey = { Name: "UnionKey", 0: "NARC", 0x0a: 10 };
Assert.equal(uk["Name"], "UnionKey");
Assert.equal(uk[0], "NARC");
Assert.equal(uk[0x0a], 10);
Assert.equal(uk[0xff], undefined);

interface StringKey {
  [key: string]: string;
}
var sk: StringKey = { "1": "0x01", "2": "0x02", 3: "0x03", "4": "0x04" };
Assert.isString(sk["1"]);
Assert.isString(sk[2]);
Assert.equal(sk[3], 0x03);

interface NumberKey {
  [key: number]: string;
}
var nk: NumberKey = { 1: "0x01", 2: "0x02", "3": "0x03", 4: "0x04" };
Assert.isString(nk["1"]);
Assert.isString(nk[2]);
Assert.equal(nk["3"], "0x03");

var rk1: Record<string, number> = { one: 1, two: 2, three: 3 };
Assert.equal(rk1["one"], 1);
var rk2: Record<"a" | "b" | "c", string> = { a: "A", b: "B", c: "C" };
Assert.equal(rk2["a"], "A");
