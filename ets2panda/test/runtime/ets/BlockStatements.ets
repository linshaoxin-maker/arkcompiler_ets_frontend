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

let a = 0;

function test1(): int {
    return 1;
}

function test2(arg: int): int {
    return 0;
}

function test3(): int {
    a += 1
    return a;
}

function test_if() {
    if (test1() == 1)
    let x = 10; //if body
    let x = 11;
    return test2(x);
}
function test_else() {
    if (test1() != 1) return 1
    else 
    let x = 10; // else body
    let x = 11;
    return test2(x);
}
function test_while() {
    while (test3() != 3)
    let x = 10; //while body
    let x = 11;
    return test2(x);
}
function test_do() {
    do
    let x = 10; //do body
    while (test3() != 5)
    let x = 11;
    return test2(x);
}
function test_for() {
    for (let i = 0; i < test1(); i++)
    let x = 10; //for body
    let x = 11;
    return test2(x);
}

function main(): int {
    return test_if() + test_else() + test_while() + test_do() + test_for();
}