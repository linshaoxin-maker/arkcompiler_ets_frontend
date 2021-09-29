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
import { TypeChecker } from "../typeChecker";
import { TypeRecorder } from "../typeRecorder";
import { PandaGen } from "../pandagen";

export enum PrimitiveType {
    NUMBER,
    BOOLEAN,
    STRING,
    SYMBOL,
    VOID,
    NULL,
    UNDEFINED,
    _LENGTH = 50
}

export enum L2Type {
    CLASS,
    FUNCTION,
    OBJECT // object literal
}

export abstract class BaseType {

    abstract transfer2LiteralBuffer(): LiteralBuffer;
    protected typeChecker = TypeChecker.getInstance().getTypeChecker();
    protected typeRecorder = TypeRecorder.getInstance();

    protected getTypePosForIdentifier(node: ts.Node): number {
        let identifierSymbol = this.typeChecker.getTypeAtLocation(node).symbol;
        return identifierSymbol.declarations[0].pos;
    }

    protected getTypeNodeForIdentifier(node: ts.Node): ts.Node {
        let identifierSymbol = this.typeChecker.getTypeAtLocation(node).symbol;
        return identifierSymbol.declarations[0];
    }

    protected getTypeFlagsForIdentifier(node: ts.Node): string {
        let typeFlag = this.typeChecker.getTypeAtLocation(node).getFlags();
        return ts.TypeFlags[typeFlag].toUpperCase();
    }

    protected addCurrentType(node: ts.Node, index: number) {
        let typePos = node.pos;
        this.typeRecorder.addType2Index(typePos, index);
    }

    protected setVariable2Type(variablePos: number, index: number) {
        this.typeRecorder.setVariable2Type(variablePos, index);
    }

    protected createType(node: ts.Node, variablePos?: number) {
        switch (node.kind) {
            case ts.SyntaxKind.MethodDeclaration:
            case ts.SyntaxKind.Constructor:
            case ts.SyntaxKind.GetAccessor:
            case ts.SyntaxKind.SetAccessor: {
                new FunctionType(<ts.FunctionLikeDeclaration>node, variablePos);
                break;
            }
            case ts.SyntaxKind.ClassDeclaration: {
                new ClassType(<ts.ClassDeclaration>node, variablePos);
                break;
            }
        }
    }

    protected getOrCreateUserDefinedType(node: ts.Node, variablePos?: number) {
        let typePos = this.getTypePosForIdentifier(node);
        let typeIndex = this.typeRecorder.tryGetTypeIndex(typePos);
        if (typeIndex == -1) {
            let typeNode = this.getTypeNodeForIdentifier(node);
            this.createType(typeNode, variablePos);
            typeIndex = this.typeRecorder.tryGetTypeIndex(typePos);
        }
        return typeIndex;
    }

    protected getTypeIndexForDeclWithType(
        node: ts.FunctionLikeDeclaration | ts.ParameterDeclaration | ts.PropertyDeclaration, variablePos?: number): number {
        if (node.type) {
            let typeRef = node.type;
            let typeFlagName = this.getTypeFlagsForIdentifier(typeRef);
            let typeIndex = -1;
            if (typeFlagName in PrimitiveType) {
                typeIndex = PrimitiveType[typeFlagName as keyof typeof PrimitiveType];
            } else {
                let identifier = typeRef.getChildAt(0);
                typeIndex = this.getOrCreateUserDefinedType(identifier, variablePos);
            }
            // set variable if variablePos is given;
            if (variablePos) {
                this.setVariable2Type(variablePos, typeIndex);
            }
            if (typeIndex == -1) {
                console.log("ERROR: Type cannot be found for: " + node.getText());
            }
            return typeIndex!;
        }
        // console.trace();
        console.log("WARNING: node type not found for: " + node.getText());
        return -1;
    }

    protected getIndexFromTypeArrayBuffer(type: BaseType): number {
        return PandaGen.appendTypeArrayBuffer(new PlaceHolderType);
    }

    protected setTypeArrayBuffer(type: BaseType, index: number) {
        PandaGen.setTypeArrayBuffer(type, index);
    }

    // temp for test
    protected getIndex() {
        let currIndex = this.typeRecorder.index;
        this.typeRecorder.index += 1;
        return currIndex;
    }

    protected getLog(node: ts.Node, currIndex: number) {
        console.log("=========== " + node.kind);
        console.log(node.getText());
        console.log("==============================");
        console.log("+++++++++++++++++++++++++++++++++");
        console.log(PandaGen.getLiteralArrayBuffer()[currIndex]);
        console.log("+++++++++++++++++++++++++++++++++");
        console.log("type2Index: ");
        console.log(this.typeRecorder.getType2Index());
        console.log("variable2Type: ");
        console.log(this.typeRecorder.getVariable2Type());
    }
}

export class PlaceHolderType extends BaseType {
    transfer2LiteralBuffer(): LiteralBuffer {
        return new LiteralBuffer();
    }
}

export class ClassType extends BaseType {
    modifier: number = 0; // 0 -> unabstract, 1 -> abstract;
    heritages: Array<number> = new Array<number>();
    // fileds Array: [typeIndex] [static -> 1] [public -> 0, private -> 1, protected -> 2] [readonly -> 1]
    fields: Map<string, Array<number>> = new Map<string, Array<number>>();
    methods: Array<number> = new Array<number>();

    constructor(classNode: ts.ClassDeclaration, variablePos?: number) {
        super();

        let currIndex = this.getIndexFromTypeArrayBuffer(new PlaceHolderType());
        // record type before its initialization, so its index can be recorded
        // in case there's recursive reference of this type
        this.addCurrentType(classNode, currIndex);

        this.fillInModifiers(classNode);
        this.fillInHeritages(classNode);
        this.fillInFieldsAndMethods(classNode);

        // initialization finished, add variable to type if variable is given
        if (variablePos) {
            this.setVariable2Type(variablePos, currIndex);
        }
        this.setTypeArrayBuffer(this, currIndex);
        // check typeRecorder
        this.getLog(classNode, currIndex);
    }

    private fillInModifiers(node: ts.ClassDeclaration) {
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

    private fillInHeritages(node: ts.ClassDeclaration) {
        if (node.heritageClauses) {
            for (let heritage of node.heritageClauses) {
                let heritageIdentifier = heritage.getChildAt(1).getChildAt(0);
                let heritageTypePos = this.getOrCreateUserDefinedType(heritageIdentifier);
                this.heritages.push(heritageTypePos);
            }
        }
    }

    private fillInFields(member: ts.PropertyDeclaration) {
        // collect modifier info
        let fieldName = member.name.getText();
        let fieldInfo = Array<number>(0, 0, 0, 0);
        if (member.modifiers) {
            for (let modifier of member.modifiers) {
                switch (modifier.kind) {
                    case ts.SyntaxKind.StaticKeyword: {
                        fieldInfo[1] = 1;
                        break;
                    }
                    case ts.SyntaxKind.PrivateKeyword: {
                        fieldInfo[2] = 1;
                        break;
                    }
                    case ts.SyntaxKind.ProtectedKeyword: {
                        fieldInfo[2] = 2;
                    }
                    case ts.SyntaxKind.ReadonlyKeyword: {
                        fieldInfo[3] = 1;
                    }
                }
            }
        }
        // collect type info
        let variablePos = member.name ? member.name.pos : member.pos;
        fieldInfo[0] = this.getTypeIndexForDeclWithType(member, variablePos);
        this.fields.set(fieldName, fieldInfo);
    }

    private fillInFieldsAndMethods(node: ts.ClassDeclaration) {
        if (node.members) {
            for (let member of node.members) {
                switch (member.kind) {
                    case ts.SyntaxKind.MethodDeclaration:
                    case ts.SyntaxKind.Constructor:
                    case ts.SyntaxKind.GetAccessor:
                    case ts.SyntaxKind.SetAccessor: {
                        // a method like declaration in class must be a new type,
                        // add it into typeRecorder
                        let typePos = member.pos;
                        let variablePos = member.name ? member.name.pos : member.pos;
                        let typeIndex = this.typeRecorder.tryGetTypeIndex(typePos);
                        if (typeIndex == -1) {
                            this.createType(member, variablePos)
                        }
                        // Then, get the typeIndex and fill in the methods array
                        typeIndex = this.typeRecorder.tryGetTypeIndex(typePos);
                        this.methods.push(typeIndex!);
                        break;
                    }
                    case ts.SyntaxKind.PropertyDeclaration: {
                        this.fillInFields(<ts.PropertyDeclaration>member);
                        break;
                    }
                }
            }
        }
    }

    transfer2LiteralBuffer() {
        let classTypeBuf = new LiteralBuffer();
        let classTypeLiterals: Array<Literal> = new Array<Literal>();
        // the first element is to determine the L2 type
        classTypeLiterals.push(new Literal(LiteralTag.INTEGER, L2Type.CLASS));
        classTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.modifier));

        classTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.heritages.length)); // num of heritages are recorded
        this.heritages.forEach(heritage => {
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, heritage));
        });

        classTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.fields.size)); // num of fields are recorded
        this.fields.forEach((typeInfo, name) => {
            classTypeLiterals.push(new Literal(LiteralTag.STRING, name));
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, typeInfo[0])); // typeIndex
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, typeInfo[1])); // static
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, typeInfo[2])); // accessFlag
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, typeInfo[3])); // readonly
        });

        classTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.methods.length)); // num of fields are recorded
        this.methods.forEach(method => {
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, method));
        });

        classTypeBuf.addLiterals(...classTypeLiterals);
        return classTypeBuf;
    }

}

export class FunctionType extends BaseType {
    name: string | undefined = '';
    accessFlag: number = 0; // 0 -> public -> 0, private -> 1, protected -> 2
    modifier: number = 0; // 0 -> unstatic, 1 -> static
    parameters: Array<number> = new Array<number>();
    returnType: number = 0;

    constructor(funcNode: ts.FunctionLikeDeclaration, variablePos?: number) {
        super();

        let currIndex = this.getIndexFromTypeArrayBuffer(new PlaceHolderType());
        // record type before its initialization, so its index can be recorded
        // in case there's recursive reference of this type
        this.addCurrentType(funcNode, currIndex);

        this.name = funcNode.name ?.getText();
        this.fillInModifiers(funcNode);
        this.fillInParameters(funcNode);
        this.fillInReturn(funcNode);

        // initialization finished, add variable to type if variable is given
        if (variablePos) {
            this.setVariable2Type(variablePos, currIndex);
        }
        this.setTypeArrayBuffer(this, currIndex);

        // check typeRecorder
        this.getLog(funcNode, currIndex);
    }

    private fillInModifiers(node: ts.FunctionLikeDeclaration) {
        if (node.modifiers) {
            for (let modifier of node.modifiers) {
                switch (modifier.kind) {
                    case ts.SyntaxKind.PrivateKeyword: {
                        this.accessFlag = 1;
                        break;
                    }
                    case ts.SyntaxKind.ProtectedKeyword: {
                        this.accessFlag = 2;
                        break;
                    }
                    case ts.SyntaxKind.StaticKeyword: {
                        this.modifier = 1;
                    }
                }
            }
        }
    }

    private fillInParameters(node: ts.FunctionLikeDeclaration) {
        if (node.parameters) {
            for (let parameter of node.parameters) {
                let variableName = parameter.pos;
                let typeIndex = this.getTypeIndexForDeclWithType(parameter, variableName);
                this.parameters.push(typeIndex);
            }
        }
    }

    private fillInReturn(node: ts.FunctionLikeDeclaration) {
        let typeIndex = this.getTypeIndexForDeclWithType(node);
        if (typeIndex != -1) {
            this.returnType = typeIndex;
        }
    }

    transfer2LiteralBuffer(): LiteralBuffer {
        let funcTypeBuf = new LiteralBuffer();
        let funcTypeLiterals: Array<Literal> = new Array<Literal>();
        funcTypeLiterals.push(new Literal(LiteralTag.INTEGER, L2Type.FUNCTION));
        funcTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.accessFlag));
        funcTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.modifier));
        funcTypeLiterals.push(new Literal(LiteralTag.STRING, this.name));

        funcTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.parameters.length));
        this.parameters.forEach((type) => {
            funcTypeLiterals.push(new Literal(LiteralTag.INTEGER, type));
        });

        funcTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.returnType));
        funcTypeBuf.addLiterals(...funcTypeLiterals);
        return funcTypeBuf;
    }
}

export class ObjectLiteralType extends BaseType {
    private properties: Map<string, number> = new Map<string, number>();
    private methods: Array<number> = new Array<number>();

    constructor(obj: ts.ObjectLiteralExpression) {
        super();

        // TODO extract object info here
    }

    transfer2LiteralBuffer(): LiteralBuffer {
        let objTypeBuf = new LiteralBuffer();

        return objTypeBuf;
    }
}

export class TypeOfVreg {
    private vregNum: number;
    private typeIndex: number;

    constructor(vregNum: number, typeIndex: number) {
        this.vregNum = vregNum;
        this.typeIndex = typeIndex;
    }
}
