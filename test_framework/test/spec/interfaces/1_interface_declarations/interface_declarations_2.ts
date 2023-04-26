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
   An interface can inherit from zero or more base types which are specified in the InterfaceExtendsClause.
   The base types must be type references to class or interface types.
 ---*/


{
    interface Mover {
        x: number;

        move(): void;

        getStatus(): { speed: number; };
    }

    interface Shaker {
        y: number;

        shake(): void;

        getStatus(): { frequency: number; };
    }

    interface MoverShaker extends Mover, Shaker {
        z: number;

        getStatus(): { speed: number; frequency: number; };
    }

    let result: { speed: number; frequency: number; } = {
        speed: 1,
        frequency: 1
    };

    let point: MoverShaker = {
        x: 1,
        z: 1,
        y: 1,
        getStatus(): { speed: number; frequency: number; } {
            return result
        },
        shake(): void {
        },
        move(): void {
        }
    };

    Assert.equal(point.x, 1);
    Assert.equal(point.y, 1);
    Assert.equal(point.z, 1);
    Assert.equal(point.getStatus(), result);
    Assert.equal(point.shake(), undefined);
    Assert.equal(point.move(), undefined);
}




