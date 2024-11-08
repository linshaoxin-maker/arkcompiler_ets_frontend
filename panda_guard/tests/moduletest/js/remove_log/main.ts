/**
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

function func1(v1, v2) {
    if (v1 === v2) {
        return;
    }
    console.log("v1 not equal v2");
}

const calc = (a, b) => {
    if (a < b) {
        console.log('a < b');
    } else {
        console.log('b > b');
    }
}

func1(1, 2);
calc(3, 4);
console.log("hello world");

let a = 1;
let b = 2;
let c = 3;
let d = 4;
console.log("parma has 1");
console.log("parma has 2", a);
console.log("parma has 3", a, b);
console.log("parma has 4", a, b, c);
console.log("parma has 5", a, b, c, d);
