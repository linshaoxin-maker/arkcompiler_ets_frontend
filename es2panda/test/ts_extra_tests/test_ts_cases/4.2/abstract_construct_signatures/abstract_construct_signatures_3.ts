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
  The feature allows us to write mixin factories in a way that supports abstract classes. 
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

abstract class myClass {
    abstract h_method(): number;
    func() { };
}

type myType<T> = abstract new (...args: any[]) => T;

function myFunc<T extends myType<object>>(value: T) {
    abstract class myC extends value {
        getS() {
        };
    }
    return myC;
}

class h_C extends myFunc(myClass) {
    h_method(): number {
        return 10;
    }
}

let test = new h_C();
Assert.equal(test.h_method(), 10);