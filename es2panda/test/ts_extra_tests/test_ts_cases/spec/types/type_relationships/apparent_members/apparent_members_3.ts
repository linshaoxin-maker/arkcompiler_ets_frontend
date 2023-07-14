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
    The apparent members of an object type T are the combination of the following:
    The declared and/or inherited members of T.
    The properties of the global interface type 'Object' that aren't hidden by properties with the same name in T.
    If T has one or more call or construct signatures, the properties of the global interface type 'Function' that aren't hidden by properties with the same name in T.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

let obj: Object;
obj = {
    toString() {
        return '123';
    }
}
Assert.notEqual(obj.toString, "123");

class Person {
    name?: string;
    age?: number;
    toString(): string {
        return "hello"
    }
}
Assert.notEqual(Person.toString(), "hello");