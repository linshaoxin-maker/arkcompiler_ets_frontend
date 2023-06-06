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
    S is a subtype of a type T, and T is a supertype of S,
    if S has no excess properties with respect to T,
    and M is a non-specialized call or construct signature and S has an apparent call or construct signature N where,
    when M and N are instantiated using type Any as the type argument for all type parameters declared by M and N (if any),
    and for each member M in T, M is a property and S has an apparent property N where,
    the signatures are of the same kind (call or construct),
    M has a rest parameter or the number of non-optional parameters in N is less than or equal to the total number of parameters in M,
    for parameter positions that are present in both signatures, each parameter type in N is a subtype or supertype of the corresponding parameter type in M, 
    and the result type of M is Void, or the result type of N is a subtype of that of M.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

interface T {
    (x: any, y: any): void
}
interface S {
    (x: number): void
}
let t: T = (x, y): void => {
    return;
}
let s: S = (x): void => {
    return;
}

t = s;
Assert.equal(t, s);