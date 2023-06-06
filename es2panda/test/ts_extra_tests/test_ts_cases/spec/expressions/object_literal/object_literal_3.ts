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
    If the object literal is contextually typed and the contextual type contains a string index signature, 
    the property assignment is contextually typed by the type of the string index signature.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../suite/assert.js'

let animal: {
  name: String;
  age: Number;
  run: Number;
  bark(run: Number): Number;
} = {
  name: "Tom",
  age: 12,
  run: 23,
  bark(run) {
    return run;
  },
};
Assert.equal(animal.run, 23);
animal.run = 66;
Assert.equal(animal.run, 66);