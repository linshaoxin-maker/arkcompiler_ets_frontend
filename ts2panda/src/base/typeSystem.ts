/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

import * as ts from "typescript";

export enum PremitiveType {
    UNDEFINED,
    STRING,
    NUMBER,
    BOOLEAN,
    _LENGTH
}

export abstract class BaseType {

}

export class ClassType extends BaseType {
    modifier: number = 0; // 0 -> unabstract, 1 -> abstract;
    heritages: Array<number> = new Array<number>();
    fields : Map<string, Array<number>> = new Map<string, Array<number>>(); // Array: [type][static][public/private]
    Methods: Array<number> = new Array<number>();

    constructor(classNode: ts.ClassDeclaration) {
        super();

        if (classNode.AbstractKeyword) {
            this.modifier = 1;
        }

        // pls extract class info here;
    }

    extractFields() {

    }

}

export class FunctionType extends BaseType {
    modifierPublic: number = 0;
    modifierStatic: number = 0; // 0 -> normal function or method, 1 -> static member of class
    name: string = '';
    parameters: Map<string, number> = new Map<string, number>();
    returnType: number = 0;

    constructor(funcNode: ts.FunctionLikeDeclaration) {
        super();

        if (funcNode.AbstractKeyword) {
            this.modifier = 1;
        }

        // pls extract function info here
    }

}