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
      for purposes of property access and function calls, the apparent members of a union type are those that are present in every one of its constituent types, 
      with types that are unions of the respective apparent members in the constituent types.
 ---*/


type numType = { num: number };
type strType = { str: string };
type boolType = { bool: boolean };
type objType = { obj: Object };
type NS = numType | strType;
type OB = objType | boolType;
var nsv: NS = { num: 1024, str: "NS" };
var obv: OB = { bool: true, obj: { 0xFF: "0xFF" } };
var nsobv_1: NS | OB = { num: 1024, obj: { 0xFF: "0xFF" } }
var nsobv_2: NS | OB = { bool: false, str: "nsobv_2" };
var nsobv_3: NS | OB = { num: 114, bool: false, str: "nsobv_3", obj: { 0xAF: "0xAF" } }
Assert.equal(typeof nsv, "object");
Assert.equal(typeof obv, "object");
Assert.equal(typeof nsobv_1, "object");
Assert.equal(typeof nsobv_2, "object");
Assert.equal(typeof nsobv_3, "object");