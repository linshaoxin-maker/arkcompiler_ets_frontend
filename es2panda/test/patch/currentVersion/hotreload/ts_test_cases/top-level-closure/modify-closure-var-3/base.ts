/*
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

// Test scenario: Modify the value of the variable within the UI.

class Text
{
    static fontSize = 90;
    static fontWeight = 90;
}

class Test
{
    message: string = 'hello world';

    onClick(): void
    {
        Text.fontSize = a;
        print(Text.fontSize);
    }
}

var a: number = 50;

let test: Test = new Test();
test.onClick();