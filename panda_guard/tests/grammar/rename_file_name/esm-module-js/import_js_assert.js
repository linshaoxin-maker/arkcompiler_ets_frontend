/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

// default import
import defaultExport from "./export_default";

let classObj = new defaultExport();
print(classObj.classProp1 === 0, 'success');

// namespace import
import * as namespace01 from "./folder/export_declarations";

print(namespace01.function01() === 10, 'success');

import {value01, generatorFunctionName01} from "./folder/export_declarations.js";

print(value01 === 1, 'success');
const generator = generatorFunctionName01();
print(generator.next().value['age'] === 20, 'success');

// rename an export when importing it
import {value03 as value03Alias} from "./folder/export_declarations";

print(value03Alias === 3, 'success');

// import multiple names from the same module.
import {name1, name2 as alias2} from "./export_list_js";

print(name1 === 1, 'success');
print(alias2 === 2, 'success');

// static Import
import "./folder/whole_modue";
