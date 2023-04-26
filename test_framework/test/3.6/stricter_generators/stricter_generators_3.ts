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
description: the new Generator type is an Iterator that always has both the return and throw methods present, and is also iterable.
options:
  lib:es2017
 ---*/


function* test(): Generator<number, string, boolean> {
    let i = 0;
    while (true) {
        if (yield i++) {
            break;
        }
    }
    return "done!";
}

let cc = test();
let ccItem = cc.next();
while (!ccItem.done) {
    Assert.equal(typeof ccItem.value, "number");
    ccItem = cc.next(typeof ccItem.value == "number");
}
