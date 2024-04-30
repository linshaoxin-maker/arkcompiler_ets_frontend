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

import assert from 'assert';

export function foo11(para1: number, para2: number) {
  return para1 + para2;
}
assert.strictEqual(foo11(1, 2), 3);

function foo12(para3: number, para4: number){
  return para3 - para4;
}
assert.strictEqual(foo12(3, 1), 2);

namespace ns13 {
  function foo14(para3: string, para4: string): string {
    return para3 + para4;
  }
  assert.strictEqual(foo14("hello", "world"), "helloworld");
}