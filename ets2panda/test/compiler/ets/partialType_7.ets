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

class A<T> {
    mmeb: Number = 2;
    foo(a0: Partial<T>){
        this.bar(a0);
    }
    bar(a0: Partial<T>){
        this.foo(a0);
    }
}

class B { fld: Number = 6;}

class C <T extends B> {
    baz(a0: Partial<T>){
    }
}

function foo(): void {
    let part_B: Partial<B> = {fld: 8};
    let a_class: A<B> = new A<B>();
    a_class.foo(part_B);
}
