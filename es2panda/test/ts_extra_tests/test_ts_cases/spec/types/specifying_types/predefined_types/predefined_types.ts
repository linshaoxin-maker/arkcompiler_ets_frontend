/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
/**---
 description: >
    The any, number, boolean, string, symbol and void keywords reference 
    the Any type and the Number, Boolean, String, Symbol, and Void primitive types respectively.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../../suite/assert.js'

let m_n: number = 5;
Assert.equal(m_n.toString(), "5");

let m_b: boolean = true;
Assert.equal(m_b.toString(), "true");

let m_s: string = 's';
Assert.equal(m_s.toString(), "s");

let m_sy: symbol = Symbol();
Assert.equal(m_sy.toString(), "Symbol()");