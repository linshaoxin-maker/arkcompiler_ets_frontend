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
  In cases where excess properties are expected, an index signature can be added to the target type as an indicator of intent.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

interface Input {
  place: string;
  phanic?: boolean;
  // the any type is allowed to be the additional properties 
  [str: string]: any;
}

var site: Input = {
  place: "Address",
  phanic: true,
  // Can do this because of index signature  
  use: "Enter address here",
  // Can do this because of index signature  
  shortcut: "Alt-A"
};
Assert.equal(JSON.stringify(site), '{"place":"Address","phanic":true,"use":"Enter address here","shortcut":"Alt-A"}');