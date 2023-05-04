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
  A type guard of the form typeof x === s, where s is a string literal with the value 'string', 'number', or 'boolean',
  when true, narrows the type of x to the given primitive type provided it is a subtype of the type of x, 
  or, if the type of x is a union type, removes from the type of x all constituent types that aren't subtypes of the given primitive type, 
  or when false, removes the primitive type from the type of x.
 ---*/


function f(x: string | number) {
    if (typeof x === "string") {
        Assert.isString(x)
    }
    else {
        Assert.isNumber(x)
    }
}
f(10)
f('s')