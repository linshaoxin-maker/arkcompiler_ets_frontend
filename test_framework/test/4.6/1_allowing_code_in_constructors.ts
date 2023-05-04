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
 description: TypeScript 4.6 is now much more lenient in that check and permits other code to run before super()., all while still ensuring that super() occurs at the top-level before any references to this.
---*/


class Base { }

class Derived extends Base {
    someProperty = true;

    constructor() {
        let ret = doSomeStuff();
        super();
        this.someProperty = false;
    }

    getSomeProperty(): boolean {
        return this.someProperty;
    }


}

function doSomeStuff(): void { }

let derived = new Derived();

Assert.equal(false, derived.getSomeProperty());

