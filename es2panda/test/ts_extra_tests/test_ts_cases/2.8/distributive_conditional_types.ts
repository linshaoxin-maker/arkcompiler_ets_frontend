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
  Distributive conditional types
module: ESNext
isCurrent: true
---*/


import { Assert } from "../../suite/assert.js"
type TypeGather<T> =
    T extends string ? string :
        T extends number ? number :
            T extends boolean ? boolean :
                T extends undefined ? undefined :
                    T extends Function ? Function :
                        object;


type T0 = TypeGather<string | (() => void)>;
type T2 = TypeGather<string | string[] | undefined>;
type T1 = TypeGather<string[] | number[]>;

type GatherValue<T> = { value: T };
type GatherArray<T> = { array: T[] };
type Gather<T> = T extends any[] ? GatherArray<T[number]> : GatherValue<T>;

type T20 = Gather<string>;
type T21 = Gather<number[]>;
type T22 = Gather<string | number[]>;

let a: T0 = 's';
let b: T0 = (() => {
});
let c: T2 = 's';
let d: T2 = ['s'];
let e: T2 = undefined;
let f: T1 = ['s'];
let g: T1 = [1];
let h: T20 = { value: "s" };
let i: T21 = { array: [1] };
let j: T22 = { value: "s" };
let k: T22 = { array: [1] };

Assert.equal(typeof a, 'string');
Assert.equal(typeof b, 'function');
Assert.equal(typeof c, 'string');
Assert.equal(typeof d, 'object');
Assert.equal(typeof e, 'undefined');
Assert.equal(typeof f, 'object');
Assert.equal(typeof g, 'object');
Assert.equal(typeof h, 'object');
Assert.equal(typeof i, 'object');
Assert.equal(typeof j, 'object');
Assert.equal(typeof k, 'object');



