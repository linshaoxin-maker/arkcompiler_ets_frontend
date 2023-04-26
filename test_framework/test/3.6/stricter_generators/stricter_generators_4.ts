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
  To correctly represent the types that can be passed in to a generator from calls to next(), 
  TypeScript 3.6 also infers certain uses of yield within the body of a generator function.
options:
  lib:es2015
 ---*/

 
function* foo() {
    let x: number = yield;
    Assert.isNumber(x);
}
let x = foo();
x.next();
x.next(1);

