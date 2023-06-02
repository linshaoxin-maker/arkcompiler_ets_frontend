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
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

interface PersonTest {
    name: string;
    age: number;
    location: string;
}
type PersonType = Partial<PersonTest>;

let pt1: PersonTest = {
    name: "caihua",
    age: 20,
    location: "earth",
};

let pt2: PersonType;
pt2 = {};
Assert.equal(pt2.name, undefined);
Assert.equal(pt2.age, undefined);
Assert.equal(pt2.location, undefined);
pt2 = { name: "caihua" };
Assert.equal(pt2.name, "caihua");
Assert.equal(pt2.age, undefined);
Assert.equal(pt2.location, undefined);
pt2 = { age: 20 };
Assert.equal(pt2.name, undefined);
Assert.equal(pt2.age, 20);
Assert.equal(pt2.location, undefined);
pt2 = { location: "earth" };
Assert.equal(pt2.name, undefined);
Assert.equal(pt2.age, undefined);
Assert.equal(pt2.location, "earth");
pt2 = { name: "caihua", age: 20 };
Assert.equal(pt2.name, "caihua");
Assert.equal(pt2.age, 20);
Assert.equal(pt2.location, undefined);
pt2 = { name: "caihua", location: "earth" };
pt2 = { age: 20, location: "earth" };
pt2 = { name: "caihua", age: 20, location: "earth" };
Assert.equal(pt2.name, "caihua");
Assert.equal(pt2.age, 20);
Assert.equal(pt2.location, "earth");