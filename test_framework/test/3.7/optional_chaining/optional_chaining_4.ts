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
  There's also optional call, which allows us to conditionally call expressions if they’re not null or undefined.
---*/


async function makeRequest(url: string, log?: (msg: string) => void) {
  log?.(`Request started at ${new Date().toISOString()}`);
  const result = ((await fetch(url)).json());
  log?.(`Request finished at ${new Date().toISOString()}`);
  return result;
}
makeRequest("http://127.0.0.1:8081/test.txt").then(res => {
  Assert.equal(typeof res, "object");
}).catch(err => {
});