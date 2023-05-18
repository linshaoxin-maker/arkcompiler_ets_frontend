/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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


// Test basic types
var a: double;
var b: char;
var c: byte;
var d: short;
var e: int;
var f: long;
var g: ubyte;
var h: ushort;
var i: uint;
var j: ulong;
var k: float;

// Test array type
var l: char[];
var m: string[][];
var n: unknown[] | 5n[];

// Test union type
var o: (byte | string)[] | true | ((undefined[] | unknown)[] | "foo")[];
var p: (short | string)[] | (int | string)[];
var q: ((long | ubyte)[] | (ushort | uint)[]) | string;

// Test parenthesized type
var r: ((((ulong))))[];
var s: ((string | boolean) | (5 | true)) | (void);

// Test function type
var func1: (a: int, b: string) => double;
var func2: (a: char[] | string, b?: 5) => ubyte | string;
var func3: (f: (a: float, b: string) => double[], [a, b]: char[]) => (a: byte, b: boolean) => true;

// Test constructor type
var ctor1: new (a: short, b: string) => int;
var ctor2: new (a: long[] | string, b?: 5) => ubyte | string;
var ctor3: new (f: (a: ushort, b: string) => uint[], [a, b]: ulong[]) => (a: float, b: boolean) => true;
var ctor4: abstract new (a: double, b: string) => char;
var ctor5: abstract new (a: byte[] | string, b?: 5) => short | string;
var ctor6: abstract new (f: (a: int, b: string) => long[], [a, b]: ubyte[]) => (a: ushort, b: boolean) => true;
