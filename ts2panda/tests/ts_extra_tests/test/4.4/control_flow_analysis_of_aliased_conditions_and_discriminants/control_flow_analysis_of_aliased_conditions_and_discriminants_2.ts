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
 description: Different sorts of type guard conditions are preserved. For example, checks on discriminated unions
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

type T1 = { kind: "circle"; radius: number };
type T2 = { kind: "square"; sideLength: number };
type T3 = T1 | T2;

let c: T1 = { kind: "circle", radius: 2 };
let s: T2 = { kind: "square", sideLength: 2 };

function hwtest01(shape: T3): number {
    const isCircle = shape.kind === "circle";
    if (isCircle) {
        return Math.PI * shape.radius ** 2;
    } else {
        return shape.sideLength ** 2;
    }
}
Assert.equal(Math.round(hwtest01(c)), 13);
Assert.equal(hwtest01(s), 4);

function hwtest02(shape: T3): number {
    const { kind } = shape;
    if (kind === "circle") {
        return Math.PI * shape.radius ** 2;
    } else {
        return shape.sideLength ** 2;
    }
}
Assert.equal(Math.round(hwtest02(c)), 13);
Assert.equal(hwtest02(s), 4);
