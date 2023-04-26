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
 description:Optional properties and methods can now be declared in classes, similar to what is already permitted in interfaces.
 ---*/


class Bar {
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
let bar = new Bar();
bar.a = 1024;
bar.b = 1408;
Assert.equal(bar.f(), 1024);

if (bar.h) {
    Assert.equal(bar.h(), 1408);
}

bar.g = () => {
    if (bar.b !== undefined) {
        return bar.a + bar.b;
    } else {
        return bar.a;
    }
}
Assert.equal(bar.g(), 2432);



