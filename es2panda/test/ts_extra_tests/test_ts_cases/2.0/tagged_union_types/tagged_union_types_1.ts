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
description: The TS compiler now support type guards that narrow union types based on tests of a discriminant property and furthermore extend that capability to switch statements.
module: ESNext
isCurrent: true
---*/


import { Assert } from "../../../suite/assert.js"

interface Color {
    name: "Color";
    rgb: [number, number, number];
}

interface Point {
    name: "Point";
    point: [number, number];
}

interface Level {
    name: "Level";
    level: number;
}

type ColorPoint = Color | Point | Level;

function test(s: ColorPoint) {
    switch (s.name) {
        case "Color":
            return s.rgb;
        case "Point":
            return s.point;
        case "Level":
            return s.level;
    }
}

function test1(s: ColorPoint) {
    if (s.name === "Color") {
        return s;
    }
    return s;
}

function test2(s: ColorPoint) {
    if (s.name === "Color" || s.name === "Point") {
        return;
    }
    return s;
}

let color: Color = {
    name: "Color",
    rgb: [255, 0, 0]
};

let point: Point = {
    name: "Point",
    point: [0, 0]
};

let level: Level = {
    name: "Level",
    level: 10
};

Assert.equal(JSON.stringify(test(color)), '[255,0,0]');
Assert.equal(test1(point).name, 'Point');
Assert.equal(test2(level)?.level, 10);