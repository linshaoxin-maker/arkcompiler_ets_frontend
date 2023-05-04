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
 ---*/


{
    let arr: ReadonlyArray<string> = ['1', '2'];
    let sa: readonly string[] = ['1', '2'];

    Assert.equal(arr[1], '2')
    Assert.equal(sa[1], '2')

    function getShapes() {
        let result = [
            { kind: "circle", radius: 100 },
            { kind: "square", sideLength: 50 },
        ] as const;
        return result;
    }
    for (const shape of getShapes()) {
        // Narrows perfectly!
        if (shape.kind === "circle") {
            console.log("Circle radius", shape.radius);
            Assert.equal(shape.radius, 100)
        } else {
            console.log("Square side length", shape.sideLength);
            Assert.equal(shape.sideLength, 50)
        }
    }
}
