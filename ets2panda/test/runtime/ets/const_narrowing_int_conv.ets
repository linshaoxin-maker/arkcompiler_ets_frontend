/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

function foo(a: byte) : int {
    return 10;
}

function foo(a: short) : int {
    return 12;
}

function foo(a: byte[]) : int {
    return 14;
}

function foo(a: short[]) : int {
    return 16;
}

function main(): void {
    let b: byte = 127;
    let c: char = 0x42E;
    let d: byte = 0x007f as short;
    let e: byte = (100 as int) as int;

    assert(b == 127);
    assert(c == 0x42E);
    assert(d == 0x007f);
    assert(e == 100);

    assert(foo(2 as byte) == 10);
    assert(foo(2 as short) == 12);

    assert(foo([2 as byte, 1]) == 14);
    assert(foo([2 as short, 1]) == 16);
}
