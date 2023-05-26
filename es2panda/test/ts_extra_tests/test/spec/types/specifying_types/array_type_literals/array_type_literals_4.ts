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
    Array types can be written using the 'Array<T>' notation. 
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

var arr: Array<string | number> = [2, 'a'];
Assert.isNumber(arr[0]);
Assert.isString(arr[1]);

var h_x = function () {
    return 'h_x';
}
var h_y = function () {
    return 'h_y';
}
var fun: Array<() => string> = [h_x, h_y];
Assert.isFunction(fun[0]);
Assert.isFunction(fun[1]);