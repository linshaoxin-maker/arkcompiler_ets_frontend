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
   An object type literal defines an object type by specifying the set of members that are statically considered to be present in instances of the type. 
   Object type literals can be given names using interface declarations but are otherwise anonymous.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

var h_obj: {
    num: number;
    str: string;
    boo: boolean;
} = {
    num: 5,
    str: 'str',
    boo: true
}
Assert.equal(h_obj.num, 5);
Assert.equal(h_obj.str, 'str');
Assert.equal(h_obj.boo, true);

interface h_i {
    name: string,
    age: number
}
var h_o: h_i = {
    name: 'xiao',
    age: 18
}
Assert.equal(h_o.name, 'xiao');
Assert.equal(h_o.age, 18);