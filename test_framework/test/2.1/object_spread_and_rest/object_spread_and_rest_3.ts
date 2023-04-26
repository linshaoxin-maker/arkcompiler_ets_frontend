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
  override existing properties and add new ones    
  The order of specifying spread operations determines what properties end up in the resulting object; 
  properties in later spreads "win out" over previously created properties.
 ---*/


let person = {
  name: "caihua",
  age: 20,
};

let copy = { ...person, name: "huahua", job: "teacher" };

let { job, ...restProperties } = copy;
Assert.equal(job, copy.job);
Assert.equal(restProperties.age, copy.age);
Assert.equal(restProperties.name, copy.name);
