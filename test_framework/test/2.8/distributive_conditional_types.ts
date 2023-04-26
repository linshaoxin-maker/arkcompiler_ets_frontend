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


import TypeName from './conditional_types';


type T0 = TypeName<string | (() => void)>;
type T2 = TypeName<string | string[] | undefined>;
type T1 = TypeName<string[] | number[]>;

type BoxedValue<T> = { value: T };
type BoxedArray<T> = { array: T[] };
type Boxed<T> = T extends any[] ? BoxedArray<T[number]> : BoxedValue<T>;

type T20 = Boxed<string>;
type T21 = Boxed<number[]>;
type T22 = Boxed<string | number[]>;

let a: T0 = 's';
let b: T0 = (() => {
});
let c: T2 = 's';
let d: T2 = ['s'];
let e: T2 = undefined;
let f: T1 = ['s'];
let g: T1 = [1];
let h: T20 = {value: "s"};
let i: T21 = {array: [1]};
let j: T22 = {value: "s"};
let k: T22 = {array: [1]};


class AssertionError extends Error {
    constructor(public msg: string) {
        super();
        this.msg = "";
        this.msg = msg;
    }
}

function defaultMessage(actual: any, expect: any, flag: boolean = true) {
    if (flag == true) {
        return "expected '" + expect + "' ,but was '" + actual + "'.";
    } else {
        return "expected not '" + expect + "' ,but was '" + actual + "'.";
    }

}

function equal(actual: any, expect: any, msg?: string) {
    if (actual != expect) {
        throw new AssertionError(msg ? msg : defaultMessage(actual, expect));
    }
}

equal(typeof a, 'string');
equal(typeof b, 'function');
equal(typeof c, 'string');
equal(typeof d, 'object');
equal(typeof e, 'undefined');
equal(typeof f, 'object');
equal(typeof g, 'object');
equal(typeof h, 'object');
equal(typeof i, 'object');
equal(typeof j, 'object');
equal(typeof k, 'object');



