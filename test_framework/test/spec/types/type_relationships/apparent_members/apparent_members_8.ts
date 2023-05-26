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
  a type's apparent members make it a subtype of the 'Object' or 'Function' interface 
 ---*/


var o: Object = { x: 10, y: 20 };
Assert.equal(JSON.stringify(o), '{"x":10,"y":20}')

var f: Function = (x: number) => x * x;
Assert.isUndefined(JSON.stringify(f))