///<reference path="ambient_module_declarations_1.d.ts"/>
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
    ambient modules are "open-ended" and ambient module declarations with the same string literal name contribute to a single module.
 ---*/


var am2: AMD1.AMD1IF = { a1_1: "am2", a1_2: 123 };
Assert.equal(JSON.stringify(am2), '{"a1_1":"am2","a1_2":123}')
