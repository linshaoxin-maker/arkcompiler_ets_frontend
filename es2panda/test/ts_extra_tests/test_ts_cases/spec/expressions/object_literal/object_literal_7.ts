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
    If neither accessor includes a type annotation, 
    the inferred return type of the get accessor becomes the parameter type of the set accessor.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../suite/assert.js'

class h_C {
    private num: number;
    constructor(num: number) {
        this.num = num;
    }
    get() {
        return this.num;
    }
    set() {
        return this.num
    }
}
let h_c = new h_C(10);
Assert.isNumber(h_c.get())
Assert.isNumber(h_c.set());