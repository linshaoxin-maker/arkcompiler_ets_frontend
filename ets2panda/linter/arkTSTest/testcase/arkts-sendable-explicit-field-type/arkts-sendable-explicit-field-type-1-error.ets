/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { lang } from './@arkts.lang';

enum Color {Red, Green, Blue}

@Sendable
class SendableClass {
    public a: number = 1;
}

class NoSendableClass {
    public b: number = 1;
}

@Sendable
class A {
    public booleanProp: boolean = true;
    public numberProp: number = 42;
    public stringProp: string = 'Hello';
    public arrayProp: number[] = [1, 2, 3];
    public tupleProp: [string, number] = ['typescript', 4];
    public enumProp: Color = Color.Green;
    public anyProp: any = { key: 'value' };
    public voidProp: void = undefined;
    public nullProp: null = null;
    public undefinedProp: undefined = undefined;
    public objectProp: object = { name: 'John', age: 30 };
    public sendableClass: SendableClass = new SendableClass();
    public noSendableClass: NoSendableClass = new NoSendableClass();

    public optBooleanProp1?: boolean = true;
    public optNumberProp1?: number = 42;
    public optStringProp1?: string = 'Hello';
    public optArrayProp1?: number[] = [1, 2, 3];
    public optTupleProp1?: [string, number] = ['typescript', 4];
    public optEnumProp1?: Color = Color.Green;
    public optAnyProp1?: any = { key: 'value' };
    public optVoidProp1?: void = undefined;
    public optNullProp1?: null = null;
    public optUndefinedProp1?: undefined = undefined;
    public optObjectProp1?: object = { name: 'John', age: 30 };
    public optSendableClass1?: SendableClass = new SendableClass();
    public optNoSendableClass1?: NoSendableClass = new NoSendableClass();

    public optBooleanProp2: boolean = true;
    public optNumberProp2: number = 42;
    public optStringProp2: string = 'Hello';
    public optArrayProp2: number[] = [1, 2, 3];
    public optTupleProp2: [string, number] = ['typescript', 4];
    public optEnumProp2: Color = Color.Green;
    public optAnyProp2: any = { key: 'value' };
    public optVoidProp2: void = undefined;
    public optNullProp2: null = null;
    public optUndefinedProp2: undefined = undefined;
    public optObjectProp2: object = { name: 'John', age: 30 };
    public optSendableClass2: SendableClass = new SendableClass();
    public optNoSendableClass2: NoSendableClass = new NoSendableClass();

    public readonly optBooleanProp3: boolean = true;
    public readonly optNumberProp3: number = 42;
    public readonly optStringProp3: string = 'Hello';
    public readonly optArrayProp3: number[] = [1, 2, 3];
    public readonly optTupleProp3: [string, number] = ['typescript', 4];
    public readonly optEnumProp3: Color = Color.Green;
    public readonly optAnyProp3: any = { key: 'value' };
    public readonly optVoidProp3: void = undefined;
    public readonly optNullProp3: null = null;
    public readonly optUndefinedProp3: undefined = undefined;
    public readonly optObjectProp3: object = { name: 'John', age: 30 };
    public readonly optSendableClass3: SendableClass = new SendableClass();
    public readonly optNoSendableClass3: NoSendableClass = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public booleanProp4 = true;
    public numberProp4 = 42;
    public stringProp4 = 'Hello';
    public arrayProp4 = [1, 2, 3];
    public tupleProp4 = ['typescript', 4];
    public enumProp4 = Color.Green;
    public anyProp4 = { key: 'value' };
    public voidProp4 = undefined;
    public nullProp4 = null;
    public undefinedProp4 = undefined;
    public objectProp4 = { name: 'John', age: 30 };
    public sendableClass4 = new SendableClass();
    public noSendableClass4 = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public booleanProp5? = true;
    public numberProp5? = 42;
    public stringProp5? = 'Hello';
    public arrayProp5? = [1, 2, 3];
    public tupleProp5? = ['typescript', 4];
    public enumProp5? = Color.Green;
    public anyProp5? = { key: 'value' };
    public voidProp5? = undefined;
    public nullProp5? = null;
    public undefinedProp5? = undefined;
    public objectProp5? = { name: 'John', age: 30 };
    public sendableClass5? = new SendableClass();
    public noSendableClass5? = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public optBooleanProp6 = true;
    public optNumberProp6 = 42;
    public optStringProp6 = 'Hello';
    public optArrayProp6 = [1, 2, 3];
    public optTupleProp6 = ['typescript', 4];
    public optEnumProp6 = Color.Green;
    public optAnyProp6 = { key: 'value' };
    public optVoidProp6 = undefined;
    public optNullProp6 = null;
    public optUndefinedProp6 = undefined;
    public optObjectProp6 = { name: 'John', age: 30 };
    public optSendableClass6 = new SendableClass();
    public optNoSendableClass6 = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public readonly optBooleanProp7 = true;
    public readonly optNumberProp7 = 42;
    public readonly optStringProp7 = 'Hello';
    public readonly optArrayProp7 = [1, 2, 3];
    public readonly optTupleProp7 = ['typescript', 4];
    public readonly optEnumProp7 = Color.Green;
    public readonly optAnyProp7 = { key: 'value' };
    public readonly optVoidProp7 = undefined;
    public readonly optNullProp7 = null;
    public readonly optUndefinedProp7 = undefined;
    public readonly optObjectProp7 = { name: 'John', age: 30 };
    public readonly optSendableClass7 = new SendableClass();
    public readonly optNoSendableClass7 = new NoSendableClass();
}

@Sendable
class A1 extends A {
    constructor() {
        super();
    }
    public booleanProp: boolean = true;
    public numberProp: number = 42;
    public stringProp: string = 'Hello';
    public arrayProp: number[] = [1, 2, 3];
    public tupleProp: [string, number] = ['typescript', 4];
    public enumProp: Color = Color.Green;
    public anyProp: any = { key: 'value' };
    public voidProp: void = undefined;
    public nullProp: null = null;
    public undefinedProp: undefined = undefined;
    public objectProp: object = { name: 'John', age: 30 };
    public sendableClass: SendableClass = new SendableClass();
    public noSendableClass: NoSendableClass = new NoSendableClass();

    public optBooleanProp1?: boolean = true;
    public optNumberProp1?: number = 42;
    public optStringProp1?: string = 'Hello';
    public optArrayProp1?: number[] = [1, 2, 3];
    public optTupleProp1?: [string, number] = ['typescript', 4];
    public optEnumProp1?: Color = Color.Green;
    public optAnyProp1?: any = { key: 'value' };
    public optVoidProp1?: void = undefined;
    public optNullProp1?: null = null;
    public optUndefinedProp1?: undefined = undefined;
    public optObjectProp1?: object = { name: 'John', age: 30 };
    public optSendableClass1?: SendableClass = new SendableClass();
    public optNoSendableClass1?: NoSendableClass = new NoSendableClass();

    public optBooleanProp2: boolean = true;
    public optNumberProp2: number = 42;
    public optStringProp2: string = 'Hello';
    public optArrayProp2: number[] = [1, 2, 3];
    public optTupleProp2: [string, number] = ['typescript', 4];
    public optEnumProp2: Color = Color.Green;
    public optAnyProp2: any = { key: 'value' };
    public optVoidProp2: void = undefined;
    public optNullProp2: null = null;
    public optUndefinedProp2: undefined = undefined;
    public optObjectProp2: object = { name: 'John', age: 30 };
    public optSendableClass2: SendableClass = new SendableClass();
    public optNoSendableClass2: NoSendableClass = new NoSendableClass();

    public readonly optBooleanProp3: boolean = true;
    public readonly optNumberProp3: number = 42;
    public readonly optStringProp3: string = 'Hello';
    public readonly optArrayProp3: number[] = [1, 2, 3];
    public readonly optTupleProp3: [string, number] = ['typescript', 4];
    public readonly optEnumProp3: Color = Color.Green;
    public readonly optAnyProp3: any = { key: 'value' };
    public readonly optVoidProp3: void = undefined;
    public readonly optNullProp3: null = null;
    public readonly optUndefinedProp3: undefined = undefined;
    public readonly optObjectProp3: object = { name: 'John', age: 30 };
    public readonly optSendableClass3: SendableClass = new SendableClass();
    public readonly optNoSendableClass3: NoSendableClass = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public booleanProp4 = true;
    public numberProp4 = 42;
    public stringProp4 = 'Hello';
    public arrayProp4 = [1, 2, 3];
    public tupleProp4 = ['typescript', 4];
    public enumProp4 = Color.Green;
    public anyProp4 = { key: 'value' };
    public voidProp4 = undefined;
    public nullProp4 = null;
    public undefinedProp4 = undefined;
    public objectProp4 = { name: 'John', age: 30 };
    public sendableClass4 = new SendableClass();
    public noSendableClass4 = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public booleanProp5? = true;
    public numberProp5? = 42;
    public stringProp5? = 'Hello';
    public arrayProp5? = [1, 2, 3];
    public tupleProp5? = ['typescript', 4];
    public enumProp5? = Color.Green;
    public anyProp5? = { key: 'value' };
    public voidProp5? = undefined;
    public nullProp5? = null;
    public undefinedProp5? = undefined;
    public objectProp5? = { name: 'John', age: 30 };
    public sendableClass5? = new SendableClass();
    public noSendableClass5? = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public optBooleanProp6 = true;
    public optNumberProp6 = 42;
    public optStringProp6 = 'Hello';
    public optArrayProp6 = [1, 2, 3];
    public optTupleProp6 = ['typescript', 4];
    public optEnumProp6 = Color.Green;
    public optAnyProp6 = { key: 'value' };
    public optVoidProp6 = undefined;
    public optNullProp6 = null;
    public optUndefinedProp6 = undefined;
    public optObjectProp6 = { name: 'John', age: 30 };
    public optSendableClass6 = new SendableClass();
    public optNoSendableClass6 = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public readonly optBooleanProp7 = true;
    public readonly optNumberProp7 = 42;
    public readonly optStringProp7 = 'Hello';
    public readonly optArrayProp7 = [1, 2, 3];
    public readonly optTupleProp7 = ['typescript', 4];
    public readonly optEnumProp7 = Color.Green;
    public readonly optAnyProp7 = { key: 'value' };
    public readonly optVoidProp7 = undefined;
    public readonly optNullProp7 = null;
    public readonly optUndefinedProp7 = undefined;
    public readonly optObjectProp7 = { name: 'John', age: 30 };
    public readonly optSendableClass7 = new SendableClass();
    public readonly optNoSendableClass7 = new NoSendableClass();
}

@Sendable
class A2 implements lang.ISendable {
    public booleanProp: boolean = true;
    public numberProp: number = 42;
    public stringProp: string = 'Hello';
    public arrayProp: number[] = [1, 2, 3];
    public tupleProp: [string, number] = ['typescript', 4];
    public enumProp: Color = Color.Green;
    public anyProp: any = { key: 'value' };
    public voidProp: void = undefined;
    public nullProp: null = null;
    public undefinedProp: undefined = undefined;
    public objectProp: object = { name: 'John', age: 30 };
    public sendableClass: SendableClass = new SendableClass();
    public noSendableClass: NoSendableClass = new NoSendableClass();

    public optBooleanProp1?: boolean = true;
    public optNumberProp1?: number = 42;
    public optStringProp1?: string = 'Hello';
    public optArrayProp1?: number[] = [1, 2, 3];
    public optTupleProp1?: [string, number] = ['typescript', 4];
    public optEnumProp1?: Color = Color.Green;
    public optAnyProp1?: any = { key: 'value' };
    public optVoidProp1?: void = undefined;
    public optNullProp1?: null = null;
    public optUndefinedProp1?: undefined = undefined;
    public optObjectProp1?: object = { name: 'John', age: 30 };
    public optSendableClass1?: SendableClass = new SendableClass();
    public optNoSendableClass1?: NoSendableClass = new NoSendableClass();

    public optBooleanProp2: boolean = true;
    public optNumberProp2: number = 42;
    public optStringProp2: string = 'Hello';
    public optArrayProp2: number[] = [1, 2, 3];
    public optTupleProp2: [string, number] = ['typescript', 4];
    public optEnumProp2: Color = Color.Green;
    public optAnyProp2: any = { key: 'value' };
    public optVoidProp2: void = undefined;
    public optNullProp2: null = null;
    public optUndefinedProp2: undefined = undefined;
    public optObjectProp2: object = { name: 'John', age: 30 };
    public optSendableClass2: SendableClass = new SendableClass();
    public optNoSendableClass2: NoSendableClass = new NoSendableClass();

    public readonly optBooleanProp3: boolean = true;
    public readonly optNumberProp3: number = 42;
    public readonly optStringProp3: string = 'Hello';
    public readonly optArrayProp3: number[] = [1, 2, 3];
    public readonly optTupleProp3: [string, number] = ['typescript', 4];
    public readonly optEnumProp3: Color = Color.Green;
    public readonly optAnyProp3: any = { key: 'value' };
    public readonly optVoidProp3: void = undefined;
    public readonly optNullProp3: null = null;
    public readonly optUndefinedProp3: undefined = undefined;
    public readonly optObjectProp3: object = { name: 'John', age: 30 };
    public readonly optSendableClass3: SendableClass = new SendableClass();
    public readonly optNoSendableClass3: NoSendableClass = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public booleanProp4 = true;
    public numberProp4 = 42;
    public stringProp4 = 'Hello';
    public arrayProp4 = [1, 2, 3];
    public tupleProp4 = ['typescript', 4];
    public enumProp4 = Color.Green;
    public anyProp4 = { key: 'value' };
    public voidProp4 = undefined;
    public nullProp4 = null;
    public undefinedProp4 = undefined;
    public objectProp4 = { name: 'John', age: 30 };
    public sendableClass4 = new SendableClass();
    public noSendableClass4 = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public booleanProp5? = true;
    public numberProp5? = 42;
    public stringProp5? = 'Hello';
    public arrayProp5? = [1, 2, 3];
    public tupleProp5? = ['typescript', 4];
    public enumProp5? = Color.Green;
    public anyProp5? = { key: 'value' };
    public voidProp5? = undefined;
    public nullProp5? = null;
    public undefinedProp5? = undefined;
    public objectProp5? = { name: 'John', age: 30 };
    public sendableClass5? = new SendableClass();
    public noSendableClass5? = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public optBooleanProp6 = true;
    public optNumberProp6 = 42;
    public optStringProp6 = 'Hello';
    public optArrayProp6 = [1, 2, 3];
    public optTupleProp6 = ['typescript', 4];
    public optEnumProp6 = Color.Green;
    public optAnyProp6 = { key: 'value' };
    public optVoidProp6 = undefined;
    public optNullProp6 = null;
    public optUndefinedProp6 = undefined;
    public optObjectProp6 = { name: 'John', age: 30 };
    public optSendableClass6 = new SendableClass();
    public optNoSendableClass6 = new NoSendableClass();

    // Error, Field in sendable class must have type annotation (arkts-sendable-explicit-field-type)
    public readonly optBooleanProp7 = true;
    public readonly optNumberProp7 = 42;
    public readonly optStringProp7 = 'Hello';
    public readonly optArrayProp7 = [1, 2, 3];
    public readonly optTupleProp7 = ['typescript', 4];
    public readonly optEnumProp7 = Color.Green;
    public readonly optAnyProp7 = { key: 'value' };
    public readonly optVoidProp7 = undefined;
    public readonly optNullProp7 = null;
    public readonly optUndefinedProp7 = undefined;
    public readonly optObjectProp7 = { name: 'John', age: 30 };
    public readonly optSendableClass7 = new SendableClass();
    public readonly optNoSendableClass7 = new NoSendableClass();
}

class A3 {
    public booleanProp: boolean = true;
    public numberProp: number = 42;
    public stringProp: string = 'Hello';
    public arrayProp: number[] = [1, 2, 3];
    public tupleProp: [string, number] = ['typescript', 4];
    public enumProp: Color = Color.Green;
    public anyProp: any = { key: 'value' };
    public voidProp: void = undefined;
    public nullProp: null = null;
    public undefinedProp: undefined = undefined;
    public objectProp: object = { name: 'John', age: 30 };
    public sendableClass: SendableClass = new SendableClass();
    public noSendableClass: NoSendableClass = new NoSendableClass();

    public optBooleanProp1?: boolean = true;
    public optNumberProp1?: number = 42;
    public optStringProp1?: string = 'Hello';
    public optArrayProp1?: number[] = [1, 2, 3];
    public optTupleProp1?: [string, number] = ['typescript', 4];
    public optEnumProp1?: Color = Color.Green;
    public optAnyProp1?: any = { key: 'value' };
    public optVoidProp1?: void = undefined;
    public optNullProp1?: null = null;
    public optUndefinedProp1?: undefined = undefined;
    public optObjectProp1?: object = { name: 'John', age: 30 };
    public optSendableClass1?: SendableClass = new SendableClass();
    public optNoSendableClass1?: NoSendableClass = new NoSendableClass();

    public optBooleanProp2: boolean = true;
    public optNumberProp2: number = 42;
    public optStringProp2: string = 'Hello';
    public optArrayProp2: number[] = [1, 2, 3];
    public optTupleProp2: [string, number] = ['typescript', 4];
    public optEnumProp2: Color = Color.Green;
    public optAnyProp2: any = { key: 'value' };
    public optVoidProp2: void = undefined;
    public optNullProp2: null = null;
    public optUndefinedProp2: undefined = undefined;
    public optObjectProp2: object = { name: 'John', age: 30 };
    public optSendableClass2: SendableClass = new SendableClass();
    public optNoSendableClass2: NoSendableClass = new NoSendableClass();

    public readonly optBooleanProp3: boolean = true;
    public readonly optNumberProp3: number = 42;
    public readonly optStringProp3: string = 'Hello';
    public readonly optArrayProp3: number[] = [1, 2, 3];
    public readonly optTupleProp3: [string, number] = ['typescript', 4];
    public readonly optEnumProp3: Color = Color.Green;
    public readonly optAnyProp3: any = { key: 'value' };
    public readonly optVoidProp3: void = undefined;
    public readonly optNullProp3: null = null;
    public readonly optUndefinedProp3: undefined = undefined;
    public readonly optObjectProp3: object = { name: 'John', age: 30 };
    public readonly optSendableClass3: SendableClass = new SendableClass();
    public readonly optNoSendableClass3: NoSendableClass = new NoSendableClass();

    public booleanProp4 = true;
    public numberProp4 = 42;
    public stringProp4 = 'Hello';
    public arrayProp4 = [1, 2, 3];
    public tupleProp4 = ['typescript', 4];
    public enumProp4 = Color.Green;
    public anyProp4 = { key: 'value' };
    public voidProp4 = undefined;
    public nullProp4 = null;
    public undefinedProp4 = undefined;
    public objectProp4 = { name: 'John', age: 30 };
    public sendableClass4 = new SendableClass();
    public noSendableClass4 = new NoSendableClass();

    public booleanProp5? = true;
    public numberProp5? = 42;
    public stringProp5? = 'Hello';
    public arrayProp5? = [1, 2, 3];
    public tupleProp5? = ['typescript', 4];
    public enumProp5? = Color.Green;
    public anyProp5? = { key: 'value' };
    public voidProp5? = undefined;
    public nullProp5? = null;
    public undefinedProp5? = undefined;
    public objectProp5? = { name: 'John', age: 30 };
    public sendableClass5? = new SendableClass();
    public noSendableClass5? = new NoSendableClass();

    public optBooleanProp6 = true;
    public optNumberProp6 = 42;
    public optStringProp6 = 'Hello';
    public optArrayProp6 = [1, 2, 3];
    public optTupleProp6 = ['typescript', 4];
    public optEnumProp6 = Color.Green;
    public optAnyProp6 = { key: 'value' };
    public optVoidProp6 = undefined;
    public optNullProp6 = null;
    public optUndefinedProp6 = undefined;
    public optObjectProp6 = { name: 'John', age: 30 };
    public optSendableClass6 = new SendableClass();
    public optNoSendableClass6 = new NoSendableClass();

    public readonly optBooleanProp7 = true;
    public readonly optNumberProp7 = 42;
    public readonly optStringProp7 = 'Hello';
    public readonly optArrayProp7 = [1, 2, 3];
    public readonly optTupleProp7 = ['typescript', 4];
    public readonly optEnumProp7 = Color.Green;
    public readonly optAnyProp7 = { key: 'value' };
    public readonly optVoidProp7 = undefined;
    public readonly optNullProp7 = null;
    public readonly optUndefinedProp7 = undefined;
    public readonly optObjectProp7 = { name: 'John', age: 30 };
    public readonly optSendableClass7 = new SendableClass();
    public readonly optNoSendableClass7 = new NoSendableClass();
}

interface I extends lang.ISendable {
    booleanProp: boolean;
    numberProp: number;
    stringProp: string;
    arrayProp: number[];
    tupleProp: [string, number];
    enumProp: Color;
    anyProp: any;
    voidProp: void;
    nullProp: null;
    undefinedProp: undefined;
    objectProp: object;
    sendableClass: SendableClass;
    noSendableClass: NoSendableClass;

    optBooleanProp1?: boolean;
    optNumberProp1?: number;
    optStringProp1?: string;
    optArrayProp1?: number[];
    optTupleProp1?: [string, number];
    optEnumProp1?: Color;
    optAnyProp1?: any;
    optVoidProp1?: void;
    optNullProp1?: null;
    optUndefinedProp1?: undefined;
    optObjectProp1?: object;
    optSendableClass1?: SendableClass;
    optNoSendableClass1?: NoSendableClass;

    readonly optBooleanProp3: boolean;
    readonly optNumberProp3: number;
    readonly optStringProp3: string;
    readonly optArrayProp3: number[];
    readonly optTupleProp3: [string, number];
    readonly optEnumProp3: Color;
    readonly optAnyProp3: any;
    readonly optVoidProp3: void;
    readonly optNullProp3: null;
    readonly optUndefinedProp3: undefined;
    readonly optObjectProp3: object;
    readonly optSendableClass3: SendableClass;
    readonly optNoSendableClass3: NoSendableClass;
}