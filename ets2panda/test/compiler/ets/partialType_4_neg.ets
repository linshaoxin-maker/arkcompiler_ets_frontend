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

/*
    Test case: T type of Partial<T> can not contain methods
    Explanation: We'll need to generate partial class for all exported classes, to solve generation duplicate problems.
                 To limit the number of generated partial classes, we'll only generate classes which contain fields (as this is the main use case).
                 Hence if we don't generate partial classes with methods in it, no base type of a partial class can contain methods.

    NOTE:
         - with this restriction, no interface can be Partial (because fields will always be getters/setters)
         - also, any class which implements any interface can't be Partial (because getters/setters will be generated for fields)
*/

interface C {
	num_fld: number
}

function baz(): void {
	let part_c: Partial<C> = {num_fld: 5}; // CTE: num_fld is a getter method, Partial can only contain fields
}
