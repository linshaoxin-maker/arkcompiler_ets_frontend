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
  The apparent members of the primitive type Boolean are the apparent members of the global interface type 'Boolean'.
  The apparent members of the primitive type String and all string literal types are the apparent members of the global interface type 'String'
 ---*/


let bool: Boolean = true;
Assert.isBoolean(bool.valueOf())

let str1: String = "string";
type color = "RED" | "BLUE" | "BALCK"
let str2: color = "BLUE";
str1.toString;
str2.toString;
Assert.equal(typeof str1.valueOf(), typeof str2.valueOf());