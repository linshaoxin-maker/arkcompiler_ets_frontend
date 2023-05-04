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
   multiple declarations is equivalent to the following single declaration.
 ---*/


{
    interface Document1 {
        createElement1(a: string): number;
    }

    interface Document1 {
        createElement2(a: boolean): number;

        createElement3(a: number): number;
    }

    interface DocumentAssert {
        createElement4(a: string): number;

        createElement5(a: boolean): number;

        createElement6(a: number): number;
    }

    let name: Document1 & DocumentAssert = {
        createElement1(a: string): number {
            return 0
        },
        createElement2(a: boolean): number {
            return 0
        },
        createElement3(a: number): number {
            return 0
        },
        createElement4(a: string): number {
            return 0
        },
        createElement5(a: boolean): number {
            return 0
        },
        createElement6(a: number): number {
            return 0
        }
    };

    class Class {
        static getName(name: Document1 & DocumentAssert): Document1 & DocumentAssert {
            return name;
        }
    }

    Assert.equal(Class.getName(name).createElement1(''), 0);
    Assert.equal(Class.getName(name).createElement3(0), 0);
    Assert.equal(Class.getName(name).createElement2(true), 0);
    Assert.equal(Class.getName(name).createElement4(''), 0);
    Assert.equal(Class.getName(name).createElement5(true), 0);
    Assert.equal(Class.getName(name).createElement6(0), 0);
}