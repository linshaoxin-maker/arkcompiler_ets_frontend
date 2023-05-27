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
  test base url import,the tsc command needs to be executed in the 2.0 file directory.
  tsconfig.json file url: testing-framework\test\2.0\tsconfig.json.
module: ESNext
isCurrent: true
---*/


import { Assert } from "../../../suite/assert.js"
import { ADD, flag } from "../test_base_url.js"

Assert.equal(ADD(50, 75), 125);
Assert.equal(flag, "baseurl");
