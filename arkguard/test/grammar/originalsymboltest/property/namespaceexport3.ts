/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

let toplevelElement7 = 1; // export and export obfuscation
let toplevelElement8 = 1; // toplevel obfuscation
let toplevelElement9 = 1; // kept
declare namespace TestNs2{
  export{toplevelElement7, toplevelElement9};
  export{toplevelElement8 as te8};
}
let nsElement1 = TestNs2.toplevelElement7;
let nsElement2 = TestNs2.te8;
export{};