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
  TypeScript allows us to mark a class as abstract. This tells TypeScript that the class is only meant to be extended from, and that certain members need to be filled in by any subclass to actually create an instance.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

abstract class Color {
    abstract red: number;
    abstract green: number;
    abstract blue: number;
    protected abstract rgb: [number, number, number];

    abstract getRGB(): [number, number, number];
}

class ColorCopy extends Color {
    red: number = 0;
    green: number = 0;
    blue: number = 0;
    protected rgb: [number, number, number] = [0, 0, 0];
    getRGB(): [number, number, number] {
        this.rgb = [this.red, this.green, this.blue]
        return this.rgb;
    }
    constructor(r: number, g: number, b: number) {
        super();
        this.red = r;
        this.green = g;
        this.blue = b;
        this.getRGB();
    }
}

let color: ColorCopy = new ColorCopy(255, 0, 0);
Assert.equal(JSON.stringify(color.getRGB()), '[255,0,0]');

abstract class Point {
    abstract a: number;
    abstract b: number;
    abstract getPoint(): [number, number];
}

type MIX<T> = abstract new (...args: any[]) => T

function mixClass<T extends MIX<object>>(Ctor: T) {
    abstract class CPColor extends Ctor {
        abstract red: number;
        abstract green: number;
        abstract blue: number;
        abstract getColor(): [number, number, number];
    }
    return CPColor;
}

class ColorPoint extends mixClass(Point) {
    red: number = 0;
    green: number = 0;
    blue: number = 0;
    a: number = 0;
    b: number = 0;
    getColor(): [number, number, number] {
        return [this.red, this.green, this.blue];
    }
    getPoint(): [number, number] {
        return [this.a, this.b];
    }
    constructor(red: number = 0, green: number = 0, blue: number = 0, a: number = 0, b: number = 0) {
        super();
        this.a = a; this.b = b;
        this.red = red; this.green = green; this.blue = blue;
    }
}

let cp = new ColorPoint(0, 255, 0, 25, 25);
Assert.equal(JSON.stringify(cp.getColor()), '[0,255,0]');
Assert.equal(JSON.stringify(cp.getPoint()), '[25,25]');