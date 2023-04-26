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
  The type inferred for a let variable, var variable, parameter, or non-readonly property with an initializer 
  and no type annotation is the widened literal type of the initializer.   
  where the widened type for a string literal type is string, number for numeric literal types, boolean for true or false 
  and the containing enum for enum literal types.
 ---*/


// let number
let a = 1;
let aa: typeof a = 12;
Assert.isNumber(aa);
// var number
var b = 1;
var bb: typeof b = 12;
Assert.isNumber(bb);
// parameter boolean
function test(a = true) {
    return a;
}
let tt: ReturnType<typeof test> = false;
Assert.isBoolean(tt);
// non-readonly property
class Test {
    static job = "coder";
}
// string
let cc: typeof Test.job = "driver";
Assert.isString(cc);