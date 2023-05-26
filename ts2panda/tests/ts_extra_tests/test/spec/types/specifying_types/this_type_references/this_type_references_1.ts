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
   The meaning of a ThisType depends on the closest enclosing FunctionDeclaration, 
   FunctionExpression, PropertyDefinition, ClassElement, or TypeMember, known as the root declaration of the ThisType,
   when the root declaration is an instance member or constructor of a class, the ThisType references the this-type of that class.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class myClass {
    num: number;
    constructor(num: number) {
        this.num = num;
    }
    get() {
        return this;
    }
}

var my_c = new myClass(10);
Assert.equal(typeof my_c.get(), 'object');