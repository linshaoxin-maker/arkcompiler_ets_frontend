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
    A signature is said to be an applicable signature with respect to an argument list when
    the number of arguments is not less than the number of required parameters,
    the number of arguments is not greater than the number of parameters, and
    for each argument expression e and its corresponding parameter P, when e is contextually typed by the type of P, 
    no errors ensue and the type of e is assignable to the type of P.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

interface IfnCall {
    (name: string, age?: number): string
}
const foo1: IfnCall = function (name, age) {
    return name + ":" + age
}
const foo2: IfnCall = function (name) {
    return name
}
var f1 = foo1("xiao", 18)
var f2 = foo2("xi")
f1 = f2
Assert.equal(f1, f2);