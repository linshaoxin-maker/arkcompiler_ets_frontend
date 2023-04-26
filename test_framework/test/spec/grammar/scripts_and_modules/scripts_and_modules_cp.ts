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


var pi: number = 3.14
function area(r: number) {
    return r * r * pi;
}
class Color {
    Red: number = 0;
    Green: number = 0;
    Bule: number = 0;
    constructor(r: number, g: number, b: number) {
        this.Red = r;
        this.Green = g;
        this.Bule = b;
    }
    toColorJSON() {
        let a: number[] = [this.Red, this.Green, this.Bule];
        return JSON.stringify(a);
    }
}
interface Weapon {
    Damage: number;
    DamageType: string;
}
type Skill = Weapon | { Damage: number, Data: string };
enum CommandE {
    end = -1,
    stop,
    run,
}
namespace AE {
    export interface PointXYZ {
        x: number;
        y: number;
        z: number;
    }
    export type StrNumBool = string | number | boolean;
}
export { pi, area, Color, Weapon, Skill, CommandE, AE };
