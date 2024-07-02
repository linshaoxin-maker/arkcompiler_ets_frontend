/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

type Matrix = BigInt[][]

interface I {
    type Matrix = boolean[] | String[]
}

class A {
   type Matrix = String[][] | number[][]
   private m_: Matrix;
   constructor() {
        this.m_ = [["first"], ["second", "third"]]
   }

   public setM(val:Matrix):void{
       this.m_ = val;
   }

   public getM() : String {
        return new String(this.m_)
   }

   public foo(): String {
        type Matrix = boolean[][]
        let b : Matrix = [[true, false], [false, true]]
        return new String(b)
   }
}

function foo() : String {
    type Matrix = String[][]
    let  m : Matrix = [["a"], ["b", "c"]]
    return new String("[" + m[0] + "], [" + m[1][0] + ","+ m[1][1]+"]")
}

function main() {
    let  m : Matrix = [[new BigInt(1)], [new BigInt(2)]];
    assert (new String(m) == "[object Object],[object Object]")
    assert (foo() == "[\"a\"], [b,c]")

    let a = new A();
    assert(a.foo() == "true,false,false,true")
    assert(a.getM() == "\"first\",\"second\",\"third\"")
    a.setM([[99.0],[100.0, 101.0]]);
    assert(a.getM() == "99,100,101")
}
