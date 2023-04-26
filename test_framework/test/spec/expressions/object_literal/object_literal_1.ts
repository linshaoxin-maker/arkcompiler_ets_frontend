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
    If the object literal is contextually typed and the contextual type contains a property with a matching name, 
    the property assignment is contextually typed by the type of that property.
 ---*/


let cc: {
  name: string;
  age: number;
  callMe(name: string): string;
  get job(): string;
  set job(jobName: string);
} = {
  name: "fan",
  age: 20,
  callMe(name) {
    return name;
  },
  job: "student",
};
Assert.equal(cc.job, "student");
cc.job = "teacher";
Assert.equal(cc.job, "teacher");
