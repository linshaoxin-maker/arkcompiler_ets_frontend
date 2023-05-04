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
    recrent versions of TypeScript (around 3.7) have had updates to the declarations of functions like Promise.all and Promise.race. 
    unfortunately, that introduced a few regressions, especially when mixing in values with null or undefined.
    this issue has now been fixed.
 options:
    lib: es2015
 ---*/


interface Lion {
    roar(): void;
}

interface Seal {
    singKissFromARose(): void;
}

async function visitZoo(
    lionExhibit: Promise<Lion>,
    sealExhibit: Promise<Seal | undefined>
) {
    let [lion, seal] = await Promise.all([lionExhibit, sealExhibit]);
    lion.roar();
}
Assert.isString("The \"lion.roar()\" in the above example was incorrectly reported as having the value \"undefined\" and has now been fixed.")