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
    The Any type is a supertype of all types, 
    and is assignable to and from all types.
 options: 
    lib: es2015
 ---*/


var x: any;
x = 12;
Assert.isNumber(x);
x = "abc";
Assert.isString(x);
x = true;
Assert.isBoolean(x);
x = Symbol();
Assert.isSymbol(x);
x = null;
let flag = false;
if (x === null) {
  flag = true
}
Assert.isTrue(flag);
x = { 0x00: "0x00" };
Assert.equal(JSON.stringify(x), '{"0":"0x00"}')
x = function voidFunction() { };
Assert.isFunction(x);
x = undefined;
Assert.isUndefined(x);

var y: any;
var n: number = 1;
n = y;
var s: string = "string";
s = y;
var b1: boolean = false;
b1 = y;
var sym: symbol = Symbol();
sym = y;
var v: void;
v = y;
var obj: object = { 0x11: "0x11" };
obj = y;
var fun1: Function = function add(a: number, b: number, c: number) {
  return a + b + c;
};
fun1 = y;
var u: undefined;
u = y;
