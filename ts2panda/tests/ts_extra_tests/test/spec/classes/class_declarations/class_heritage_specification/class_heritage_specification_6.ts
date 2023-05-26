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
  A class that includes an extends clause is called a derived class, 
  and the class specified in the extends clause is called the base class of the derived class. 
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class Person {
    name: string;
    age: number;
    constructor(name: string, age: number) {
        this.name = name;
        this.age = age;
    }
    introduce() {
        return 'I am a college student';
    }
}
class Student extends Person {
    constructor(name: string, age: number) {
        super(name, age);
    }
}
var m_C = new Student('xiao', 18);
Assert.equal(m_C.introduce(), 'I am a college student');