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
import * as jshelpers from "../jshelpers";

export enum PrimitiveType {
    ANY,
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
        if (identifierSymbol && identifierSymbol.declarations) {
            return identifierSymbol.declarations[0].pos;
        }
        return -1;
    }

    protected getTypeNodeForIdentifier(node: ts.Node): ts.Node {
        let identifierSymbol = this.typeChecker.getTypeAtLocation(node).symbol;
        if (identifierSymbol && identifierSymbol.declarations) {
            return identifierSymbol!.declarations[0];
        }
        return node;
    }

    protected getTypeFlagsForIdentifier(node: ts.Node): string {
        let typeFlag = this.typeChecker.getTypeAtLocation(node).getFlags();
        return ts.TypeFlags[typeFlag].toUpperCase();
    }

    protected addCurrentType(node: ts.Node, index: number) {
        this.typeRecorder.addType2Index(node, index);
    }

    protected setVariable2Type(variableNode: ts.Node, index: number) {
        this.typeRecorder.setVariable2Type(variableNode, index);
    }

    protected createType(node: ts.Node, variableNode?: ts.Node) {
        switch (node.kind) {
            case ts.SyntaxKind.MethodDeclaration:
            case ts.SyntaxKind.Constructor:
            case ts.SyntaxKind.GetAccessor:
            case ts.SyntaxKind.SetAccessor: {
                new FunctionType(<ts.FunctionLikeDeclaration>node, variableNode);
                break;
            }
            case ts.SyntaxKind.ClassDeclaration: {
                new ClassType(<ts.ClassDeclaration>node, variableNode);
                break;
            }
            // create other type as project goes on;
            default:
                throw new Error("Currently this type is not supported");
        }
    }

    protected getOrCreateUserDefinedType(node: ts.Node, variableNode?: ts.Node) {
        let typeNode = this.getTypeNodeForIdentifier(node);
        let typeIndex = this.typeRecorder.tryGetTypeIndex(typeNode);
        if (typeIndex == -1) {
            let typeNode = this.getTypeNodeForIdentifier(node);
            this.createType(typeNode, variableNode);
            typeIndex = this.typeRecorder.tryGetTypeIndex(typeNode);
        }
        return typeIndex;
    }

    protected getTypeIndexForDeclWithType(
        node: ts.FunctionLikeDeclaration | ts.ParameterDeclaration | ts.PropertyDeclaration, variableNode?: ts.Node): number {
        if (node.type) {
            // get typeFlag to check if its a primitive type
            let typeRef = node.type;
            let typeFlagName = this.getTypeFlagsForIdentifier(typeRef);
            let typeIndex = -1;
            if (typeFlagName in PrimitiveType) {
                typeIndex = PrimitiveType[typeFlagName as keyof typeof PrimitiveType];
            } else {
                let identifier = typeRef.getChildAt(0);
                typeIndex = this.getOrCreateUserDefinedType(identifier, variableNode);
            }
            // set variable if variable node is given;
            if (variableNode) {
                this.setVariable2Type(variableNode, typeIndex);
            }
            if (typeIndex == -1) {
                console.log("ERROR: Type cannot be found for: " + jshelpers.getTextOfNode(node));
            }
            return typeIndex!;
        }
        console.log("WARNING: node type not found for: " + jshelpers.getTextOfNode(node));
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

    protected printMap(map: Map<ts.Node, number>) {
        map.forEach((value, key) => {
            console.log(jshelpers.getTextOfNode(key) + ": " + value);
        });
    }

    protected getLog(node: ts.Node, currIndex: number) {
        console.log("=========== NodeKind ===========: " + node.kind);
        console.log(jshelpers.getTextOfNode(node));
        console.log("=========== currIndex ===========: ", currIndex);
        console.log(PandaGen.getLiteralArrayBuffer()[currIndex]);
        console.log("==============================");
        console.log("type2Index: ");
        console.log(this.printMap(this.typeRecorder.getType2Index()));
        console.log("variable2Type: ");
        console.log(this.printMap(this.typeRecorder.getVariable2Type()));
        console.log("==============================");
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
    // fileds Array: [typeIndex] [public -> 0, private -> 1, protected -> 2] [readonly -> 1]
    staticFields: Map<string, Array<number>> = new Map<string, Array<number>>();
    staticMethods: Array<number> = new Array<number>();
    fields: Map<string, Array<number>> = new Map<string, Array<number>>();
    methods: Array<number> = new Array<number>();

    constructor(classNode: ts.ClassDeclaration, variableNode?: ts.Node) {
        super();

        let currIndex = this.getIndexFromTypeArrayBuffer(new PlaceHolderType());
        // record type before its initialization, so its index can be recorded
        // in case there's recursive reference of this type
        this.addCurrentType(classNode, currIndex);

        this.fillInModifiers(classNode);
        this.fillInHeritages(classNode);
        this.fillInFieldsAndMethods(classNode);

        // initialization finished, add variable to type if variable is given
        if (variableNode) {
            this.setVariable2Type(variableNode, currIndex);
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
        let fieldName: string = "";
        switch (member.name.kind) {
            case ts.SyntaxKind.Identifier:
            case ts.SyntaxKind.StringLiteral:
            case ts.SyntaxKind.NumericLiteral:
                fieldName = jshelpers.getTextOfIdentifierOrLiteral(member.name);
                break;
            case ts.SyntaxKind.ComputedPropertyName:
                fieldName = "#computed";
                break;
            default:
                throw new Error("Invalid proerty name");
        }

        let fieldInfo = Array<number>(0, 0, 0);
        let isStatic: boolean = false;
        if (member.modifiers) {
            for (let modifier of member.modifiers) {
                switch (modifier.kind) {
                    case ts.SyntaxKind.StaticKeyword: {
                        isStatic = true;
                        break;
                    }
                    case ts.SyntaxKind.PrivateKeyword: {
                        fieldInfo[1] = 1;
                        break;
                    }
                    case ts.SyntaxKind.ProtectedKeyword: {
                        fieldInfo[1] = 2;
                        break;
                    }
                    case ts.SyntaxKind.ReadonlyKeyword: {
                        fieldInfo[2] = 1;
                        break;
                    }
                }
            }
        }
        // collect type info
        let variableNode = member.name ? member.name : undefined;
        fieldInfo[0] = this.getTypeIndexForDeclWithType(member, variableNode);

        if (isStatic) {
            this.staticFields.set(fieldName, fieldInfo);
        } else {
            this.fields.set(fieldName, fieldInfo);
        }
    }

    private fillInFieldsAndMethods(node: ts.ClassDeclaration) {
        if (node.members) {
            for (let member of node.members) {
                switch (member.kind) {
                    case ts.SyntaxKind.MethodDeclaration:
                    case ts.SyntaxKind.Constructor:
                    case ts.SyntaxKind.GetAccessor:
                    case ts.SyntaxKind.SetAccessor: {
                        /**
                         * a method like declaration in class must be a new type,
                         * add it into typeRecorder
                         */
                        let variableNode = member.name ? member.name : undefined;
                        let typeIndex = this.typeRecorder.tryGetTypeIndex(member);
                        if (typeIndex == -1) {
                            this.createType(member, variableNode);
                        }
                        // Then, get the typeIndex and fill in the methods array
                        typeIndex = this.typeRecorder.tryGetTypeIndex(member);
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

        classTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.heritages.length));
        this.heritages.forEach(heritage => {
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, heritage));
        });

        // record static methods and fields;
        this.transferFields2Literal(classTypeLiterals, true);

        // TODO record static methods here

        // record unstatic fields and methods
        this.transferFields2Literal(classTypeLiterals, false);

        classTypeLiterals.push(new Literal(LiteralTag.INTEGER, this.methods.length));
        this.methods.forEach(method => {
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, method));
        });

        classTypeBuf.addLiterals(...classTypeLiterals);
        return classTypeBuf;
    }

    private transferFields2Literal(classTypeLiterals: Array<Literal>, isStatic: boolean) {
        let transferredTarget: Map<string, Array<number>> = isStatic ? this.staticFields : this.fields;

        classTypeLiterals.push(new Literal(LiteralTag.INTEGER, transferredTarget.size));
        transferredTarget.forEach((typeInfo, name) => {
            classTypeLiterals.push(new Literal(LiteralTag.STRING, name));
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, typeInfo[0])); // typeIndex
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, typeInfo[1])); // accessFlag
            classTypeLiterals.push(new Literal(LiteralTag.INTEGER, typeInfo[2])); // readonly
        });
    }
}

export class FunctionType extends BaseType {
    name: string | undefined = '';
    accessFlag: number = 0; // 0 -> public -> 0, private -> 1, protected -> 2
    modifier: number = 0; // 0 -> unstatic, 1 -> static
    parameters: Array<number> = new Array<number>();
    returnType: number = 0;

    constructor(funcNode: ts.FunctionLikeDeclaration, variableNode?: ts.Node) {
        super();

        let currIndex = this.getIndexFromTypeArrayBuffer(new PlaceHolderType());
        // record type before its initialization, so its index can be recorded
        // in case there's recursive reference of this type
        this.addCurrentType(funcNode, currIndex);

        if (funcNode.name) {
            this.name = jshelpers.getTextOfIdentifierOrLiteral(funcNode.name);
        } else {
            this.name = "constructor";
        }
        this.fillInModifiers(funcNode);
        this.fillInParameters(funcNode);
        this.fillInReturn(funcNode);

        // initialization finished, add variable to type if variable is given
        if (variableNode) {
            this.setVariable2Type(variableNode, currIndex);
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
                let variableNode = parameter;
                let typeIndex = this.getTypeIndexForDeclWithType(parameter, variableNode);
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
