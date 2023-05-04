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
    Allows destructuring a rest binding from a generic variable. 
    This is achieved by using the predefined Pick and Exclude helper types from lib.d.ts, 
    and using the generic type in question as well as the names of the other bindings in the destructuring pattern.
 ---*/


function excludeTag<T extends { tag: string }>(obj: T) {
    let { tag, ...rest } = obj
    return rest
}

const taggedPoint = { x: 10, y: 20, tag: "point" }
const point = excludeTag(taggedPoint)
Assert.equal(point, '[object Object]')