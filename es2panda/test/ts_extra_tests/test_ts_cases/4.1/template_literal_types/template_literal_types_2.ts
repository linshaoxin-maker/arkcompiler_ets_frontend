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
    template literal string type has the same syntax as template literal strings in JavaScript, but is used in type positions.   
    When you use it with concrete literal types, it produces a new string literal type by concatenating the contents.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../suite/assert.js'

type World = "world";
type Greeting = `hello ${World}`;
function helloworld(w: World | Greeting) {
  if (w == "world") return 1;
  else if (w == "hello world") return 2;
}
let saying1 = helloworld("world");
Assert.equal(saying1, 1);
let saying2 = helloworld("hello world");
Assert.equal(saying2, 2);
type Color = "red" | "blue";
type Quantity = "one" | "two";
type SeussFish = `${Quantity | Color} fish`;
function getfish(fish: SeussFish) {
  if (fish == "one fish") return 1;
  else if (fish == "two fish") return 2;
  else if (fish == "red fish") return "red";
  else if (fish == "blue fish") return "blue";
}
let fish1 = getfish("one fish");
Assert.equal(fish1, 1);
let fish2 = getfish("two fish");
Assert.equal(fish2, 2);
let fish3 = getfish("red fish");
Assert.equal(fish3, "red");
let fish4 = getfish("blue fish");
Assert.equal(fish4, "blue");
