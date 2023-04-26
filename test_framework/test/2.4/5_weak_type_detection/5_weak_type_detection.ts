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
---*/


interface Options {
    data?: string;
    timeout?: number;
    maxRetries?: number;
}

function sendMessage(options: Options) {
    Assert.isTrue(true);
}

const opts = {
    payload: "hello world!",
    retryOnFail: true
};

// Declare the properties if they really do exist.
const opts2 = {
    data: "hello world!"
};
sendMessage(opts2);

// Add an index signature to the weak type (i.e. [propName: string]: {}).
const opts3: {
    [index: string]: { data: string };
} = {};

sendMessage(opts3);

// Use a type assertion (i.e. opts as Options).
const opts4 = {
    payload: "hello world!",
    retryOnFail: true
} as Options;

sendMessage(opts4);