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

export enum PremitiveType {
    UNDEFINED,
    STRING,
    NUMBER,
    BOOLEAN,
    _LENGTH
}

export class TypeRecorder {
    private type2Index: Map<string, number> = new Map<string, number>();
    private typeInfo: Array<any>;
    private index: number;

    constructor() {
        this.index = PremitiveType._LENGTH;
    }

    setType2Index(typePosition: string) {
        this.type2Index.set(typePosition, this.index);
    }

    setTypeInfo(typeContent: any) {
        this.typeInfo[this.index] = typeContent;
    }

    addType(typePosition: string, typeContent: any) {
        this.setType2Index(typePosition);
        this.setTypeInfo(typeContent);
        this.index += this.index;
    }

    getType2Index(): Map<string, number> {
        return this.type2Index;
    }

    getTypeInfo(): Array<any> {
        return this.typeInfo;
    }


}