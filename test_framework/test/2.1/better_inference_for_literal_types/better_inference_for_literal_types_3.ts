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
description: Literal type widening can be controlled through explicit type annotations.
 ---*/


const c1 = "hello";
// Type string
let v1 = c1;
v1 = "world";
const c2: "hello" = "hello";
let v2 = c2;
Assert.equal(v2, "hello");

const c3 = 1;
let c4 = c3;
c4 = 10;
Assert.equal(c4, 10);
const c5: 1 = 1;
let c6 = c5;
c6 = 1;
Assert.equal(c6, 1);

const c7 = false;
let c8 = c7;
c8 = true;
Assert.equal(c8, true);
const c9: false = false;
let c10 = c9;
c10 = false;
Assert.equal(c10, false);