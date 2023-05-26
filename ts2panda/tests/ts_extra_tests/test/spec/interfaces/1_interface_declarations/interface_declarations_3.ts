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
   Since function and constructor types are just object types containing call and construct signatures, interfaces can
   be used to declare named function and constructor types
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../suite/assert.js'

{
    interface StringC {
        (h_a: string, h_b: string): number;
    }

    let ps: StringC =
        (h_a: string, h_b: string): number => {
            return 1
        };

    class Class {
        h_x: number;
        h_y: number;

        constructor(h_a: number, h_b: number) {
            this.h_x = h_a;
            this.h_y = h_b;
        }
    }

    let pt: Class = new Class(0, 1);
    // assert declare named function
    Assert.equal(pt.h_x, 0);
    // assert constructor types
    Assert.equal(pt.h_y, 1);
};
