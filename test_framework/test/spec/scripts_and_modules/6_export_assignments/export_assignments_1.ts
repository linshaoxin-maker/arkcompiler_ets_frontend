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
   Export assignments exist for backward compatibility with earlier versions of TypeScript. An export assignment
   designates a module member as the entity to be exported in place of the module itself.
 module: ESNext
 isCurrent: true
 ---*/


import Pt from '../1_programs_and_source_files/source_9';
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

let point = new Pt(10, 20);
equal(point.x, 10);
equal(point.y, 20);

