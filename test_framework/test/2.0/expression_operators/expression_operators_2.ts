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
 description: The && operator adds null and/or undefined to the type of the right operand depending on which are present in the type of the left operand, and the || operator removes both null and undefined from the type of the left operand in the resulting union type.
 ---*/


interface Entity {
  name: string;
}
// Compiled with --strictNullChecks
function testEntityFunc(e: Entity) {
  return e.name;
}
Assert.equal(testEntityFunc({ name: "caihua" }), "caihua");

let x = testEntityFunc;
let s = x;
let y = x || { name: "test" };

Assert.equal(x.name, "testEntityFunc")