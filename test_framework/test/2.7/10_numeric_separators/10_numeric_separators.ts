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
description: TypeScript 2.7 brings support for ES Numeric Separators. Numeric literals can now be separated into segments using _.
 ---*/


const million = 1_000_000;
const phone = 555_734_2231;
const bytes = 0xff_0c_00_ff;
const word = 0b1100_0011_1101_0001;

Assert.equal(1000000, million);
Assert.equal(5557342231, phone);
Assert.equal(4278976767, bytes);
Assert.equal(50129, word);