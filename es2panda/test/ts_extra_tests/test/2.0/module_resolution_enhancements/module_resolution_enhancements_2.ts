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
  The TypeScript compiler supports the declaration of such mappings using paths property in tsconfig.json files,
  Path mapping.
  The tsc command needs to be executed in the 2.0 file directory.
  tsconfig.json file url: testing-framework\test\2.0\tsconfig.json.
module: ESNext
isCurrent: true
---*/


import { Assert } from "../../../suite/assert.js"
import { Directions } from "../test_virtual_directories_with_rootDirs/view2.js"

Assert.equal(Directions.Up, 0);
