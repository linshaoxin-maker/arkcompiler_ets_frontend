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
  A class may optionally have type parameters. A class with type parameters is called a generic class. 
  The type parameters of a generic class declaration are in scope in the entire declaration and may be referenced in the ClassHeritage and ClassBody.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

class MyClass<T>{
    myArry: T[] = [];
    add(num: T): void {
        this.myArry.push(num);
    }
    max(): T {
        var mNum = this.myArry[0];
        for (var i = 0; i < this.myArry.length; i++) {
            if (mNum < this.myArry[i]) {
                mNum = this.myArry[i];
            }
        }
        return mNum;
    }
}
var h_m = new MyClass<number>();
h_m.add(5);
h_m.add(4);
h_m.add(9);
h_m.add(15);
Assert.equal(h_m.max(), 15);