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
    allows you to re-map keys in mapped types with a new as clause.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

type G<T> = {
  [K in keyof T as `get${Capitalize<string & K>}`]: () => T[K];
};
interface I1 {
  name: string;
  id: number;
  lo: string;
}
type I1Type = G<I1>;
let k1: I1Type = {
  getName() {
    return "honny";
  },
  getId() {
    return 2;
  },
  getLo() {
    return "qingdao";
  },
};
Assert.equal(k1.getName(), "honny");
Assert.equal(k1.getId(), 2);
Assert.equal(k1.getLo(), "qingdao");