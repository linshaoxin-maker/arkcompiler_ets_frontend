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
 description: TypeScript 2.9 allows passing generic type arguments to tagged template strings.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../suite/assert.js'

let jianbing = '煎饼'

function kitchen(strings: TemplateStringsArray, value: string) {
  Assert.equal(2, strings.length);
}

let zaocan = kitchen`今天的早餐是${jianbing}!`;



function tag<T>(strs: TemplateStringsArray, args: T): T {
  Assert.equal(2, strs.length);
  return args;
};


let a = tag<string> `今天的午餐是${jianbing}!`;
Assert.equal("煎饼", a);

let b = tag<string> `今天的午餐是${"dessert"}!`;

Assert.equal("dessert", b);
