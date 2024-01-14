/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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


class X { x: number = 0 }
class Y { y: number = 0 }

class C1<T0, T1> extends X {
    c1: number = 0;
    t0: T0 | null = null;
    t1: T1 | null = null;
}

function fn<T0, T1 extends C1<X, T1>>() {}

function test1<A0, A1 extends C1<Y, A1>>() {
    fn<A0, A1>();
}
