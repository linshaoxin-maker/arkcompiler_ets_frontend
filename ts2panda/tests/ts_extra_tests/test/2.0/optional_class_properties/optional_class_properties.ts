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
 description: Optional properties and methods can now be declared in classes, similar to what is already permitted in interfaces.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

class Class0000 {
    a: number = 0;
    b?: number;
    f() {
        return this.a;
    }
    // Body of optional method can be omitted
    g?(): number;
    h?() {
        return this.b;
    }
}
let c0000 = new Class0000();
c0000.a = 1024;
c0000.b = 1408;
Assert.equal(c0000.f(), 1024);

if (c0000.h) {
    Assert.equal(c0000.h(), 1408);
}

c0000.g = () => {
    if (c0000.b !== undefined) {
        return c0000.a + c0000.b;
    } else {
        return c0000.a;
    }
}
Assert.equal(c0000.g(), 2432);
