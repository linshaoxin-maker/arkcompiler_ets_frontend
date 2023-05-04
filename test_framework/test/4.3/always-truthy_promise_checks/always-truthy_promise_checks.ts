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
  Under strictNullChecks, checking whether a Promise is 'truthy' in a conditional will trigger an error.
 options:
  lib: es2015
 ---*/


async function foo(): Promise<boolean> {
    return false;
}

async function bar(): Promise<string> {
    if (!foo()) {
        return 'false';
    }
    return 'true';
}

var b = bar()
Assert.equal(b, '[object Promise]')