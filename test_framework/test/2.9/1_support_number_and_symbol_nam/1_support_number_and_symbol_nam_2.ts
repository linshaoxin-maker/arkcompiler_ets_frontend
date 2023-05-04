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
description: Since keyof now reflects the presence of a numeric index signature by including type number in the key type, mapped types such as Partial<T> and Readonly<T> work correctly when applied to object types with numeric index signatures.
---*/


type Arrayish<T> = {
    length: number;
    [x: number]: T;
}

type ReadonlyArrayish<T> = Readonly<Arrayish<T>>;

let map: ReadonlyArrayish<string> = {
    [1]: "b",
    [2]: "a",
    length: 2,
};


let n = map.length;
let x = map[1];
Assert.equal(2, n);
Assert.equal("b", x);