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
  they can have optional elements and rest elements, and can even have labels for tooling and readability.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'


let c: [string, string?] = ["hello"];
c = ["hello", "world"];

let d: [first: string, second?: string] = ["hello"];
d = ["hello", "world"];

let e: [string, string, ...boolean[]];

e = ["hello", "world"];
e = ["hello", "world", false];
e = ["hello", "world", true, false, true];

Assert.equal(JSON.stringify(c), "[\"hello\",\"world\"]");
Assert.equal(JSON.stringify(e), "[\"hello\",\"world\",true,false,true]");