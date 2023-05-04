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
 description: the Number primitive type behaves as an object type with the same properties as the global interface type 'Number'.
 ---*/


// Same as z: number = 123.456
var z = 123.456;
// Property of Number interface
var s = z.toFixed(2);
Assert.equal(s, 123.46);
var a = z.toString();
Assert.equal(a, "123.456");
var b = z.toExponential();
Assert.equal(b, "1.23456e+2");
b = z.toExponential(2);
Assert.equal(b, "1.23e+2");
var c = z.toPrecision();
Assert.equal(c, "123.456");
c = z.toPrecision(3);
Assert.equal(c, "123");
var d = z.valueOf();
Assert.equal(d, 123.456);