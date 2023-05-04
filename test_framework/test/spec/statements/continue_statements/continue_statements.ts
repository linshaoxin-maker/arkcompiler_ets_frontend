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
description: A 'continue' statement is required to be nested, directly or indirectly (but not crossing function boundaries), within an iteration ('do', 'while', 'for', or 'for-in') statement.
---*/


let num: number = 0;
let count: number = 0;
do {
    num++;
    if (num % 2 == 0) {
        continue;
    }
    count++;
} while (num < 10);
Assert.equal(5, count);


num = 0;
count = 0;
for (num = 0; num <= 20; num++) {
    if (num % 2 == 0) {
        continue;
    }
    count++;
}
Assert.equal(10, count);

count = 0;
let arr = [0, 1, 2, 3, 4, 5];
for (let index in arr) {
    if (arr[index] % 2 == 0) {
        continue;
    }
    count++;
}
Assert.equal(3, count);