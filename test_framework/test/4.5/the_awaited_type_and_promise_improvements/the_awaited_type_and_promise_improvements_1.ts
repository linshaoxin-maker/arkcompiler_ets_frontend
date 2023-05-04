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
    TypeScript 4.5 introduces a new utility type called the Awaited type. 
    This type is meant to model operations like await in async functions,
    or the .then() method on Promises - specifically, the way that they recursively unwrap Promises;
    some of the problems around inference with Promise.all served as motivations for Awaited.
 module: ESNext
 isCurrent: true
---*/


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

function isObject(obj: any, msg?: string) {
    equal(typeof obj, "object", msg);
}

function MaybePromise<T>(value: number): number | Promise<number> | PromiseLike<number> {
    return new Promise((resolve, reject) => {
        resolve(Math.random())
    });
}

async function doSomething(): Promise<[number, number]> {
    const result = await Promise.all([MaybePromise(100), MaybePromise(200)]);
    return result;
}

let D = doSomething().then(res => {
    isObject(res, "object");
});
// E = void;
type E = Awaited<Promise<typeof D>>;

let e: E;
equal(e, undefined);