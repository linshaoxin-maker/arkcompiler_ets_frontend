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

let nums = [0, 1, 2, 3, 4, 5];

function loggingIdentity1<T>(arg: T[]): T[] {
  return arg;
}

print(loggingIdentity1(nums).length === 6, 'success');

print(loggingIdentity1(nums).indexOf(1) === 1, 'success');

function loggingIdentity2<T>(arg: Array<T>): Array<T> {
  print(arg.length);  // Array has a .length, so no more error
  return arg;
}

print(loggingIdentity2(nums).length === 6, 'success');