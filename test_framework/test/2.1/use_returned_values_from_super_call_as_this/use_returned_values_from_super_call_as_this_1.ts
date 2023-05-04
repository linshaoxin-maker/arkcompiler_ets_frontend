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
  In ES2015, constructors which return an object implicitly substitute the value of this for any callers of super(). 
  it is necessary to capture any potential return value of super() and replace it with this.
 ---*/

 
class Base {
    x: number;
    constructor(x: number) {
        this.x = x;
        return {
            x: 1,
        };
    }
}
class Derived extends Base {
    constructor(x: number) {
        super(x);
    }
}
let cc = new Derived(12);
Assert.equal(cc.x, 1);
Assert.notEqual(cc.x, 12);
