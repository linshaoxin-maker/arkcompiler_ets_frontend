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
    typescript 4.9 makes the in operator a little bit more powerful when narrowing types that don’t list the property at all.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

interface RGBIN {
    red: number;
    green: number;
    blue: number;
}

interface HSV {
    hue: number;
    saturation: number;
    value: number;
}

function setColor(color: RGBIN | HSV) {
    if ("hue" in color && "red" in color) {
        return "RGBIN | HSV";
    } else if ("hue" in color) {
        return "HSV";
    } else if ("red" in color) {
        return "RGBIN";
    }
}
var c1: RGBIN = { red: 0, green: 0, blue: 0 };
var c2: HSV = { hue: 0, saturation: 0, value: 0 };
var c3: RGBIN | HSV = { hue: 0, saturation: 0, value: 0, red: 0, green: 0, blue: 0 };

Assert.equal(setColor(c1), "RGBIN");
Assert.equal(setColor(c2), "HSV");
Assert.equal(setColor(c3), "RGBIN | HSV");

interface Package {
    JSON: unknown;
}

function getPackageName(pak: Package): string {
    const p:any = pak.JSON;
    if (p && typeof p === "object") {
        if ("name" in p && typeof p.name === "string") {
            return p.name;
        }
    }
    return "no JSON";
}
let pak: Package = { JSON: { name: "JSON" } };
let none: Package = { JSON: "null" };
let out1 = getPackageName(pak);
let out2 = getPackageName(none);

Assert.equal(out1, "JSON");
Assert.equal(out2, "no JSON");
