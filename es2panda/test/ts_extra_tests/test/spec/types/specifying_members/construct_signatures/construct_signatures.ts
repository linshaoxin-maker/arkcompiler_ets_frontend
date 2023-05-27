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
 description: A type may overload new operations by defining multiple construct signatures with different parameter lists.
 module: ESNext
 isCurrent: true
 ---*/


import {Assert} from '../../../../../suite/assert.js'

class Animal {
    name: string | undefined;
    age: number | undefined;
    constructor();
    constructor(name: string);
    constructor(age: number);
    constructor(nameorage?: string | number, age?: number) {
        if (typeof nameorage == "number") {
            this.age = nameorage;
        }
        if (typeof nameorage == "string") {
            this.name = nameorage;
        }
        if (age) {
            this.age = age;
        }
    }
}
type AnimalConstructor = {
    new (name: string, age: number): Animal;
    new (name: string): Animal;
    new (age: number): Animal;
    new (): Animal;
};

const AnimalConstructor: AnimalConstructor = Animal;

let tt1 = new AnimalConstructor();
Assert.isUndefined(tt1.age);
Assert.isUndefined(tt1.name);
let tt2 = new AnimalConstructor("caihua2", 12);
Assert.equal(tt2.name, "caihua2");
Assert.equal(tt2.age, 12);
let tt3 = new AnimalConstructor("caihua3");
Assert.equal(tt3.name, "caihua3");
Assert.isUndefined(tt3.age);
let tt4 = new AnimalConstructor(1230);
Assert.equal(tt4.age, 1230);
Assert.isUndefined(tt4.name);
