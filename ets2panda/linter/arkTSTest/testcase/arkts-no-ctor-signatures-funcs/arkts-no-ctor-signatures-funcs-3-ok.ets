/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 

class Person {
  public name: string;
  public age: number;

  constructor(name: string, age: number) {
    this.name = name;
    this.age = age;
  }
}

// define a personCtor type that represents the constructor type
type personCtor = (name: string, age: number) => Person;

// define a create function that takes a constructor and arguments and returns an instance of Person 
function create(ctor: personCtor, name: string, age: number): Person {
  return ctor(name, age);
}

// define a impersonizer type that represents the constructor type
let impersonizer: personCtor = (name: string, age: number): Person => {
  return new Person(name, age);
}

// create intrance of Person with create function
const person = create(impersonizer, 'John', 30);
console.log(person.name); // John
console.log(person.age); // 30