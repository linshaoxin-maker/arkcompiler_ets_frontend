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
    this appendix contains a summary of the grammar found in the main document. 
    typescript grammar is a superset of the grammar defined in the ECMAScript 2015 Language Specification (specifically, the ECMA-262 Standard, 6th Edition) and this appendix lists only productions that are new or modified from the ECMAScript grammar.
 ---*/


declare class ClassA {
    constructor(cname?: string, cost?: number);
    public cname: string;
    public ccost: number;
}
declare enum Code {
    stop = 0x00,
    run
}
declare type F1 = () => void;
declare type F2 = (a: string, b: string) => string;
declare type F3 = (a: number, b?: number) => string;
declare namespace AmbientNamespace {
    var an1: string;
    export var an2: string;
    interface ColorInterface {
        Red: number;
        Green: number;
        Blue: number;
    }
}
declare var varAny: any;
declare let letString: string;
declare var varBool: boolean;