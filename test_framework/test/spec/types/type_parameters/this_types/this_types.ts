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
     every class and interface has a this-type that represents the actual type of instances of the class or interface within the declaration of the class or interface. The this-type is referenced using the keyword this in a type position. 
     within instance methods and constructors of a class, the type of the expression this  is the this-type of the class.
 ---*/


class CA {
  name: string = "CA";
  foo() {
    return this;
  }
  caName(cname: string) {
    this.name = cname;
    return "CA:" + this.name;
  }
}

class CB extends CA {
  bar() {
    return this;
  }
  cbName(cname: string) {
    this.name = cname;
    return "CB:" + this.name;
  }
}

let bB: CB = new CB();
let xB = bB.foo().bar();
Assert.equal(bB, xB);

var thisStrB: string = bB.cbName("CB");
Assert.equal(thisStrB, "CB:CB");
var thisStrA: string = bB.caName("CA");
Assert.equal(thisStrA, "CA:CA");
