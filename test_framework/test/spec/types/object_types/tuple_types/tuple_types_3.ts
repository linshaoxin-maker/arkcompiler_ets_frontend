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
  Named tuple types can be created by declaring interfaces that derive from Array<T> 
  and introduce numerically named properties
---*/


interface tt<K, V> extends Array<K | V> {
  0: K;
  1: V;
}
let x: tt<number, string> = [10, "ten"];
Assert.isNumber(x[0]);
Assert.equal(x[0], 10);
Assert.isString(x[1]);
Assert.equal(x[1], "ten");
