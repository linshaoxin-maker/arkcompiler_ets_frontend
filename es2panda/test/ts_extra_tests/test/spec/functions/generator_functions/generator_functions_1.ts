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
  for..of loops over an iterable object, invoking the Symbol.iterator property on the object.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../suite/assert.js'

let someArray = [1, "string", false];

for (let item of someArray) {
    if (typeof item === "number") {
        Assert.equal(item, 1);
    } else if (typeof item === "string") {
        Assert.equal(item, "string");
    } else if (typeof item === "boolean") {
        Assert.equal(item, false);
    }
};
