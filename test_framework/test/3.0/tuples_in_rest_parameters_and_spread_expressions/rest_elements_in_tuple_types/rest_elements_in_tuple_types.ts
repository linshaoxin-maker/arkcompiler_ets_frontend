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
    the last element of a tuple type can be a rest element of the form ...X, where X is an array type. A rest element indicates that the tuple type is open-ended and may have zero or more additional elements of the array element type.
    the type of the length property of a tuple type with a rest element is number.
 ---*/


function tuple<T extends any[]>(...args: T): T {
    return args;
}
function getArrayOfNumbers0_10() {
    let arr: number[] = [];
    for (let i = 0; i <= 10; i++) {
        arr.push(i);
    }
    return arr;
}
const numbers: number[] = getArrayOfNumbers0_10();
const t1 = tuple("foo", 1, true);
const t2 = tuple("bar", ...numbers);
const t3 = ["A", true, ...numbers, false];
Assert.equal(JSON.stringify(t1), "[\"foo\",1,true]");
Assert.equal(JSON.stringify(t2), "[\"bar\",0,1,2,3,4,5,6,7,8,9,10]");
Assert.equal(JSON.stringify(t3), "[\"A\",true,0,1,2,3,4,5,6,7,8,9,10,false]");
Assert.isNumber(t1.length);