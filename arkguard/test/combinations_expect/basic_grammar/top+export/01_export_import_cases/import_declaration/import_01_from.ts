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
import v from 'assert';
import w from './export_api';
import * as x from './export_api';
import {} from './export_api';
import { d } from './export_api';
import { e, f, } from './export_api';
import { default as y } from './export_api';
import z, * as a1 from './export_api';
import b1, { b, } from './export_api';
v(w(2, 3) === 3);
v(x.var1 === 1);
v(d === 1);
v(e === 2);
v(f === 3);
v(y(3, 4) === 4);
v(z(7, 8) === 8);
v(a1.var3 === 3);
v(b1(3, 4) === 4);
v(b(5, 4) === 1);
// import ""; TypeError: The argument 'id' must be a non-empty string. Received ''
// import '';
import './export_api';
import { m } from './export_api_02';
import * as c1 from './export_api_02';
v(m.addFunc(3, 4) === 7);
v(c1.moduleAlias.reduceFunc(5, 4) === 1);
