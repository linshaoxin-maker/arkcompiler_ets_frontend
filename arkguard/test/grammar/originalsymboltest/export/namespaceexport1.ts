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

let toplevelElement1 = 1; // export and export obfuscation
let toplevelElement2 = 1; // toplevel obfuscation
let toplevelElement3 = 1; // kept
declare namespace TestNs1{
  export{toplevelElement1, toplevelElement3};
  export{toplevelElement2 as te2};
}
let nsElement1 = TestNs1.toplevelElement1;
let nsElement2 = TestNs1.te2;
export{};