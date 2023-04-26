///<reference path="declaration_merging_1_1.ts"/>
///<reference path="declaration_merging_1_2.ts"/>
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
    namespaces are "open-ended" and namespace declarations with the same qualified name relative to a common root (as defined in section 2.3) contribute to a single namespace. 
 ---*/


var o1: outer.OA = { OA: "OA" };
var o2: outer.OB = { OB: 1024 };
Assert.isString(o1.OA);
Assert.isNumber(o2.OB);