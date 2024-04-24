/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

/*---
flags: [dynamic-ast]
---*/

// JS code:
// Class A {
//      constructor(arg1, arg2) {}
// } 
// exports.NamespaceOfClasses = {A}

import { A, NamespaceOfClasses } from "dynamic_js_import_tests"

function main(): int {
    let a: JSValue = new A("arg", 1);
    if (!(a instanceof A)) {
        return 1;
    }
    let varA: JSValue = A;
    let var_a: JSValue = new varA("arg", 1);
    if (!(var_a instanceof varA)) {
        return 1;
    }
    let fromNamespace: JSValue = new NamespaceOfClasses.A("arg", 1);
    // NOTE(staroverovag): support member access in instanceof rhs
    // if (!(fromNamespace instanceof NamespaceOfClasses.A)) {
    //     return 1;
    // }
    let varNameA: JSValue = NamespaceOfClasses.A;
    let var_name_a: JSValue = new varNameA("arg", 1);
    if (!(var_name_a instanceof varNameA)) {
        return 1;
    }
    let varName: JSValue = NamespaceOfClasses;
    let var_var_name_a: JSValue = new varName.A("arg", 1);
    // NOTE(staroverovag): support member access in instanceof rhs
    // if (!(var_var_name_a instanceof varName.A)) {
    //     return 1;
    // }
    return 0;
}
