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
   In prior versions, TypeScript only allowed ...rest elements at the very last position of a tuple type.However, now rest elements can occur anywhere within a tuple - with only a few restrictions.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

let f: [...string[], number];

f = [123];
f = ['hello', 123];
f = ['hello!', 'hello!', 'hello!', 123];

let g: [boolean, ...string[], boolean];

g = [true, false];
g = [true, 'some text', false];
g = [true, 'some', 'separated', 'text', false];

Assert.equal(JSON.stringify(f), "[\"hello!\",\"hello!\",\"hello!\",123]");
Assert.equal(JSON.stringify(g), "[true,\"some\",\"separated\",\"text\",false]");

