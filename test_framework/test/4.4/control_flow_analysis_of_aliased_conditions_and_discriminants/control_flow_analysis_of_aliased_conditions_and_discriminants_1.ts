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
description:> 
  When TypeScript sees that we are testing a constant value, it will do a little bit of extra work to see if it contains a type guard. 
  If that type guard operates on a const, a readonly property, or an un-modified parameter, 
  then TypeScript is able to narrow that value appropriately.
 ---*/

 
// const property
function foo(arg: unknown) {
    const argIsString = typeof arg === "string";
    if (argIsString) {
        Assert.equal(arg.toUpperCase(), "C");
    }
}
foo("c");
// readonly property
interface Test {
    readonly name: "caihua";
}
function test() {
    let tt: Test = { name: "caihua" };
    let res = tt.name === "caihua";
    if (res) {
        Assert.isNumber(tt.name.length);
        Assert.equal(tt.name[0], "c");
    }
}
test();
// un-modified parameter
function func(a: number, b = "function" as const) {
    let cc = typeof b === "function";
    if (cc) {
        Assert.isNumber(b.length);
        Assert.equal(b[0], "f");
        Assert.equal(cc, true);
    } else {
        Assert.equal(cc, false);
    }
}
func(12);
