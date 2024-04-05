/*
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

class MyClass {
    public constructor(elements: Iterable<Number>) {
        let it = elements.iterator();
        while (true) {
            const v = it.next();
            if (v.done) {
                break;
            }
        }
    }
}
function main(): void  {
    let a = new Array<Number>;
    a.push(11);
    let mmm = new MyClass(a);
}