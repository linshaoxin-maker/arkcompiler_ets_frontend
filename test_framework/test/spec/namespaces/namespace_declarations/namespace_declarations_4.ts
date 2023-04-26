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
    a namespace declaration that specifies an IdentifierPath with more than one identifier is equivalent to a series of nested single-identifier namespace declarations where all but the outermost are automatically exported. 
 ---*/


namespace A.B.C {
   export var x = "This is equivalent to the code below.";
}
Assert.equal(A.B.C.x, "This is equivalent to the code below.");
namespace A {
   export namespace B {
      export namespace C {
         export var x = "This is equivalent to the code above.";
      }
   }
}
Assert.equal(A.B.C.x, "This is equivalent to the code above.");
