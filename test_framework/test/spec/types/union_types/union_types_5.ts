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
      the || and conditional operators may produce values of union types, and array literals may produce array values that have union types as their element types.
 ---*/


var ns5: A1 = 1408 || "NARC";
var ns5_1: number | string;
var ns5_2: string | number;
ns5 = 1500;
ns5_2 = ns5 <= 1408 ? "NARC" : 1024;
Assert.equal(ns5_2, 1024);
ns5 = 100;
ns5_2 = ns5 <= 1408 ? "NARC" : 1024;
Assert.equal(ns5_2, "NARC");

type A1 = string | number | object;
type B1 = number | boolean | string;
var ns5_3: A1 & B1;
ns5_3 = 125;
Assert.isNumber(ns5_3);
ns5_3 = "Fn";
Assert.isString(ns5_3);

var arruni1: Array<number | boolean> | Array<boolean | string>;
arruni1 = [0, true, -1, false];
Assert.equal(JSON.stringify(arruni1), '[0,true,-1,false]');
arruni1 = [true, "True", false, "False"];
Assert.equal(JSON.stringify(arruni1), '[true,"True",false,"False"]');

var arruni2: number[] | boolean[];
arruni2 = [2, 4, 6];
Assert.equal(JSON.stringify(arruni2), '[2,4,6]');
arruni2 = [true, false];
Assert.equal(JSON.stringify(arruni2), '[true,false]');

var arruni3: (number | string)[] | (boolean | Object)[];
arruni3 = [1, 3, 5, "AND", "OR"];
Assert.equal(JSON.stringify(arruni3), '[1,3,5,"AND","OR"]');
arruni3 = [true, false, { 0x00: "0x00" }, { 0xFA: "0xFA" }];
Assert.equal(JSON.stringify(arruni3), '[true,false,{"0":"0x00"},{"250":"0xFA"}]');