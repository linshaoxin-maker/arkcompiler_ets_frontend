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
description: TypeScript 2.0 relaxes this constraint and allows duplicate identifiers across blocks, as long as they have identical types.
---*/


interface ErrorA {
    stack?: string;
}

interface ErrorB {
    code?: string;
    path?: string;
    stack?: string;  // OK
}

let sa: ErrorA = {
    stack: "123456"
};

let sb: ErrorB = {
    code: "29189",
    path: "/path/ts/doc",
    stack: "65867967",
};

Assert.equal(sa.stack, "123456");
Assert.equal(sb.stack, "65867967");