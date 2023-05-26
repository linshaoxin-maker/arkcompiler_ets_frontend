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
    typescript 3.0 introduces a new top type unknown. unknown is the type-safe counterpart of any. 
    anything is assignable to unknown, but unknown isn't assignable to anything but itself and any without a type assertion or a control flow based narrowing. 
    likewise, no operations are permitted on an unknown without first asserting or narrowing to a more specific type.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

type HWT00 = unknown & null;
type HWT01 = unknown & undefined;
type HWT02 = unknown & null & undefined;
type HWT03 = unknown & string;
type HWT04 = unknown & string[];
type HWT05 = unknown & unknown;
type HWT06 = unknown & any;

type HWT10 = unknown | null;
type HWT11 = unknown | undefined;
type HWT12 = unknown | null | undefined;
type HWT13 = unknown | string;
type HWT14 = unknown | string[];
type HWT15 = unknown | unknown;
type HWT16 = unknown | any;

type HWT20<T> = T & {};
type HWT21<T> = T | {};
type HWT22<T> = T & unknown;
type HWT23<T> = T | unknown;

type HWT30<T> = unknown extends T ? true : false;
type HWT31<T> = T extends unknown ? true : false;
type HWT32<T> = never extends T ? true : false;
type HWT33<T> = T extends never ? true : false;

type HWT40 = keyof any;
type HWT41 = keyof unknown;

function hwfun01(x: unknown): unknown {
    x = 10;
    return x;
}
Assert.equal(hwfun01(0), 10);

declare function isFunction(x: unknown): x is Function;

function hwfun02(x: unknown) {
    return typeof x;
}
Assert.equal(hwfun02("N"), "string");

type HWT50<T> = { [P in keyof T]: number };
type HWT51 = HWT50<any>;
type HWT52 = HWT50<unknown>;

function hwfun03<T>(pAny: any, pundefined: undefined, pT: T) {
    let x: unknown;
    x = 123;
    Assert.isNumber(x);
    x = "hello";
    Assert.isString(x);
    x = [1, 2, 3];
    x = new Error();
    x = x;
    x = pAny;
    x = pundefined;
    x = pT;
}
hwfun03(1024, undefined, "A");

function hwfun04(x: unknown) {
    let v1: any = x;
    let v2: unknown = x;
}
hwfun04(1024);

function hwfun05(x: { [x: string]: unknown }) {
    x = {};
    x = { a: 5 };
}

function hwfun06() {
    let x: unknown;
    let y = x;
}

function hwfun07(): unknown {
    let x: unknown;
    return x;
}

class C1 {
    a: string = "";
    b: unknown;
    c: any;
}

let x: number = 1024;
(x as unknown as string).length;
