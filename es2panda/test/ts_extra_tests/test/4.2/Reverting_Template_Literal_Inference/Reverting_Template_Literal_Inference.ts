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
    template string literals would either be given template string types or simplify to multiple string literal types.
    These types would then widen to string when assigning to mutable variables.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

const logo:string ="just do it"

const hw: string = `go ${logo}`;

let hwa: string = `hello ${logo}`;

let hws:string = `luckily dog ${logo}` as const;

function check(op?:string){
    if (op){
        Assert.isString(op)
    }
}
check(hw+hwa+hws);
Assert.equal(hw,"go just do it");
Assert.equal(hwa,"hello just do it");