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

import {default as varAlis} from './y1_export_default_value';
import defaultExport, {CDE1} from './y1_import_alias';

let ins = new CDE1.ClassAlis;
print(ins.classProp1 === 2, 'success');
type typeAlis = CDE1.InterOriginal;
print(CDE1.fooOriginal() === '11', 'success');
print(defaultExport() === 22, 'success');
print(varAlis === 5, 'success');