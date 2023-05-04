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
  if S and T are object types, then for each member M in T,
  if M is a numeric index signature and S contains a numeric index signature N,
  inferences are made from the type of N to the type of M.
 ---*/


interface T {
    [key: number]: string
}
let S: { [value: number]: T } = {}
Assert.equal(typeof S, 'object')