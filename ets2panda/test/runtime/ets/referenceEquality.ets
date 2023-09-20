/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

class A{}

function main(): void {
  ClassEquality();
  NullEquality();
  ArrayEquality();
}

function ClassEquality(): void {
    let a : A = new A();
    let b = a;
    assert (a === b);
    let c : A = new A();
    assert (a !== c);

    assert(new A() != new A())
    let x1 = new A()
    let x2 = x1
    assert(x1 == x2)

    assert(5 == new Int(5))
}


function NullEquality(): void {
    let a : A | null = null;
    let b = a;
    assert (a === b);
    let c : A | null = null;
    assert (c === null);
    assert (null === null);

    assert(5 != null)
    assert("a string" != null)
    assert(5.0 != null)

    let x: A | null = null
    assert(x == null)

    assert(null != new Int(5))
    assert(new Double(5) != null)

    let i: Int|null = null
    assert(i == null)

    let f: Int|null = 1
    assert(f != null)
}


function ArrayEquality(): void {
    let a : int[] = [1, 2, 3];
    let b = a;
    assert (a === b);
    let c : int[] = [1, 2, 3];
    assert (a !== c);

    assert([1, 2, 3] != [1, 2, 3])
    let y1 : Object[] = []
    let y2 = y1
    assert(y1 == y2)
}

function NumericEquality(): void {
    let a : Int = 1;
    let b = a;
    assert (a === b);
    let c : Int = 1;
    assert (a !== c);
    let d : Double = 1.0;
    assert (a !== c);
}
