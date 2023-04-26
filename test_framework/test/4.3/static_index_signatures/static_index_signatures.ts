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
  Index signatures allow us set more properties on a value than a type explicitly declares,
  And index signatures can now be declared as static.
 ---*/


class Foo {
    hello = "hello";
    world = "1213";
    [propName: string]: string | number | undefined
}

let instance = new Foo()
instance['whatever'] = 42
Assert.equal(instance['whatever'], 42)

// Has type 'string | number | undefined'
let x = instance['something']
x = undefined
Assert.isUndefined(x)
x = 20
Assert.equal(x, 20)
x = 'x'
Assert.equal(x, 'x')

class Bar {
    static hello = 'hello';
    static world = 1342;
    static [propName: string]: string | number | undefined
}

Bar['whatever'] = 42
Assert.equal(Bar['whatever'], 42)

// Has type 'string | number | undefined'
let y = Bar['something']
y = 'y'
Assert.equal(y, 'y')
y = 10
Assert.equal(y, 10)
y = undefined
Assert.isUndefined(y)