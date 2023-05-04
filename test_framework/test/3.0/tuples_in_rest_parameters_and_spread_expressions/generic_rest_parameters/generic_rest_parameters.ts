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
    a rest parameter is permitted to have a generic type that is constrained to an array type, and type inference can infer tuple types for such generic rest parameters.
 ---*/


function bind<T, U extends any[], V>(f: (x: T, ...args: U) => V): (...args: U) => V {
    function frm(...args: U) { let json = JSON.stringify(args); return json as unknown as V; }
    return frm;
};
function gf(x: number, y: string, z: boolean): string {
    let a: any[] = [x, y, z];
    let json = JSON.stringify(a);
    return json;
}
let a = gf(5, "A", true);
let gf1 = bind(gf);
let b = gf1("B", true);
let gf2 = bind(gf1);
let c = gf2(true);
let gf3 = bind(gf2);
let d = gf3();
Assert.equal(a, "[5,\"A\",true]");
Assert.equal(b, "[\"B\",true]");
Assert.equal(c, "[true]");
Assert.equal(d, "[]");
