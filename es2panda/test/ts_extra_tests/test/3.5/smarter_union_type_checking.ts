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
   Smarter union type checking.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../suite/assert.js'

{
    type PointData = {
        x: number;
        y: number;
        name: string;
    };

    type Data = {
        name: string;
    };

    const point: PointData | Data = {
        x: 0,
        y: 0,
        name: 'name'
    }
    Assert.equal(point.name, 'name')
}