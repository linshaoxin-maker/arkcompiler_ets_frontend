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
description: Type guards for dotted names also work with user defined type guard functions and the instanceof operators and do not depend on the strictNullChecks compiler option.
---*/


interface Base {
    name: string;
}

class A implements Base {
    a: string;
    name: string;
    constructor(name: string, a: string) {
        this.name = name;
        this.a = a;
    }
}

class B implements Base {
    b: number;
    name: string;
    constructor(name: string, b: number) {
        this.name = name;
        this.b = b;
    }
}

// user defined type guard functions
function doStuff(arg: A | B) {
    if (arg instanceof A) {
        Assert.equal(typeof arg.a, "string");
    } else {
        Assert.equal(typeof arg.b, "number");
    }
}

const a: A = new A("aa", "Ril");
const b: B = new B("bb", 100);
doStuff(a);
doStuff(b);