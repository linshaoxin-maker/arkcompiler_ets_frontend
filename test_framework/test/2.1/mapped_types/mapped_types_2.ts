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
  Readonly<Type>
  Constructs a type with all properties of Type set to readonly, 
  meaning the properties of the constructed type cannot be reassigned.
 ---*/

 
interface Person {
    name: string;
    age: number;
    location: string;
}
type ReadonlyPerson = Readonly<Person>;

let cc: Person = {
    name: "caihua",
    age: 20,
    location: "earth",
};
cc.name = "caihua1";
Assert.notEqual(cc.name, "caihua");
cc.age = 15;
Assert.notEqual(cc.age, 20);
cc.location = "Mars";
Assert.notEqual(cc.location, "earth");

let dd: ReadonlyPerson = {
    name: "caihua",
    age: 20,
    location: "earth",
};
