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
import {
    Literal,
    LiteralBuffer,
    LiteralTag
} from "./literal";

export enum PremitiveType {
    UNDEFINED,
    STRING,
    NUMBER,
    BOOLEAN,
    _LENGTH
}

export abstract class BaseType {
    abstract transfer2LiteralBuffer(): LiteralBuffer;
}

export class ClassType extends BaseType {
    modifier: number = 0; // 0 -> unabstract, 1 -> abstract;
    heritages: Array<number> = new Array<number>();
    fields : Map<string, Array<number>> = new Map<string, Array<number>>(); // Array: [type][static][public/private]
    methods: Array<number> = new Array<number>();

    constructor(classNode: ts.ClassDeclaration) {
        super();
        this.fillInModifiers(classNode);
        this.fillInHeritages(classNode);
        this.fillInFields(classNode);
        this.fillInMethods(classNode);
    }

    fillInModifiers(node: ts.ClassDeclaration) {
        if (node.modifiers) {
            for (let modifier of node.modifiers) {
                switch (modifier.kind) {
                    case ts.SyntaxKind.AbstractKeyword: {
                        this.modifier = 1;
                        break;
                    }
                    case ts.SyntaxKind.ExportKeyword: {
                        break;
                    }
                }
            }
        }
    }

    fillInHeritages(node: ts.ClassDeclaration) {
        if (node.heritageClauses) {

        }
    }

    fillInFields(node: ts.ClassDeclaration) {

    }

    fillInMethods(node: ts.ClassDeclaration) {

    }

    transfer2LiteralBuffer() {
        let classTypeBuf = new LiteralBuffer();
        let classTypeLiterals: Array<Literal> = new Array<Literal>();

        classTypeLiterals.push(new Literal(LiteralTag.MODIFIER, this.modifier)); // modifier is added at the front

        classTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.heritages.length)); // num of heritages are recorded
        this.heritages.forEach(heritage => {
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, heritage));
        });

        classTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.fields.size)); // num of fields are recorded
        this.fields.forEach((typeInfo, name) => {
            classTypeLiterals.push(new Literal(LiteralTag.STRING, name));
            classTypeLiterals.push(new Literal(LiteralTag.MODIFIER, typeInfo[0]));
            classTypeLiterals.push(new Literal(LiteralTag.ACCESSFLAG, typeInfo[1]));
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, typeInfo[2]));
        });

        classTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.methods.length)); // num of fields are recorded
        this.fields.forEach(method => {
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, method));
        });

        classTypeBuf.addLiterals(...classTypeLiterals);
        return classTypeBuf;
    }

}

export class FunctionType extends BaseType {
    accessFlag: number = 0; // 0 -> private, 1 -> public
    modifier: number = 0; // 0 -> unstatic, 1 -> static
    name: string = '';
    parameters: Map<string, number> = new Map<string, number>();
    returnType: number = 0;

    constructor(funcNode: ts.FunctionLikeDeclaration) {
        super();

        // if (funcNode.AbstractKeyword) {
        //     this.modifier = 1;
        // }

        // pls extract function info here
    }

    transfer2LiteralBuffer() : LiteralBuffer {
        let funcTypeBuf = new LiteralBuffer();
        let funcTypeLiterals: Array<Literal> = new Array<Literal>();
        funcTypeLiterals.push(new Literal(LiteralTag.ACCESSFLAG, this.accessFlag));
        funcTypeLiterals.push(new Literal(LiteralTag.MODIFIER, this.modifier));
        funcTypeLiterals.push(new Literal(LiteralTag.STRING, this.name));

        funcTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.parameters.size));
        this.parameters.forEach((type, paramName) => {
            funcTypeLiterals.push(new Literal(LiteralTag.STRING, paramName));
            funcTypeLiterals.push(new Literal(LiteralTag.INTEGER, type));
        });

        funcTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.returnType));

        funcTypeBuf.addLiterals(...funcTypeLiterals);
        return funcTypeBuf;
    }

}