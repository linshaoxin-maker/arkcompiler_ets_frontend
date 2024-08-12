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
import assert from 'assert';
import f1 from './export_api';
import * as g1 from './export_api';
import {} from './export_api';
import { i } from './export_api';
import { j, k, } from './export_api';
import { default as h1 } from './export_api';
import i1, * as j1 from './export_api';
import l1, { h, } from './export_api';
assert(f1(2, 3) === 3);
assert(g1.i === 1);
assert(i === 1);
assert(j === 2);
assert(k === 3);
assert(h1(3, 4) === 4);
assert(i1(7, 8) === 8);
assert(j1.k === 3);
assert(l1(3, 4) === 4);
assert(h(5, 4) === 1);
// import ""; TypeError: The argument 'id' must be a non-empty string. Received ''
// import '';
import './export_api';
import { t } from './export_api_02';
import * as m1 from './export_api_02';
assert(t.g(3, 4) === 7);
assert(m1.t.h(5, 4) === 1);
