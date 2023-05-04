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
   TypeScript 3.5 introduces the new Omit helper type, which creates a new type with some properties dropped from the original.
 ---*/


{
    type Person = {
        name: string;
        age: number;
        location: string;
    };
    type QuantumPersons = Omit<Person, "location">;
    let s: QuantumPersons = {
        name: 'string',
        age: 1,
    }
    let s1: Person = {
        name: 'string',
        age: 1,
        location: 'string'
    }
    Assert.isFalse("location" in s)
    Assert.isTrue("location" in s1)
}