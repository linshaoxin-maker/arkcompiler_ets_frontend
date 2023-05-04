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
   When a generic interface has multiple declarations, all declarations must have identical type parameter lists,
   i.e. identical type parameter names with identical constraints in identical order.
 ---*/


{
    interface Documents<T> {
        createElement1(a: T): number;
    }

    interface Documents<T> {
        createElement2(a: T): number;

        createElement3(a: T): number;
    }

    let name: Documents<number | string | boolean> = {
        createElement1(a: string): number {
            return 0
        },
        createElement2(a: boolean): number {
            return 0
        },
        createElement3(a: number): number {
            return 0
        }
    };

    class Class {
        static getName(name: Documents<number | string | boolean>): Documents<number | string | boolean> {
            return name;
        }
    }

    Assert.equal(Class.getName(name).createElement1(''), 0);
    Assert.equal(Class.getName(name).createElement3(0), 0);
    Assert.equal(Class.getName(name).createElement2(true), 0);
}