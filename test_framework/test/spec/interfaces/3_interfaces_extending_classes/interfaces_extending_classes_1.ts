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
   When an interface type extends a class type it inherits the members of the class but not their implementations.
   It is as if the interface had declared all of the members of the class without providing an implementation.
 ---*/


{
    class Control {
        private state: any = 0;
        point: any = 0;
    }

    interface SelectableControl extends Control {
        select(): number;
    }

    class Button extends Control implements SelectableControl {
        select(): number {
            return 0
        }
    }

    Assert.isTrue(new Button().hasOwnProperty('point'));
    Assert.isTrue(new Button().hasOwnProperty('state'));

}
