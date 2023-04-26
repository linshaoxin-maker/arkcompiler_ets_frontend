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
 ---*/

 
type PersonA = [name: string, age: number];
type PartialPersonA = [name?: string, age?: number];

const myTuple: [string, number, boolean] = ["hello", 123, true];
Assert.equal(myTuple[0], "hello");
Assert.equal(myTuple[1], 123);
Assert.equal(myTuple[2], true);
