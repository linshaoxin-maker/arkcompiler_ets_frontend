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
  The && operator permits the operands to be of any type and produces a result of the same type as the second operand.
 ---*/


var a: any = 10
var b: any = 's'
var c: boolean = false
var d: boolean = true
var e: number = 20
var f: number = 15
var g: string = 'a'
var h: string = 'b'
var i: undefined = undefined
var j: undefined = undefined

var k = a && b
Assert.isString(k)
var l = c && d
Assert.isBoolean(l)
var m = e && f
Assert.isNumber(m)
var n = g && h
Assert.isString(n)
var o = i && j
Assert.isUndefined(o)
var p = a && c
Assert.isBoolean(p)
var q = a && e
Assert.isNumber(q)
var r = a && g
Assert.isString(r)
var s = a && i
Assert.isUndefined(s)
var t = c && e
Assert.isBoolean(t)
var u = c && g
Assert.isBoolean(u)
var v = c && i
Assert.isBoolean(v)
var w = e && g
Assert.isString(w)
var x = e && i
Assert.isUndefined(x)
var y = g && i
Assert.isUndefined(y)