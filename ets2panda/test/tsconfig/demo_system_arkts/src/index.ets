/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

import * from "../include/test_1"
import * from "../include/test_2"

let inst_hehe = new hehe();

function main(): int {
    let inst_hehe1 = new hehe();
    let inst_hehe2 = new hehe_2();
    inst_hehe1.foo();
    inst_hehe2.foo_2();

    return 0;

}