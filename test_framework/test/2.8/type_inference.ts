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
   Type inference in conditional types
 ---*/


{
    type Unpacked<T> =
        T extends (infer U)[] ? U :
        T extends (...args: any[]) => infer U ? U :
        T extends Promise<infer U> ? U :
        T;

    // string
    type T0 = Unpacked<string>;
    // string
    type T1 = Unpacked<string[]>;
    // string
    type T2 = Unpacked<() => string>;
    // string
    type T3 = Unpacked<Promise<string>>;
    // string
    type T4 = Unpacked<Unpacked<Promise<string>[]>>;

    let a: T0 = 's';
    let b: T1 = 's';
    let c: T2 = 's';
    let d: T3 = 's';
    let e: T4 = 's';

    Assert.equal(typeof a, 'string');
    Assert.equal(typeof b, 'string');
    Assert.equal(typeof c, 'string');
    Assert.equal(typeof d, 'string');
    Assert.equal(typeof e, 'string');

    type Foo<T> = T extends { a: infer U, b: infer U } ? U : never;
    // string
    type T10 = Foo<{ a: string, b: string }>;
    // string | number
    type T11 = Foo<{ a: string, b: number }>;

    let f: T10 = 's';
    let g: T11 = 's';
    let h: T11 = 1;

    Assert.equal(typeof f, 'string');
    Assert.equal(typeof g, 'string');
    Assert.equal(typeof h, 'number');
}