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
description：indexed access types, also called lookup types.
 ---*/


class Person {
    name: string;
    age: number;
    // must initialize, or cannot pass python script
    constructor(name: string, age: number, job: string) {
        this.name = name;
        this.age = age;
    }
}

type TestName = Person["name"];
let tt1: TestName = "";
Assert.equal(typeof tt1, "string");
Assert.notEqual(typeof tt1, "number");
Assert.notEqual(typeof tt1, "boolean");

type TestAge = Person["age"];
let tt2: TestAge = 0;
Assert.equal(typeof tt2, "number");
Assert.notEqual(typeof tt2, "string");
Assert.notEqual(typeof tt2, "boolean");
