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
    All instance property members of a class must satisfy 
    the constraints implied by the index members of the class as specified in section 3.9.4.
 ---*/


class ind {
  name: string;
  [index: string]: number | string;
  constructor(name: string) {
    this.name = name;
  }
}
let a = new ind("aa");
a.name = "pig";
a.age = 12;
Assert.equal(a.age, 12);
a.add = "qindao";
Assert.equal(a.add, "qindao");
