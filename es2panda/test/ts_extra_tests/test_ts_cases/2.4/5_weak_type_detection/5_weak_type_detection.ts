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
 description: TypeScript 2.4 introduces the concept of "weak types". Any type that contains nothing but a set of all-optional properties is considered to be weak.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

interface Options {
    data?: string;
    type?: number;
    value?: number;
}

function check(options: Options) {
    if (options !== undefined && options !== null) {
        return true;
    } else {
        return false;
    }
}

const opts1 = {
    str: "hello world!",
    retryOnFail: true
};


const opts2 = {
    data: "hello world!"
};
Assert.isTrue(check(opts2));


const opts3: {
    [index: string]: { data: string };
} = {};
Assert.isTrue(check(opts3));

const opts4 = {
    payload: "hello world!",
    retryOnFail: true
} as Options;
Assert.isTrue(check(opts4));
