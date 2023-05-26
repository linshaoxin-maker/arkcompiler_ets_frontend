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
 description: Expression operators permit operand types to include null and/or undefined but always produce values of non-null and non-undefined types.
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from "../../../suite/assert.js"

function sum(a: number, b: number) {
    return a + b;
}
sum(3, 5);

interface Entity {
    name: string;
}

function getEntityName(e: Entity): string {
    return e.name;
}
Assert.equal(getEntityName({ name: "caihua" }), "caihua");

let x: (e: Entity) => string = getEntityName;
let s: (e: Entity) => string = x;
let y: (e: Entity) => string = x || { name: "test" };
