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
description:Different sorts of type guard conditions are preserved. For example, checks on discriminated unions
 ---*/

 
type Circle = { kind: "circle"; radius: number };
type Square = { kind: "square"; sideLength: number };
type Shape = Circle | Square;

let circle: Circle = { kind: "circle", radius: 2 };
let square: Square = { kind: "square", sideLength: 2 };

function areaV1(shape: Shape): number {
    const isCircle = shape.kind === "circle";
    if (isCircle) {
        return Math.PI * shape.radius ** 2;
    } else {
        return shape.sideLength ** 2;
    }
}
Assert.equal(Math.round(areaV1(circle)), 13);
Assert.equal(areaV1(square), 4);

function areaV2(shape: Shape): number {
    const { kind } = shape;
    if (kind === "circle") {
        return Math.PI * shape.radius ** 2;
    } else {
        return shape.sideLength ** 2;
    }
}
Assert.equal(Math.round(areaV2(circle)), 13);
Assert.equal(areaV2(square), 4);
