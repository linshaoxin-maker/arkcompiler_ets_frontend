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
 description: TypeScript 2.5 implements a new ECMAScript feature that allows users to omit the variable in catch clauses.
 ---*/


let input = "have a exception!";
try {
    JSON.parse(input);
} catch {
    // ^ Notice that our `catch` clause doesn't declare a variable.
    Assert.equal("have a exception!", input);
}