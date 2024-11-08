/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// export class
export class ClassA {
    static sFieldA0 = 0;
    fieldA1 = 1;

    constructor() {
        this.fieldA2 = 2;
    }

    methodA1() {
        return ClassA.sFieldA0 + this.fieldA1 + this.fieldA2;
    }

    get value1() {
        return this.fieldA1;
    }
}

class ClassB {
    static sFieldB0 = 4;
    filedB1 = 5;

    constructor() {
        this.filedB2 = 6;
    }

    methodB1() {
        class ClassC {
            static sFieldC0 = 7;
            fieldC1 = 8;

            constructor() {
                this.fieldC2 = 9;
            }

            methodC1() {
                return ClassC.sFieldC0 + this.fieldC1 + this.fieldC2;
            }
        }

        return ClassB.sFieldB0 + this.filedB1 + this.filedB2 + new ClassC().methodC1();
    }
}

let a = new ClassA();
let b = new ClassB();

print('methodA1 = ' + a.methodA1());
print('methodB1 = ' + b.methodB1());