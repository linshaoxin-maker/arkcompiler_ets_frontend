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
  Using ‘rootDirs’, you can inform the compiler of the roots making up this “virtual” directory;
  and thus the compiler can resolve relative modules imports within these “virtual” directories 
  as if they were merged together in one directory.
  The tsc command needs to be executed in the 2.0 file directory.
  tsconfig.json file url: testing-framework\test\2.0\tsconfig.json.
module: ESNext
isCurrent: true
---*/


import './view2';
import { Directions } from '../test_virtual_directories_with_rootDirs/view2';

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
equal(Directions.Left, 2);