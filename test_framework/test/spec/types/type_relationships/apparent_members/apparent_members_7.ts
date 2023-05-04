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
  When one or more constituent types of I have an apparent numeric index signature, 
  I has an apparent numeric index signature of an intersection type of the respective numeric index signature types.
 ---*/


interface Array1 {
    [index1: string]: number
}
interface Array2 {
    [index2: string]: number
}

function test(obj: Array1 & Array2) {
    return "right"
}
const stu1 = {
    age: 19
}
const stu2 = {
    number: 10001
}
Assert.equal(test(stu1), "right")
Assert.equal(test(stu2), "right")