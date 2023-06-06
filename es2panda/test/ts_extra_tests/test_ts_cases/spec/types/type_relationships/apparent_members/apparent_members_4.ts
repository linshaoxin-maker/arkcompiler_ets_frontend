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
  When all constituent types of U have an apparent string index signature, 
  U has an apparent string index signature of a union type of the respective string index signature types.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

interface Array1 {
    [index1: string]: string
}
interface Array2 {
    [index2: string]: string
}

function test(obj: Array1 | Array2) {
    return "right"
}
const stu1 = {
    age: "19"
}
const stu2 = {
    name: "xiao"
}
Assert.equal(test(stu1), "right");
Assert.equal(test(stu2), "right");