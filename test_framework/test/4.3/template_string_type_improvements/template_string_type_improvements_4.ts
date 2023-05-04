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
    TypeScript add better inference capabilities
 ---*/


function foo<V extends string>(arg: `*${V}*`): `*${V}*` {
    return arg
}
function test<T extends string>(s: string, n: number, b: boolean, t: T) {
    // "hello"
    let x1 = foo("*hello*");
    Assert.equal(x1, '*hello*')

    // "*hello*"
    let x2 = foo("**hello**");
    Assert.equal(x2, '**hello**')

    // string
    let x3 = foo(`*${s}*` as const);
    Assert.equal(x3, '*s*')

    // `${number}`
    let x4 = foo(`*${n}*` as const);
    Assert.equal(x4, '*5*')

    // "true" | "false"
    let x5 = foo(`*${b}*` as const);
    Assert.equal(x5, '*true*')

    // `${T}`
    let x6 = foo(`*${t}*` as const);
    Assert.equal(x6, '*t*')

    // `*${string}*`
    let x7 = foo(`**${s}**` as const);
    Assert.equal(x7, '**s**')
}

test('s', 5, true, 't')