/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

class C  {
  bar(): string {
    return "Class C";
  }
}

function foo(c: Object|null|undefined): void {
  if (c instanceof string)  {
    assert(c.length == 11);
    c = "Case 1";
  } else if (c instanceof C) {
    assert(c.bar() == "Class C")
    c = "Case 2";
  } else if (c instanceof Int) {
    assert(c * 7 == 49);
    c = "Case 3";
  } else if (c instanceof null) {
    assert(c == null);
    c = "Case 4";
  } else {
    c = "Case 5";
  }

 assert(c.length == 6);
}


function main(): void {
  foo("Test string"); 
  foo(new Int(7));
  foo(new C());
  foo(null);
  foo(undefined);
  foo(new Number(3.0));
}
