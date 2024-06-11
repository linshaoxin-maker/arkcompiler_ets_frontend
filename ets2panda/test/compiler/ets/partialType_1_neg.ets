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
    Test case: Partial type annotation only modifies provided class, but not superclasses

    Explanation: Partial classes can only contain fields, to limit the generated partial types for exported classes.
                 If all superclasses would be modified by making subclass partial,
                 then it'd require the Object class to be partial too, but it has methods.
*/

class C { c_num_memb: number; }
class D extends C { d_num_memb: number; }

function foo(){
    let part_a_3: Partial<D> = {d_num_memb: 3.0, c_num_memb: 5.0};
    part_a_3.c_num_memb = undefined // CTE
}
