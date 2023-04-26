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
   Predefined conditional types
 ---*/


{
    // "b" | "d"
    type T00 = Exclude<"a" | "b" | "c" | "d", "a" | "c" | "f">;
    // "a" | "c"
    type T01 = Extract<"a" | "b" | "c" | "d", "a" | "c" | "f">;

    // string | number
    type T02 = Exclude<string | (() => void), Function>;
    // () => void
    type T03 = Extract<string | (() => void), Function>;

    function f1(s: string) {
        return { a: 1, b: s };
    }

    class C {
        x = 0;
        y = 0;
    }

    // { a: number, b: string }
    type T14 = ReturnType<typeof f1>;
    // C
    type T20 = InstanceType<typeof C>;

    let a: T00 = "b";
    let b: T00 = "d";
    let c: T01 = "a";
    let d: T01 = "c";
    let e: T02 = "c";
    let f: T03 = ((): string => { return 's' });
    let x: T14 = { a: 1, b: 's' }
    let y: T20 = { x: 1, y: 2 }

    Assert.equal(a, 'b');
    Assert.equal(b, 'd');
    Assert.equal(c, 'a');
    Assert.equal(d, 'c');
    Assert.equal(e, 'c');
    Assert.equal(typeof f, "function");
    Assert.equal(x.a, 1)
    Assert.equal(y.x, 1)
}
