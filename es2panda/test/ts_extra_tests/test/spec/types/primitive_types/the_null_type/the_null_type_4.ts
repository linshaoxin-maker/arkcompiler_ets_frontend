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
    The Null type is a subtype of all types, except the Undefined type. 
    This means that null is considered a valid value for all primitive types, object types, union types, intersection types, and type parameters, including even the Number and Boolean primitive types.
 error:
    code: TS2322
    message: Type 'null' is not assignable to type 'boolean'.
 module: ESNext
 isCurrent: true
 ---*/


var nullNumber: number = null;
var nullBoolean: boolean = null;
var nullString: string = null;
var nullSymbol: symbol = null;
var nullVoid: void = null;
