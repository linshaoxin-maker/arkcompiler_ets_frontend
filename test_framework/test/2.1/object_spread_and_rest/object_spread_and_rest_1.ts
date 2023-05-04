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
description: spreading an object can be handy to get a shallow copy.
 ---*/


let person = {
    name: "caihua",
    age: 20,
};
let job = { job: "stu" };
let loc = {
    location: "mars",
    pets: {
        type: "dog",
        name: "ahuang",
    },
};

let copy = { ...person, ...job, ...loc };

Assert.equal(copy.name, person.name);
Assert.equal(copy.age, person.age);
Assert.equal(copy.job, job.job);
Assert.equal(copy.location, loc.location);
Assert.equal(copy.pets.name, loc.pets.name);
Assert.equal(copy.pets.type, loc.pets.type);
// shallow copy
Assert.equal(copy.name == person.name, true);
Assert.equal(copy.age == person.age, true);
Assert.equal(copy.job == job.job, true);
Assert.equal(copy.location == loc.location, true);
Assert.equal(copy.pets == loc.pets, true);
Assert.equal(copy.pets.type == loc.pets.type, true);
Assert.equal(copy.pets.name == loc.pets.name, true);
