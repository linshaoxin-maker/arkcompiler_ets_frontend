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
    Type arguments for call signature type parameters may be explicitly specified in a call operation or may, 
    when possible, be inferred from the types of the regular arguments in the call.
 ---*/


function identity<T>(x: T): T {
  return x;
}
let x: number = 3;
// CHECK explicitly specified type in call
Assert.isNumber(<number>identity(x));
let y = "string";
// CHECK inferred from argument in the call
Assert.isString(identity(y));
