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
    array types represent JavaScript arrays with a common element type.
    array types are named type references created from the generic interface type 'Array' in the global namespace with the array element type as a type argument.
 module: ESNext
 isCurrent: true
 ---*/


import {Assert} from '../../../../../suite/assert.js'

var arr1: Array<string> = ["ABC", "DEF", "GHI"];
var arr2: Array<number> = [1, 3, 5, 7, 9];
var arr3: Array<boolean> = [true, false];
var arr4: Array<object> = [Object, { 0xff: "0xFF" }];
Assert.isString(arr1[2]);
var objArray: object[] = [];
arr1.forEach(function (element, index, arr) {
  objArray[index] = { index: index, element: element, arr: arr };
});
Assert.isString(arr2.toString());
Assert.equal(arr2.toString(), "1,3,5,7,9");
Assert.isNumber(arr3.length);
Assert.equal(arr3.length, 2);
Assert.isNumber(arr2.pop());
Assert.equal(arr2.pop(), 7);
arr2.push(15);
Assert.equal(arr2.toString(), "1,3,5,15");
arr3[0] = false;
Assert.equal(arr3[0], false);
