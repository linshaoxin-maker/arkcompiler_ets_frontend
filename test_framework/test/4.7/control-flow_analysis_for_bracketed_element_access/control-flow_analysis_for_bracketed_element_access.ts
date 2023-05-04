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
  TypeScript 4.7 now narrows the types of element accesses when the indexed keys are literal types and unique symbols.
 ---*/


const key = Symbol()

const numberOrString = Math.random() < 0.5 ? 42 : "hello"

const obj = {
    [key]: numberOrString,
}

if (typeof obj[key] === "string") {
    let str = obj[key].toUpperCase()
    Assert.equal(str, 'HELLO')
}