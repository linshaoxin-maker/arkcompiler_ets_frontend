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
   When union, intersection, function, or constructor types are used as array element types they must be enclosed in parentheses. 
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

var h_test1: string | number[];
h_test1 = 'string';
Assert.isString(h_test1);
h_test1 = [3, 5];
Assert.isNumber(h_test1[0]);
Assert.isNumber(h_test1[1]);

var h_test2: (string | number)[];
h_test2 = [2, 'b'];
Assert.isNumber(h_test2[0]);
Assert.isString(h_test2[1]);