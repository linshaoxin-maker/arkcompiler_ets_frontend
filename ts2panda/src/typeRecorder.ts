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

import ts from "typescript";
import {
    BaseType,
    PrimitiveType
} from "./base/typeSystem";

export class TypeRecorder {
    private static instance: TypeRecorder;
    private type2Index: Map<ts.Node, number> = new Map<ts.Node, number>();
    private typeInfo: Array<BaseType> = new Array<BaseType>();
    private variable2Type: Map<ts.Node, number> = new Map<ts.Node, number>();

    private constructor() {}

    public static getInstance() {
        return TypeRecorder.instance;
    }

    public static createInstance() {
        TypeRecorder.instance = new TypeRecorder();
    }

    public addType2Index(typeNode: ts.Node, index: number) {
        this.type2Index.set(typeNode, index);
    }

    public setVariable2Type(variableNode: ts.Node, index: number) {
        this.variable2Type.set(variableNode, index);
    }

    public hasType(typeNode: ts.Node): boolean {
        return this.type2Index.has(typeNode);
    }

    public tryGetTypeIndex(typeNode: ts.Node): number {
        if (this.type2Index.has(typeNode)) {
            return this.type2Index.get(typeNode)!;
        } else {
            return -1;
        }
    }

    public tryGetVariable2Type(variableNode: ts.Node): number {
        if (this.variable2Type.has(variableNode)) {
            return this.variable2Type.get(variableNode)!;
        } else {
            return -1;
        }
    }

    // might not needed
    public getType2Index(): Map<ts.Node, number> {
        return this.type2Index;
    }

    public getTypeInfo(): Array<BaseType> {
        return this.typeInfo;
    }

    public getVariable2Type(): Map<ts.Node, number> {
        return this.variable2Type;
    }
}