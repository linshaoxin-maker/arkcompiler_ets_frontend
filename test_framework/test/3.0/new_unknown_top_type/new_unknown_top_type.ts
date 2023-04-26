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
 ---*/


type T00 = unknown & null;
type T01 = unknown & undefined;
type T02 = unknown & null & undefined;
type T03 = unknown & string;
type T04 = unknown & string[];
type T05 = unknown & unknown;
type T06 = unknown & any;

type T10 = unknown | null;
type T11 = unknown | undefined;
type T12 = unknown | null | undefined;
type T13 = unknown | string;
type T14 = unknown | string[];
type T15 = unknown | unknown;
type T16 = unknown | any;

type T20<T> = T & {};
type T21<T> = T | {};
type T22<T> = T & unknown;
type T23<T> = T | unknown;

type T30<T> = unknown extends T ? true : false;
type T31<T> = T extends unknown ? true : false;
type T32<T> = never extends T ? true : false;
type T33<T> = T extends never ? true : false;

type T40 = keyof any;
type T41 = keyof unknown;

function f10(x: unknown): unknown {
    x = 10;
    return x;
}
Assert.equal(f10(0), 10);

declare function isFunction(x: unknown): x is Function;

function f20(x: unknown) {
    return typeof x;
}
Assert.equal(f20("N"), "string");

type T50<T> = { [P in keyof T]: number };
type T51 = T50<any>;
type T52 = T50<unknown>;

function f21<T>(pAny: any, pundefined: undefined, pT: T) {
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
f21(1024, undefined, "A");

function f22(x: unknown) {
    let v1: any = x;
    let v2: unknown = x;
}
f22(1024);

function f24(x: { [x: string]: unknown }) {
    x = {};
    x = { a: 5 };
}

function f25() {
    let x: unknown;
    let y = x;
}

function f27(): unknown {
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
