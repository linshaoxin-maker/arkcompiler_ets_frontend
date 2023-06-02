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
 description: TypeScript is able to narrow types based on what’s called a discriminant property. For example, in the following code snippet, TypeScript is able to narrow the type of action based on every time we check against the value of kind.
 module: ESNext
 isCurrent: true
---*/


import { Assert } from '../../suite/assert.js'

type HWA =
  | { kind: "NumberContents"; payload: number }
  | { kind: "StringContents"; payload: string };

function hwtest01(action: HWA) {
  if (action.kind === "NumberContents") {
    // `action.payload` is a number here.
    let num = action.payload * 2;
    return num;
  } else if (action.kind === "StringContents") {
    // `action.payload` is a string here.
    const str = action.payload.trim();
    return str;
  }
}

let action1: HWA = {
  kind: "NumberContents",
  payload: 1
}

let action2: HWA = {
  kind: "StringContents",
  payload: " 1234 "
}

Assert.equal(2, hwtest01(action1));
Assert.equal("1234", hwtest01(action2));


// you might have wanted to destructure kind and payload in the the example above. 
type Action2 =
  | { kind: "NumberContents"; payload: number }
  | { kind: "StringContents"; payload: string };

function hwtest02(action: Action2) {
  const { kind, payload } = action;
  if (kind === "NumberContents") {
    let num = payload * 2;
    return num;
  } else if (kind === "StringContents") {
    const str = payload.trim();
    return str;
  }
}


let action3: HWA = {
  kind: "NumberContents",
  payload: 2
}

let action4: HWA = {
  kind: "StringContents",
  payload: " 5678 "
}

Assert.equal(hwtest02(action3),4);
Assert.equal(hwtest02(action4),"5678");