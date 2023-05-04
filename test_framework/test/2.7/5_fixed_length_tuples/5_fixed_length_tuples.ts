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
---*/


interface NumStrTuple extends Array<number | string> {
    0: number;
    1: string;
    // using the numeric literal type '2'
    length: 2;
}

const myNumStrTuple: NumStrTuple = [1, "this is a string"];

// If you need to resort to the original behavior in which tuples only enforce a minimum length, you can use a similar declaration that does not explicitly define a length property
interface MinimumNumStrTuple extends Array<number | string> {
    0: number;
    1: string;
}

const myMinimumNumStrTuple: MinimumNumStrTuple = [2, "this is a string", 3, "two string"];


Assert.equal(1, myNumStrTuple[0]);
Assert.equal("this is a string", myNumStrTuple[1]);
Assert.equal(2, myMinimumNumStrTuple[0]);
Assert.equal("this is a string", myMinimumNumStrTuple[1]);
Assert.equal(3, myMinimumNumStrTuple[2]);
Assert.equal("two string", myMinimumNumStrTuple[3]);