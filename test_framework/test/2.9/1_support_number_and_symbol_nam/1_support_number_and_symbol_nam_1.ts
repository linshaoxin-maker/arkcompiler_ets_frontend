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
description: TypeScript 2.9 adds support for number and symbol named properties in index types and mapped types. 
---*/


const c = "c";
const d = 10;
const e = Symbol();
const f = true;

const enum E1 { A, B, C }
const enum E2 { A = "A", B = "B", C = "C" }

// An index type keyof T for some type T is a subtype of string | number | symbol.
type Foo = {
    a: string;
    5: string;
    // A mapped type { [P in K]: XXX } permits any K assignable to string | number | symbol.
    [c]: string;
    [d]: string;
    [e]: string;
    // f is boolean, so it can not be used here, like: [f]: string; 
    [E1.A]: string;
    [E2.A]: string;
}

// In a for...in statement for an object of a generic type T, the inferred type of the iteration variable was previously keyof T but is now Extract<keyof T, string>. (In other words, the subset of keyof T that includes only string-like values.)
type K1 = keyof Foo;
type K2 = Extract<keyof Foo, string>;
type K3 = Extract<keyof Foo, number>;
type K4 = Extract<keyof Foo, symbol>;


let k1: K1 = 'c';
let k2: K2 = E2.A;
let k3: K3 = 10;
Assert.equal("c", k1);
Assert.equal(E2.A, k2);
Assert.equal(10, k3);