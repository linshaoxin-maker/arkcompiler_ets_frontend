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
description: TypeScript 2.7 introduces a new flag called strictPropertyInitialization. This flag performs checks to ensure that each instance property of a class gets initialized in the constructor body, or by a property initializer. 
---*/


class C {
  foo: number = 1;
  foo2!: number;
  bar = "hello";
  baz: boolean = true;

  constructor() {
    this.initialize();
  }

  initialize() {
    this.foo2 = 2;
  }
}

let c = new C();
Assert.equal(1, c.foo);
Assert.equal(2, c.foo2);
Assert.equal('hello', c.bar);
Assert.isTrue(c.baz);