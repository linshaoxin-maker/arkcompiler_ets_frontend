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
  The type T of a variable introduced by a simple variable declaration is determined as follows:
  If the declaration includes a type annotation, T is that type.
  Otherwise, if the declaration includes an initializer expression, T is the widened form (section 3.12) of the type of the initializer expression.
  Otherwise, T is the Any type.
 ---*/


interface Point { x: number; y: number; }

var a = { x: 0, y: 1 };
var b: Point = { x: 10, y: 11 };
var c = <Point>{ x: 100, y: 111 };
var d: { x: number; y: number; } = { x: 1000, y: 1001 };
var e = <{ x: number; y: number; }>{ x: 10000, y: 10001 };

Assert.equal(0, a.x);
Assert.equal(1, a.y);

Assert.equal(10, b.x);
Assert.equal(11, b.y);

Assert.equal(100, c.x);
Assert.equal(111, c.y);

Assert.equal(1000, d.x);
Assert.equal(1001, d.y);

Assert.equal(10000, e.x);
Assert.equal(10001, e.y);