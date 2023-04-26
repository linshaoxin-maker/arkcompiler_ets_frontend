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
  TypeScript 4.7 allows exactly that we can now take functions and constructors and feed them type arguments directly.
 ---*/


interface Box<T> {
    value: T
}
interface Hammer {
    name: string
}
interface Wrench {
    use: string
}
function makeBox<T>(value: T) {
    return { value }
}

const makeHammerBox = makeBox<Hammer>({ name: 'hammer' })
Assert.equal(makeHammerBox, '[object Object]')
const makeWrenchBox = makeBox<Wrench>({ use: 'turn the screw' })
Assert.equal(makeWrenchBox, '[object Object]')