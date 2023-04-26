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
    If the object literal is contextually typed, if the contextual type contains a numeric index signature, 
    and if the property assignment specifies a numeric property name, the property assignment is contextually typed by the type of the numeric index signature.
 ---*/


let numeric_object: {
  1: string;
  2: number;
  3(name: string): string;
  get job(): string;
  set job(jobName: string);
} = {
  1: "mingshuo",
  2: 18,
  3(name) {
    return name;
  },
  job: "student",
};
Assert.equal(numeric_object.job, "student");
numeric_object.job = "teacher";
Assert.equal(numeric_object.job, "teacher");
