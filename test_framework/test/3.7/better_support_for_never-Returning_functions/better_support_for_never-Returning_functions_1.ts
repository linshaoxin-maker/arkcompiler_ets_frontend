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
  when these never-returning functions are called, TypeScript recognizes that they affect the control flow graph and accounts for them.
  Need to execute the command: npm i -- save dev @ types/node
---*/


function doThingWithString(x: string) {
    return x;
}

function doThingWithNumber(x: number) {
    return x;
}

function dispatch(x: string | number): any {
    if (typeof x === "string") {
        return doThingWithString(x);
    }
    else if (typeof x === "number") {
        return doThingWithNumber(x);
    }
    process.exit(1);
}

Assert.equal(typeof dispatch(10), "number");
Assert.equal(typeof dispatch("hello"), "string");