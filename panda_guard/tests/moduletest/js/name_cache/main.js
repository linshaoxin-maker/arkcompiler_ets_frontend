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

/************************** class test **************************/
import {class1, test_class_in_func} from './common/class_test'

let classObj = new class1();
print(`class1 :` + classObj.add());
print(`test_class_in_func :` + test_class_in_func());

/************************** method test **************************/
import {methodClass1, test_method_in_func} from './common/method_test'

let methodObj = new methodClass1();
print(`methodClass1 :` + methodObj.methodAdd1());
print(`test_method_in_func :` + test_method_in_func());

/************************** property test **************************/
import {propertyClass1, test_property_in_func} from './common/property_test'

let propertyObj = new propertyClass1();
print(`propertyClass1 :` + propertyObj.field1 + propertyClass1.sField1);
print(`test_property_in_func :` + test_property_in_func());

/************************** function test **************************/
import {function_test1, test_function_in_func} from './common/function_test'

print(`function_test1 :` + function_test1());
print(`test_function_in_func :` + test_function_in_func());

/************************** record test **************************/
import {recordObject1, test_record_in_func} from './common/record_test'

print(`recordObject1 :` + recordObject1["age1"] + recordObject1.age1);
print(`test_record_in_func :` + test_record_in_func());

/************************** name space test **************************/
import {
    name_space_test_func_regular_export,
    name_space_test_func_regular_redefine_export
} from './common/name_space_test1'
import {
    name_space_test_func_indirect_export,
    name_space_test_func_indirect_redefine_export
} from './common/name_space_test2'
import * as MyNameSpace from './common/name_space_test3'

print(`name_space_test_func_regular_export :` + name_space_test_func_regular_export());
print(`name_space_test_func_regular_redefine_export :` + name_space_test_func_regular_redefine_export());
print(`name_space_test_func_indirect_export :` + name_space_test_func_indirect_export());
print(`name_space_test_func_indirect_redefine_export :` + name_space_test_func_indirect_redefine_export());
print(`name_space_test_func_5 :` + MyNameSpace.name_space_test_func_5());
print(`name_space_test_func_6 :` + MyNameSpace.name_space_test_func_6());
