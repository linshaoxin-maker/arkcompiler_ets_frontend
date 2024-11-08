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

let arr = [0, 1, null, '1']

print(arr instanceof Array, 'success');

let num: any = 333;

print(typeof num === 'number', 'success');

let str: any = '[0, 1, null]';

print(typeof str === 'string', 'success');

class Animal {
  move(distanceInMeters: number = 0): void {
    print(`Animal moved ${distanceInMeters}m.`);
  }
}

let ani = new Animal();

print(ani instanceof Animal, 'success');