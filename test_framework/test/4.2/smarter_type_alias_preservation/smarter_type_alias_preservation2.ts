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
  If we hover our mouse over x in an editor like Visual Studio, Visual Studio Code, or the TypeScript Playground, we’ll get a quick info panel that shows the type BasicPrimitive. Likewise, if we get the declaration file output (.d.ts output) for this file, TypeScript will say that doStuff returns BasicPrimitive.
---*/


type BasicPrimitive = number | string | boolean;

function doStuff(value: BasicPrimitive) {
  if (Math.random() < 0.5) {
    Assert.equal(Math.random(), 0.3)
    return undefined;
  }

  return value;
}

let arr = [10, "hello", false];
Assert.equal(typeof arr[0], "number")