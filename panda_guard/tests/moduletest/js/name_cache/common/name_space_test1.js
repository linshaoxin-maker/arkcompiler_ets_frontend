/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

export function name_space_test_func_regular_export() {
    return 1;
}

function name_space_test_func_2() {
    return 2;
}

export {name_space_test_func_2 as name_space_test_func_regular_redefine_export}

export function name_space_test_func_indirect_export() {
    return 3;
}

export function name_space_test_func_4() {
    return 4;
}
