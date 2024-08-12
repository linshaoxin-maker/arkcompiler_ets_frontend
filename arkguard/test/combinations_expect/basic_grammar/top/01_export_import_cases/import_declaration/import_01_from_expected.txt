/*
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
import j from 'assert';
import k from './export_api';
import * as l from './export_api';
import {} from './export_api';
import { var1 } from './export_api';
import { var2, var3, } from './export_api';
import { default as m } from './export_api';
import n, * as o from './export_api';
import p, { reduceFunc, } from './export_api';
j(k(2, 3) === 3);
j(l.var1 === 1);
j(var1 === 1);
j(var2 === 2);
j(var3 === 3);
j(m(3, 4) === 4);
j(n(7, 8) === 8);
j(o.var3 === 3);
j(p(3, 4) === 4);
j(reduceFunc(5, 4) === 1);
// import ""; TypeError: The argument 'id' must be a non-empty string. Received ''
// import '';
import './export_api';
import { moduleAlias } from './export_api_02';
import * as q from './export_api_02';
j(moduleAlias.addFunc(3, 4) === 7);
j(q.moduleAlias.reduceFunc(5, 4) === 1);
