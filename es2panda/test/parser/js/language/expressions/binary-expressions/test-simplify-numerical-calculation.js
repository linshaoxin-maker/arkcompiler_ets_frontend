/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let continueAdd = 1 + 2 + 3;
let continueMinus = 1 - 2 - 3;
let continueMultiply = 1 * 2 * 3;
let continueDivide = 1 / 2 / 3;
let continuePow = (2 ** 3) ** 2;
let continueOr = 1 | 0 | 0;
let continueAnd = 1 & 0 & 0;
let continueXor = 1 ^ 0 ^ 0;
let continuePositiveLeftShift = 10 << 1 << 1;
let continuePositiveRightShift = 10 >> 1 >> 1;
let continueUnsignedRightShift = 10 >>> 1 >>> 1;
let diffPriority = 1 + 2 * 3 / 2 - (1 | 0) & 1 ^ 0 + (10 << 1) >>> 2;
