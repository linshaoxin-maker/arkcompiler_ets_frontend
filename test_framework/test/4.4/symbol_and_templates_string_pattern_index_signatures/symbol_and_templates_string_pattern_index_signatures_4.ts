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
description: we can write an index signature with template string pattern type
 ---*/

 
interface PersonInfo {
    name: string;
    age: number;
}
interface PersonInfoWithExtraProperties extends PersonInfo {
    [optName: `personal-${string}`]: unknown;
}
let a: PersonInfoWithExtraProperties = {
    name: "wangcai",
    age: 20,
    "personal-specialHobby": "eat",
};
Assert.isString(a.name);
Assert.isNumber(a.age);
Assert.equal(a["personal-specialHobby"], "eat");
