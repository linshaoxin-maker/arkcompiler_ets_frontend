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
    Template string types are types that either construct new string-like types by concatenating,
    or match patterns of other string-like types.
 ---*/


type Color = "red" | "blue"
type Quantity = "one" | "two"

// SeussFish1 and SeussFish2 are of the same type
type SeussFish1 = `${Quantity | Color} fish`
type SeussFish2 = "one fish" | "two fish" | "red fish" | "blue fish"
var fish1: SeussFish1 = "blue fish"
var fish2: SeussFish2 = fish1
fish1 = "one fish"
fish2 = fish1
fish1 = "red fish"
fish2 = fish1
fish1 = "two fish"
fish2 = fish1

let s1: `${number}-${number}-${number}`
let s2: `1-2-3` = `1-2-3`
s1 = s2
Assert.equal(s1, '1-2-3')