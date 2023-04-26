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
   imports a given module and creates local bindings for a specified list of exported members of the module. The specified
   names must each reference an entity in the export member set (11.3.4.4) of the given module. The local bindings have t
   he same names and classifications as the entities they represent unless as clauses are used to that specify different
   local names.
 module: ESNext
 isCurrent: true
 ---*/


import { x as a, y as b, z as c} from '../1_programs_and_source_files/source_2.js';
class AssertionError extends Error {
    constructor(public msg :string) {
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


equal(a,0);
equal(b,1);
equal(c,2);
