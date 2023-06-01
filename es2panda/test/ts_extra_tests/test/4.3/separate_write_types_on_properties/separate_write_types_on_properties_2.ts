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
   Be able to write getters and setters with different types in object literals.
 options:
   lib: es2015
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../../suite/assert.js"

interface HWC {
    get size(): number;
    set size(value: number | string | boolean);
}

function hwtest(): HWC {
    let size = 0;
    return {
        get size(): number {
            return size;
        },
        set size(value: string | number | boolean) {
            let num = Number(value);

            if (!Number.isFinite(num)) {
                size = 0;
                return;
            }
            size = num;
        },
    };
}

let t = hwtest();
Assert.equal(t.size, 0);
