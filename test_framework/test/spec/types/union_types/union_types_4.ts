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
      a union type U is assignable to a type T if each type in U is assignable to T.
      a type T is assignable to a union type U if T is assignable to any type in U.
 ---*/


type numType = { num: number };
type strType = { str: string };
type boolType = { bool: boolean };
type objType = { obj: Object };
var u4a: any;
var u4: numType | strType | boolType | objType | undefined;
var u4_1: numType = { num: 0xCA };
u4a = u4_1;
Assert.equal(JSON.stringify(u4a), '{"num":202}');

var u4_2: strType = { str: "QWER" };
u4a = u4_2;
Assert.equal(JSON.stringify(u4a), '{"str":"QWER"}');

var u4_3: boolType = { bool: false };
u4a = u4_3;
Assert.equal(JSON.stringify(u4a), '{"bool":false}');

var u4_4: objType = { obj: { 0: "ZERO" } };
u4a = u4_4;
Assert.equal(JSON.stringify(u4a), '{"obj":{"0":"ZERO"}}');


u4 = { num: 0xCA, str: "ABC", bool: false, obj: u4_4 };
u4a = u4;
Assert.equal(JSON.stringify(u4a), '{"num":202,"str":"ABC","bool":false,"obj":{"obj":{"0":"ZERO"}}}');


u4 = u4_1;
Assert.equal(JSON.stringify(u4), '{"num":202}');

u4 = u4_2;
Assert.equal(JSON.stringify(u4), '{"str":"QWER"}');

u4 = u4_3;
Assert.equal(JSON.stringify(u4), '{"bool":false}');

u4 = u4_4;
Assert.equal(JSON.stringify(u4), '{"obj":{"0":"ZERO"}}');
