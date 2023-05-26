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
 description: A signature can be declared with a rest parameter whose type is a discriminated union of tuples.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../suite/assert.js'

type HWT01 = (...args: ["a", number] | ["b", string]) => void;

const hwf: HWT01 = (kind, payload) => {
    if (kind === "a") {
        Assert.equal(42, payload.toFixed());
    }
    if (kind === "b") {
        Assert.equal("HELLO", payload.toUpperCase());
    }
};

hwf("a", 42);
hwf("b", "hello");