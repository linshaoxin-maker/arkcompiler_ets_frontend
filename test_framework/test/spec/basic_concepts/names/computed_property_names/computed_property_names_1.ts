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
  ECMAScript 2015 permits object literals and classes to declare members with computed property names. 
  A computed property name specifies an expression that computes the actual property name at run-time.
options:
  lib: es6
 ---*/

 
class ComputedName {
    // computed property name with a well-known symbol name  have a simple literal type
    aa: 1 | undefined;
    // computed property name have a simple literal type
    ["address"]: string;
    constructor(x: string, y: 1) {
        this.address = x;
        if (y === 1) {
            this.aa = y;
        } else {
            this.aa = undefined;
        }
    }
}
var c: ComputedName = new ComputedName("address No1", 1);
Assert.equal(1, c.aa);
Assert.equal("address No1", c.address);

// computed property name with in object literal
var objectliteral = { ["xx" + "123".length]: 22, name: "string" };
Assert.equal(22, objectliteral.xx3);
