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

import {
    BaseType,
    PrimitiveType
} from "./base/typeSystem";

export class TypeRecorder {
    private static instance: TypeRecorder;
    private type2Index: Map<number, number> = new Map<number, number>();
    private typeInfo: Array<BaseType> = new Array<BaseType>();
    private variable2Type: Map<number, number> = new Map<number, number>();

    // temp for test index
    public index: number;

    private constructor() {
        this.index = PrimitiveType._LENGTH;
    }

    public static getInstance() {
        return TypeRecorder.instance;
    }

    public static createInstance() {
        TypeRecorder.instance = new TypeRecorder();
    }

    public addType2Index(typePosition: number, index: number) {
        this.type2Index.set(typePosition, index);
    }

    public setVariable2Type(variablePos: number, index: number) {
        this.variable2Type.set(variablePos, index);
    }

    public hasType(typePosition: number): boolean {
        return this.type2Index.has(typePosition);
    }

    public tryGetTypeIndex(typePosition: number): number {
        if (this.type2Index.has(typePosition)) {
            return this.type2Index.get(typePosition)!;
        } else {
            return -1;
        }
    }

    public tryGetVariable2Type(variablePosition: number): number {
        if (this.variable2Type.has(variablePosition)) {
            return this.variable2Type.get(variablePosition)!;
        } else {
            return -1;
        }
    }

    // might not needed
    public getType2Index(): Map<number, number> {
        return this.type2Index;
    }

    public getTypeInfo(): Array<BaseType> {
        return this.typeInfo;
    }

    public getVariable2Type(): Map<number, number> {
        return this.variable2Type;
    }
}