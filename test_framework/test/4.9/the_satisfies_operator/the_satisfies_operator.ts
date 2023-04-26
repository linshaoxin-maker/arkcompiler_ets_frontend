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
    the new satisfies operator lets us validate that the type of an expression matches some type, without changing the resulting type of that expression.
 ---*/


type Colors = "Red" | "Green" | "Blue";
type RGB = [Red: number, Green: number, Blue: number];
const palette = {
    Red: [255, 0, 0],
    Green: "0x00ff00",
    Blue: [0, 0, 255],
} satisfies Record<Colors, string | RGB>;

const favoriteColors = {
    "Red": "yes",
    "Green": false,
    "Blue": "kinda",
} satisfies Record<Colors, unknown>;

const redComponent = palette.Red.length;
const greenNormalized = palette.Green.toUpperCase();
const g: boolean = favoriteColors.Green;

Assert.equal(redComponent, 3);
Assert.equal(greenNormalized, "0X00FF00");
Assert.isFalse(g);
