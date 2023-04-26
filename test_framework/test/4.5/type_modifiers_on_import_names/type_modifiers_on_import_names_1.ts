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
  That’s part of why TypeScript 4.5 allows a type modifier on individual named imports.
  BaseType is always guaranteed to be erased and someFunc will be preserved under preserveValueImports.
module: ESNext
isCurrent: true
---*/


import { someFunc, type BaseType } from "./some-module.js";

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

function isFunction(fun: any, msg?: string) {
    equal(typeof fun, "function", msg);
}

function isObject(obj: any, msg?: string) {
    equal(typeof obj, "object", msg);
}

export class Thing implements BaseType {
    someMethod() {
        someFunc();
    }
}

const thing = new Thing();
isFunction(Thing, "function");
isObject(thing, "object");