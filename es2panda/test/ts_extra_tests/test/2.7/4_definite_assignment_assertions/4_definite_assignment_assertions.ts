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
 description: The definite assignment assertion is a feature that allows a ! to be placed after instance property and variable declarations to relay to TypeScript that a variable is indeed assigned for all intents and purposes, even if TypeScript's analyses cannot detect so.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../../suite/assert.js'

// With definite assignment assertions, we can assert that x is really assigned by appending an ! to its declaration
let x!: number;

// In a sense, the definite assignment assertion operator is the dual of the non-null assertion operator (in which expressions are post-fixed with a !)
let y = x! + x!;

Assert.isTrue(Number.isNaN(y));
initialize();

// No error!
let z = x + x;
function initialize() {
  x = 10;
}

Assert.equal(20, z);