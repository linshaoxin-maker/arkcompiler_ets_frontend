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
 description: Improved Recursion Depth Checks
---*/


// In a structural type system, object types are compatible based on the members they have.
interface Source<T> {
    prop: Source<Source<T>>;
}

interface Target<T> {
    prop: Target<Target<T>>;
}

function check(source: Source<string>, target: Target<number>) {
    target = source;
}


interface Foo<T> {
    prop: T;
}

declare let x: Foo<Foo<Foo<Foo<Foo<Foo<string>>>>>>;
declare let y: Foo<Foo<Foo<Foo<Foo<string>>>>>;

// TypeScript 4.6 is now able to distinguish these cases, and correctly errors on the last example.
// x = y; this is error;

Assert.equal(1, 1);