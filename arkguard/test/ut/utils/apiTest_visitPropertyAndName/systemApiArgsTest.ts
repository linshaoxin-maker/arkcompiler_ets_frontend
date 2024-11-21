/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
export class TestClass1 {
    prop1: number;
    constructor (public param1: number, param2: number) {
        this.param1 = param1;
        this.prop1 = param2;
    }
    foo1(param5: number) {}
}

class TestClass2 {
    prop2: number;
    constructor (public param3: number, param4: number) {
        this.param3 = param3;
        this.prop2 = param4;
    }
    foo2(param6: number) {}
}