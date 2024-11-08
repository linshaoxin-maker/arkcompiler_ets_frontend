/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

function foo() {
  return {Propx1: 1, Propy1: 2};
}

const {Propx1, Propy1} = foo();
print(Propx1 === 1, 'success');
print(Propy1 === 2, 'success');

// let Propx2 =3;
// let Propy3 =4;
// let Propy4 =5;
const {Propx2, Obj: {Propy3, Propy4}} = {Propx2: 12, Obj: {Propy3: 13, Propy4: 14}}
print(Propx2 === 12, 'success');
print(Propy3 === 13, 'success');
print(Propy4 === 14, 'success');


const {...rest1} = {prop1: 11, prop2: 12};
print(rest1.prop1 === 11, 'success');
print(rest1.prop2 === 12, 'success');

const {prop3, ...rest2} = {prop3: 13, prop4: 14};
print(prop3 === 13, 'success');
print(rest2.prop4 === 14, 'success');

let name11 = 'hello';
let info = {name11};