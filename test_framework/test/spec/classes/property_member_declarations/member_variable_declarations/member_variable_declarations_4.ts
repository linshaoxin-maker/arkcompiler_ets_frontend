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
    Instance member variable initializers are equivalent to assignments to properties of this in the constructor
 ---*/


class Employee {
  constructor(public name: string, public address: string, public retired = false) { }
}
class Employee2 {
  public name: string;
  public address: string;
  public retired: boolean;
  constructor(name: string, address: string, retired: boolean = false) {
    this.name = name;
    this.address = address;
    this.retired = retired;
  }
}
let em1 = new Employee("name", "qindao", false);
let em2 = new Employee2("name", "qingdao", true);
Assert.equal(em1.retired, false);
Assert.equal(em2.retired, true);

