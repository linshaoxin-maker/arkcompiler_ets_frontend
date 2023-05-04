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
    typescript 4.8 brings a series of correctness and consistency improvements under --strictNullChecks. These changes affect how intersection and union types work, and are leveraged in how TypeScript narrows types.
    on their own, these changes may appear small - but they represent fixes for many many paper cuts that have been reported over several years.
 ---*/


function f(x: unknown, y: {} | null | undefined) {
    x = y;
    y = x;
}
f(0, {});
f(undefined, {});
f(null, {});
f(0, null);
f(0, undefined);
f(undefined, null);
f(undefined, undefined);
f(null, undefined);
f(null, null);

let n: NonNullable<string> = "\"type NonNullable<T> = T extends null | undefined ? never : T; \" Is reduced to \"type NonNullable<T> = T & {};\""
Assert.equal(n, "\"type NonNullable<T> = T extends null | undefined ? never : T; \" Is reduced to \"type NonNullable<T> = T & {};\"");

function foo<T>(x: NonNullable<T>, y: NonNullable<NonNullable<T>>) {
    x = y;
    y = x;
    return "NonNullable<NonNullable<T>> now simplifies at least to NonNullable<T>, whereas it didn't before.";
}
let fo = foo<string>("NonNullable<T>", "NonNullable<NonNullable<T>>");
let s = "NonNullable<NonNullable<T>> now simplifies at least to NonNullable<T>, whereas it didn't before.";
Assert.equal(fo, s);

function narrowUnknownishUnion(x: {} | null | undefined) {
    if (x) {
        x;
        return "(parameter) x: {}";
    }
    else {
        x;
        return "(parameter) x: {} | null | undefined";
    }
}
function narrowUnknown(x: unknown) {
    if (x) {
        x;
        return "(parameter) x: {}";
    }
    else {
        x;
        return "(parameter) x: unknown";
    }
}
let x1_1 = narrowUnknownishUnion(1);
let x1_2 = narrowUnknownishUnion(null);
let x2_1 = narrowUnknown(1);
let x2_2 = narrowUnknown(undefined);
let s1 = "(parameter) x: {}";
let s2 = "(parameter) x: {} | null | undefined";
let s3 = "(parameter) x: unknown";
Assert.equal(x1_1, s1);
Assert.equal(x1_2, s2);
Assert.equal(x2_1, s1);
Assert.equal(x2_2, s3);

function throwIfNullable<T>(value: T): NonNullable<T> | string {
    if (value === undefined || value === null) {
        return "Nullable value!"
    }
    return value;
}

Assert.equal(throwIfNullable<any>(null), "Nullable value!");
Assert.equal(throwIfNullable<any>(1024), 1024);