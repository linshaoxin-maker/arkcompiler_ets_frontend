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
    if the member declaration specifies no value, the member is considered a constant enum member. 
    If the member is the first member in the enum declaration, it is assigned the value zero. 
    Otherwise, it is assigned the value of the immediately preceding member plus one, and an error occurs if the immediately preceding member is not a constant enum member.
    if the member declaration specifies a value that can be classified as a constant enum expression, 
    the member is considered a constant enum member.otherwise, the member is considered a computed enum member.
 ---*/


enum ABCList {
  A,
  B,
  C = "string".length,
  D = 10,
  E,
  F = ~17,
  G = 0x0f << 0x02,
  H = 0xff & 0xaa,
  I = E | F,
}
Assert.equal(ABCList.A, 0);
Assert.equal(ABCList.B, 1);
Assert.equal(ABCList.C, 6);
Assert.equal(ABCList.D, 10);
Assert.equal(ABCList.E, 11);
Assert.equal(ABCList.F, -18);
Assert.equal(ABCList.G, 60);
Assert.equal(ABCList.H, 170);
Assert.equal(ABCList.I, -17);
