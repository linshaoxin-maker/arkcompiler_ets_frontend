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
  The type inferred for an object literal is considered a fresh object literal type. 
  The freshness disappears when an object literal type is widened or is the type of the expression in a type assertion.
 ---*/


// { x: 0, y: 0 } is the fresh object literal type
const p: { x: number, y: number } = { x: 0, y: 0 }
Assert.equal(JSON.stringify(p), '{"x":0,"y":0}')

// The freshness disappears when the type of the express in a type assertion
// There is Excess Property y but no error
const p1: { x: number } = { x: 0, y: 0 } as { x: 0, y: 0 }
Assert.equal(JSON.stringify(p1), '{"x":0,"y":0}')