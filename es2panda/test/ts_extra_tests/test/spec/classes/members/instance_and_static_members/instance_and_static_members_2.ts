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
    Instance members are members of the class type and its associated this-type. Within constructors, 
    instance member functions, and instance member accessors, 
    the type of this is the this-type of the class.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class Counter {
  private count: number = 0;
  constructor(count_: number) {
    this.count = count_;
  }
  public add(): this {
    this.count++;
    return this;
  }
  public subtract(): this {
    this.count--;
    return this;
  }

  public getResult(): number {
    return this.count;
  }
}
const counter = new Counter(1);
counter.add();
Assert.equal(counter.getResult(), 2);
counter.add();
counter.subtract();
Assert.equal(counter.getResult(), 2);
