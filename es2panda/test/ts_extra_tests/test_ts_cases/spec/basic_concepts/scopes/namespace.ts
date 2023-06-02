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
    locally declared entities in a namespace are closer in scope than exported entities declared in other namespace declarations for the same namespace.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../suite/assert.js'

var num = 1;
namespace ns {
    export var num = 2;
    Assert.equal(num, 2);
}
namespace ns {
    Assert.equal(num, 2);
}
namespace ns {
    var num = 3;
    Assert.equal(num, 3);
}