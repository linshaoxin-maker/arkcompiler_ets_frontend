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

import {obj4} from "./object_test2";

export const obj1 = {
    obj_field1: 1
}
obj1.obj_field2 = 2;

const obj2 = {
    obj_field4: 4
}

obj2.obj_field5 = 5;

export function test_object_property_func() {
    obj1.obj_field3 = 3;
    obj2.obj_field6 = 6;

    const obj3 = {
        obj_field7: 7
    }
    obj3.obj_field8 = 8;
    obj3["obj_field9"] = 9;

    obj4.obj_field11 = 11;

    return obj1.obj_field1 + obj1.obj_field2 + obj1.obj_field3
        + obj2.obj_field4 + obj2.obj_field5 + obj2.obj_field6
        + obj3.obj_field7 + obj3.obj_field8 + obj3["obj_field9"]
        + obj4.obj_field10 + obj4.obj_field11;
}