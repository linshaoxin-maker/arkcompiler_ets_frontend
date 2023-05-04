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
  Given a type parameter T and set of candidate types, the actual inferred type argument is determined,
  if at least one of the candidate types is a supertype of all of the other candidate types, 
  let C denote the widened form of the first such candidate type. 
  If C satisfies T's constraint, the inferred type argument for T is C. 
  Otherwise, the inferred type argument for T is T's constraint.
 ---*/


type c = number | undefined
type t = number | null | undefined
function choose<T extends t>(x: T, y: T): T {
    return Math.random() < 0.5 ? x : y;
}

var y = choose(10, undefined)
if (typeof y === 'number') {
    Assert.isNumber(y)
}
else {
    Assert.isUndefined(y)
}