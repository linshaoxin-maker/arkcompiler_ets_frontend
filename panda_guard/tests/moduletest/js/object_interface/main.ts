/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

interface PersonProperties {
    name: string,
    age: number
}

interface PersonMethods {
    onTest1(num: number): number,

    onTest2(num: number): number
}

interface Person extends PersonProperties, PersonMethods {
}

const person: Person = {
    name: 'bob',
    age: 30,
    onTest1(num: number) {
        return num + 1;
    },
    onTest2(num: number) {
        return num + 2;
    }
};

print(`1 :` + person.name);
print(`2 :` + person.age);
print(`3 :` + person.onTest1(1));
print(`4 :` + person.onTest2(2));
