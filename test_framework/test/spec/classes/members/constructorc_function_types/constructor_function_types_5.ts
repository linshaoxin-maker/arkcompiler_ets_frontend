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
   Every class automatically contains a static property member named 'prototype', 
   the type of which is the containing class with type Any substituted for each type parameter.
 ---*/


class Pair<T1, T2> {
  constructor(public item1: T1, public item2: T2) { }
}
class TwoArrays<T> extends Pair<T[], T[]> { }
let x: number = 1;
let y: number = 2;
let p = new Pair(x, y);
Assert.equal(p.item1, 1);
let x2: string = "one";
let y2: string = "two";
let p2 = new Pair(x2, y2);
Assert.equal(p2.item1, "one");
let x3: boolean = true;
let p3 = new Pair(x3, y);
Assert.isBoolean(p3.item1);
