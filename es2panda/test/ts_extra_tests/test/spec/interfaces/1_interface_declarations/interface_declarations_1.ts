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
   An interface declaration declares an interface type.
   An InterfaceDeclaration introduces a named type (section 3.7) in the containing declaration space.
   The BindingIdentifier of an interface declaration may not be one of the predefined type names (section 3.8.1).
 module: ESNext
 isCurrent: true
 ---*/


import { Assert } from '../../../../suite/assert.js'

{
    interface Worker {
        x: number;

        work(): void;

        getS(): { velocity: number; };
    }

    interface Tool {
        y: number;

        tool(): void;

        getS(): { rate: number; };
    }

    let result1: { rate: number; } = {
        rate: 1
    };
    let result2: { velocity: number; } = {
        velocity: 1,
    };

    let point: Worker = {
        x: 1,
        getS(): { velocity: number; } {
            return result2
        },
        work(): void {
        }
    };
    let pointer: Tool = {
        y: 1,
        getS(): { rate: number; } {
            return result1
        },
        tool(): void {
        },
    };

    Assert.equal(point.x, 1);
    Assert.equal(pointer.y, 1);
    Assert.equal(point.getS(), result2);
    Assert.equal(pointer.getS(), result1);
    Assert.equal(pointer.tool(), undefined);
    Assert.equal(point.work(), undefined);
};
