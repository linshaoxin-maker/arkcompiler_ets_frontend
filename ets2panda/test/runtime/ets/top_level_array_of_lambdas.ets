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


type FUNC = (p: int) => int
let lambda1 : FUNC[] = [
    (p: int): int =>{ return p + 1 },
    (p: int): int => { return p + 2 }
]

let lambda2 : ((p: int) => int)[] = [
    (p: int): int =>{ 
        let c : int = 1;
        return p + c;
    },
    (p: int): int => { return p + 2 }
]

function main() {
    assert lambda1[0](1) == 2;
    assert lambda1[1](1) == 3;
    assert lambda1.length == 2;

    assert lambda2[0](1) == 2;
    assert lambda2[1](1) == 3;
    assert lambda2.length == 2;    
}