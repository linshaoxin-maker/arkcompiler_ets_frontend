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
  Partial<Type>
  Constructs a type with all properties of Type set to optional. 
  This utility will return a type that represents all subsets of a given type.
 ---*/


interface Person {
    name: string;
    age: number;
    location: string;
}
type PartialPerson = Partial<Person>;

let cc: Person = {
    name: "caihua",
    age: 20,
    location: "earth",
};

let dd: PartialPerson;
dd = {};
Assert.equal(dd.name, undefined);
Assert.equal(dd.age, undefined);
Assert.equal(dd.location, undefined);
dd = { name: "caihua" };
Assert.equal(dd.name, "caihua");
Assert.equal(dd.age, undefined);
Assert.equal(dd.location, undefined);
dd = { age: 20 };
Assert.equal(dd.name, undefined);
Assert.equal(dd.age, 20);
Assert.equal(dd.location, undefined);
dd = { location: "earth" };
Assert.equal(dd.name, undefined);
Assert.equal(dd.age, undefined);
Assert.equal(dd.location, "earth");
dd = { name: "caihua", age: 20 };
Assert.equal(dd.name, "caihua");
Assert.equal(dd.age, 20);
Assert.equal(dd.location, undefined);
dd = { name: "caihua", location: "earth" };
dd = { age: 20, location: "earth" };
dd = { name: "caihua", age: 20, location: "earth" };
Assert.equal(dd.name, "caihua");
Assert.equal(dd.age, 20);
Assert.equal(dd.location, "earth");