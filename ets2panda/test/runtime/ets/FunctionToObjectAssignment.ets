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

function main() {
    let v1: Object = (): Int => {return 24;};
    let v2: ()=>Int = v1 as (()=>Int);
    assert 24 == v2();

    let v3: Object = foo;
    let v4: ()=>number = v3 as (()=>number);
    assert 42.0 == v4()
}

function foo(): number {
  return 42.0;
}