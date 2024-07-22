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
 let a : Number | Int = 5

function foo() : Number | Int {
    return 5
}

class A {
    foo : Number | Int

    func() : Number | Int {
        return this.foo;
    }
}

function foo1(a : Number | Int) : Number | Int {
    return 5;
}
let ca : A  = {foo : 5}

let b : number = foo()
let c : number = a
let d : number = ca.foo
let f : number = ca.func()
let g : number = foo1(a)
let h : number = foo1(ca.foo)

assert b == 5
assert c == 5
assert d == 5
assert f == 5
assert g == 5
assert h == 5