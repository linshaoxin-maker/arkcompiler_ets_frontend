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
   TypeScript 3.4 introduces a new construct for literal values called const assertions. Its syntax is a type assertion with const in place of the type name (e.g. 123 as const).
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../suite/assert.js"
let a = 1408 as const;
Assert.equal(a, 1408);
let b = 'NARC' as const;
Assert.equal(b, 'NARC');
let c = [255, 0, 0] as const;
Assert.equal(JSON.stringify(c), '[255,0,0]');
