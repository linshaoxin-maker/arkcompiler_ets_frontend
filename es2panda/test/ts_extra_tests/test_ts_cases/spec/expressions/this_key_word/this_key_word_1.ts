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
    In a constructor, instance member function, instance member accessor,
    or instance member variable initializer, this is of the this-type (section 3.6.3) of the containing class.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../suite/assert.js'

class Dog {
    name: string;
    age: number;
    constructor(name: string, age: number) {
        this.name = name;
        this.age = age;
    }

    getName(): string {
        return this.name;
    }

    bark(): string {
        return this.getName() + " is bark";
    }
}
var dog = new Dog("doggy", 7);
Assert.equal(dog.age, 7);
Assert.equal("doggy is bark", dog.bark())
class Duck {
    age: number = this.b;
    constructor(private readonly b: number) {
    }
}
var duck = new Duck(6);
Assert.equal(6, duck.age);