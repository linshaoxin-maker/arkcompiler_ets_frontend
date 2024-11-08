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

class ClassA {
    constructor(name: string);
    constructor(name: number, id: number);
    constructor(name?: string | number, id?: number) {
        if (typeof name === 'number') {
            print('name number :' + name);
            print('id number :' + id);
        } else {
            print('name string :' + name);
        }
    }

    static ClassA() {
        return 3;
    }

    add() {
        return 1;
    }
}

let obj = new ClassA('test');
print(obj.add());
print(ClassA.ClassA());