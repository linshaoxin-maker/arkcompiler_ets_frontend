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
    Since nothing is known about the type on the left side of the &&, 
    we propagate any and unknown outward instead of the type on the right side.
 ---*/


let foo: unknown;
declare let somethingElse: { someProp: string };
let anyx = foo && somethingElse;
function isThing(x: any): boolean {
  return x && typeof x === "object" && x.blah === "foo";
}
let y = isThing(anyx);
Assert.isUndefined(y);
