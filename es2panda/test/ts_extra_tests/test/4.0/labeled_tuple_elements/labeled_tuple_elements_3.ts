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
    There are a few rules when using labeled tuples. 
    For one, when labeling a tuple element, all other elements in the tuple must also be labeled.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

type A01 = [name: string, age: number];
type A02 = [name?: string, age?: number];

const t: [string, number, boolean] = ["hello", 123, true];
Assert.equal(t[0], "hello");
Assert.equal(t[1], 123);
Assert.equal(t[2], true);
