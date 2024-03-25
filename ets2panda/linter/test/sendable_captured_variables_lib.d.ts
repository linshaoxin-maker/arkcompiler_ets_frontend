/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

export const libPi = 3.14;
export let libString = "lib string";

export function libFoo(): string {
  return "libFoo";
}

export enum libRGB = {RED, GREEN, BLUE };

export class libClass {
public foo(): string {
  return "lib class foo";
}


export let libClassVar: libClass;
