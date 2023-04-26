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
  index signatures now permit union types,as long as they’re a union of infinite-domain primitive types
  specifically:string,number,symbol,template string patterns
  An index signature whose argument is a union of these types will de-sugar into several different index signatures.
options:
  lib:es2015
 ---*/

 
interface DataItem1 {
    [key: string | number | symbol | `info-${string}`]: any;
}
interface DataItem2 {
    [key: string]: any;
    [key: symbol]: any;
    [key: number]: any;
}
interface DataItem3 extends DataItem2 {
    [key: `info-${string}`]: unknown;
}
const one = Symbol(1);
let a: DataItem1 = {};
let b: DataItem3 = {};
b[0] = "string";
b[one] = 1;
b["string"] = "string";
b[`info-hobby`] = "run";
// Successful assignment
a = b;
Assert.equal(a[0], "string");
Assert.equal(a[one], 1);
Assert.equal(a["string"], "string");
Assert.equal(a["info-hobby"], "run");
