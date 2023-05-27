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
   Improvements for ReadonlyArray and readonly tuples.
   const assertions.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../suite/assert.js"


    let hw_arr: ReadonlyArray<string> = ["1", "2"];
    let hw_rsa: readonly string[] = ["1", "2"];

    Assert.equal(hw_arr[1], "2");
    Assert.equal(hw_rsa[1], "2");

    function hwfun() {
        let result = [
            { kind: "circle", radius: 100 },
            { kind: "square", sideLength: 50 },
        ] as const;
        return result;
    }
    for (const shape of hwfun()) {
        if (shape.kind === "circle") {
            Assert.equal(shape.radius, 100);
        } else {
            Assert.equal(shape.sideLength, 50);
        }
    }

