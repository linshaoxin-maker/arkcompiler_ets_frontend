///<reference path="ambient_module_declarations_1.d.ts"/>
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


declare module AMD2_1 {
    interface AMD2_1IF {
        a2_1: number;
        a2_2: number;
    }
    var AMD2Var1: string;
    export import X = AMD1;
    export import Y = AMD2_2;
}
declare module AMD2_2 {
    interface AMD2_2IF {
        a2_1: string;
        a2_2: string;
        a2_3: string;
    }
    var AMD2Var2: number;
}

