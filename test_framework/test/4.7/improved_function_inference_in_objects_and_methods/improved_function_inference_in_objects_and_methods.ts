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
  TypeScript 4.7 can now perform more granular inferences from functions within objects and arrays. 
  This allows the types of these functions to consistently flow in a left-to-right manner just like for plain arguments.
 ---*/


function f<T>(arg: {
    produce: (n: string) => T,
    consume: (x: T) => void
}
): void { }

// Works
var a = f({
    produce: () => "hello",
    consume: x => x.toLowerCase()
});
Assert.isUndefined(a)
// Works
var b = f({
    produce: (n: string) => n,
    consume: x => x.toLowerCase(),
});
Assert.isUndefined(b)
// Was an error, now works.
var c = f({
    produce: n => n,
    consume: x => x.toLowerCase(),
});
Assert.isUndefined(c)
// Was an error, now works.
var d = f({
    produce: function () { return "hello"; },
    consume: x => x.toLowerCase(),
});
Assert.isUndefined(d)
// Was an error, now works.
var e = f({
    produce() { return "hello" },
    consume: x => x.toLowerCase(),
});
Assert.isUndefined(e)