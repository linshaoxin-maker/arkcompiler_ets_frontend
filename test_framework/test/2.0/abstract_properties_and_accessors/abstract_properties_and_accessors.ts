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
  An abstract class can declare abstract properties and/or accessors. Any sub class will need to declare the abstract properties or be marked as abstract. Abstract properties cannot have an initializer. Abstract accessors cannot have bodies.
 ---*/

abstract class Base {
  abstract name: string;
  abstract get value();
  abstract set value(v: number);
}
class Derived extends Base {
  name = "derived";
  value = 1;
}
let d = new Derived();
Assert.equal(d.name, "derived");
Assert.equal(d.value, 1);

