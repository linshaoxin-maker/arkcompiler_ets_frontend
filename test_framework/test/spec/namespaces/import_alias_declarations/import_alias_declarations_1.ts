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
    import alias declarations are used to create local aliases for entities in other namespaces.
    an EntityName consisting of a single identifier is resolved as a NamespaceName and is thus required to reference a namespace. The resulting local alias references the given namespace and is itself classified as a namespace.
 ---*/


namespace B {
    interface A { n: number }
    export import Y = A;
    import Z = A.X;
    export var vb: Z = { s: "v" };
}
namespace A {
    export interface X { s: string }
    export import va = B.vb;
}
var v1: B.Y.X = B.vb;
var v2 = A.va;
let iadFlag: boolean = false;
if (v1 == v2) {
    iadFlag = true;
}
Assert.isTrue(iadFlag);

