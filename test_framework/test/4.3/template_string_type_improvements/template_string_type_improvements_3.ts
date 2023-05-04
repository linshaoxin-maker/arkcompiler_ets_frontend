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
    TypeScript can now better-relate, and infer between, different template string types.
 ---*/


let s1: `${number}-${number}-${number}`
let s2: `1-2-3` = `1-2-3`
let s3: `${number}-2-3` = `${4}-2-3`
let s4: `1-${number}-3` = `1-${5}-3`
let s5: `1-2-${number}` = `1-2-${6}`
let s6: `${number}-2-${number}` = `${7}-2-${8}`

// Now *all of these* work
s1 = s2
Assert.equal(s1, "1-2-3");
s1 = s3
Assert.equal(s1, "4-2-3");
s1 = s4
Assert.equal(s1, "1-5-3");
s1 = s5
Assert.equal(s1, "1-2-6");
s1 = s6
Assert.equal(s1, "7-2-8");