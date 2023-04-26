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
  In a type assertion expression of the form < T > e, e is contextually typed by T and the resulting type of* e* is required to be assignable to T, 
  or T is required to be assignable to the widened form of the resulting type of e, or otherwise a compile-time error occurs. The type of the result is T.
 ---*/


function getLength(something: string | number): number {
    if ((<string>something).length) {
        return (<string>something).length
    } else {
        return something.toString().length
    }
}
Assert.equal(getLength('length'), 6)