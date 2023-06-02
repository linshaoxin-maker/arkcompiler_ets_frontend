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
 description: Improved Recursion Depth Checks
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../suite/assert.js'

// In a structural type system, object types are compatible based on the members they have.
interface HWA<T> {
    prop: HWA<HWA<T>>;
}

interface HWB<T> {
    prop: HWB<HWB<T>>;
}

function hwtest(source: HWA<string>, target: HWB<number>) {
    target = source;
}


interface HWC<T> {
    prop: T;
}

let x: HWC<HWC<HWC<HWC<HWC<HWC<string>>>>>>;
let y: HWC<HWC<HWC<HWC<HWC<string>>>>>;


y = { prop: { prop: { prop: { prop: { prop: "prop" } } } } }
x = { prop: y };

Assert.isString("x = y; this is error");
Assert.notEqual(JSON.stringify(x), JSON.stringify(y));