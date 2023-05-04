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
description: The scope of a local var name declared anywhere in the body of a function-like declaration is the body of that function-like declaration.
 ---*/

 
var GlobalScope = 1;

function someFunc() {
    var FunctionScope = 2;
    if (true) {
        var BlockScope = 3;
    }
    Assert.equal(FunctionScope, 2);
    Assert.equal(BlockScope, 3);
}
Assert.equal(GlobalScope, 1);
someFunc();