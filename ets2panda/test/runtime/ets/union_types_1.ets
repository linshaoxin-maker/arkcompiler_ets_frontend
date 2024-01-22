/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

function bar(str: string): double {
    const value = (new Number(str)).valueOf()
    const str2 = value == null ? undefined : (new Number(str)).toString()
    if (str2 == str) {
        return value
    }
    return 0.0
}

function foo(a : int | null) : Int | null {
    return a;
}

function main() : void {
    let f = foo(5);
    assert f == 5;
    assert bar("3.14") == 3.14;
}
