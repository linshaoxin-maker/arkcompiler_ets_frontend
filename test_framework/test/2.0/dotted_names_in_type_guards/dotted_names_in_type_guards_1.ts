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
description: Type guards now support checking “dotted names” consisting of a variable or parameter name followed one or more property accesses.
---*/


interface Options {
    location?: {
        x?: number;
        y?: number;
    };
}

function foo(options?: Options) {
    if (options && options.location && options.location.x) {
        const x = options.location.x;
        Assert.equal(typeof x, "number");
    }

    if (options && options.location && options.location.y) {
        const x = options.location.y;
        Assert.equal(typeof x, "number");
    }
}

let options: Options = {
    location: {
        x: 10,
        y: 20,
    }
};
foo(options);