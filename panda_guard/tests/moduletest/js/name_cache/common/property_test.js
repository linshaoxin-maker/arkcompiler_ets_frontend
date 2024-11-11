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

export class propertyClass1 {
    static sField1 = 1;
    field1 = 2;
}

class propertyClass2 {
    static sField2 = 3;
    field2 = 4;
}

export function test_property_in_func() {
    class propertyClass3 {
        static sField3 = 5;
        field3 = 6;
    }

    let obj2 = new propertyClass2();
    return obj2.field2 + propertyClass3.sField3;
}