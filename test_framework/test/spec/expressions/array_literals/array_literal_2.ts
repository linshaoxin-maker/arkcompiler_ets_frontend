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
   If the array literal is contextually typed by a type T with a numeric index signature, 
   the element expression is contextually typed by the type of the numeric index signature
 ---*/


var MyArray: {
  [0]: number;
  [1]: string;
  [2]: string;
} = [1, "aaa", "bbb"];

Assert.isNumber(MyArray[0]);
Assert.isString(MyArray[1]);
Assert.isString(MyArray[2]);
