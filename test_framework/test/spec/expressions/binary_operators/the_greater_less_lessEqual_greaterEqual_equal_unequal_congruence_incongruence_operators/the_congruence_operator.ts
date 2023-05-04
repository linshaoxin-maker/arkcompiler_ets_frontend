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
  The '===' operator require one or both of the operand types to be assignable to the other. 
  The result is always of the Boolean primitive type.
 ---*/


var a: any = undefined
var b: boolean = true
var c: number = 10
var d: string = 'str'
Assert.isBoolean(a === b)
Assert.isBoolean(a === c)
Assert.isBoolean(a === d)