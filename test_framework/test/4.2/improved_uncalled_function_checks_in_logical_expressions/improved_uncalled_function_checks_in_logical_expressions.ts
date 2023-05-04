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
  TypeScript’s uncalled function checks now apply within && and || expressions.
---*/


class FUN {
  add2?(a: number, b: number): number;
  add3(a: number, b: number, c: number) {
    return a + b + c;
  }
}
let fff = new FUN();
Assert.isUndefined(fff.add2);
Assert.isFunction(fff.add3);
Assert.isUndefined(fff.add2 && fff.add3);
Assert.isFunction(fff.add2 || fff.add3);





