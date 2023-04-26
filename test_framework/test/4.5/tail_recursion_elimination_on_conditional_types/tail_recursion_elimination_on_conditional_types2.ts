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
  The following type won't be optimized, since it uses the result of a conditional type by adding it to a union.
  You can introduce a helper that takes an "accumulator" type parameter to make it tail-recursive
---*/


type GetChars<S> =
  S extends `${infer Char}${infer Rest}` ? Char | GetChars<Rest> : never;
type gc = GetChars<"                getChar">
var g1: gc = 'e'
Assert.isString(g1)


type GetChars1<S> = GetCharsHelper<S, never>;
type GetCharsHelper<S, Acc> =
  S extends `${infer Char}${infer Rest}` ? GetCharsHelper<Rest, Char | Acc> : Acc;

type gch = GetCharsHelper<string, number>
var g2: gch = 10
Assert.isNumber(g2)
