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
 description: tuple types now encode their arity into the type of their respective length property. This is accomplished by leveraging numeric literal types, which now allow tuples to be distinct from tuples of different arities.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

interface NumStrTuple extends Array<number | string> {
    0: number;
    1: string;
    length: 2;
}

const numstr01: NumStrTuple = [1, "string"];

interface NumStr extends Array<number | string> {
    0: number;
    1: string;
}

const numstr02: NumStr = [2, "string", 3, "string2"];


Assert.equal(1, numstr01[0]);
Assert.equal("string", numstr01[1]);
Assert.equal(2, numstr02[0]);
Assert.equal("string", numstr02[1]);
Assert.equal(3, numstr02[2]);
Assert.equal("string2", numstr02[3]);