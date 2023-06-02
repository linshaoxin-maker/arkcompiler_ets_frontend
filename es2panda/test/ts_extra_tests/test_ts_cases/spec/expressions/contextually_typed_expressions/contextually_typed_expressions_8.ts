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
  In a typed function call, argument expressions are contextually typed by their corresponding parameter types.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../suite/assert.js'

const state: Record<string, any> = {
  isPending: false,
  results: ['a', 'b', 'c']
}

const useValue = <T extends {}>(name: string): [T, Function] => {
  const value: T = state[name]
  const setValue: Function = (value: T): void => {
    state[name] = value
  }
  return [value, setValue]
}

const [isPending, setIsPending] = useValue('isPending')
const [results, setResults] = useValue('results')

Assert.isBoolean(isPending)
Assert.equal(results, 'a,b,c');