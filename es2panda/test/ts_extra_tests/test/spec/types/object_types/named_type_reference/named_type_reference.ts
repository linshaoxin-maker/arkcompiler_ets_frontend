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
    Type references to class and interface types are classified as object types.
    Type references to generic class and interface types include type arguments that are substituted for the type parameters of the class
    or interface to produce an actual object type.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class obj {
    num1: number;
    num2: number;
    constructor(num1: number, num2: number) {
        this.num1 = num1;
        this.num2 = num2;
    }
    add(h_x: number, h_y: number): void {
        var sum: number = h_x + h_y;
        Assert.equal(sum, 10);
    }
}
let o: obj = new obj(4, 6);
o.add(3, 7);

interface h_inf {
    h_name: string;
    h_age: number;
    greet: () => string
}
let h_i: h_inf = {
    h_name: 'xiao',
    h_age: 18,
    greet() { return "hello"; }
}
Assert.equal(h_i.h_name, "xiao");
Assert.equal(h_i.h_age, 18);
Assert.equal(h_i.greet(), "hello");