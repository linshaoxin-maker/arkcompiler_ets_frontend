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
  In the body of a get accessor with no return type annotation, 
  if a matching set accessor exists and that set accessor has a parameter type annotation, 
  return expressions are contextually typed by the type given in the set accessor's parameter type annotation.
 ---*/


class Developer {
    private _language = ''
    private _tasks: string[] = []
    get language() {
        return this._language
    }
    set language(value: string) {
        this._language = value
    }
    get tasks() {
        return this._tasks
    }
    set tasks(value: string[]) {
        this._tasks = value
    }
}

const dev = new Developer()
dev.language = 'TS'
Assert.isString(dev.language)
dev.tasks = ['develop', 'test']
dev.tasks.push('ship')
Assert.equal(dev.tasks, 'develop,test,ship')