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


// any  
var a;

// number                           
var b: number;

// number  
var c = 30;

// { x: number; y: string; }  
var d = { x: 40, y: "hello" };

// any
var e: any = "test";
a = 1;
Assert.equal(1, a);

a = '111';
Assert.equal('111', a);

b = 20;
Assert.equal(20, b);

Assert.equal(30, c);

Assert.equal(40, d.x);
Assert.equal("hello", d.y);

var x = 50;
Assert.equal(50, x);
var x: number;
Assert.equal(50, x);
if (x == 50) {
  var x = 100;
  Assert.equal(100, x);

  x = 200;

  Assert.equal(200, x);
}

Assert.equal(200, x);