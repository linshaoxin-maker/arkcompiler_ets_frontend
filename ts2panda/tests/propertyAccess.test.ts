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
    expect
} from 'chai';
import 'mocha';
import {
    CreateObjectWithBuffer,
    DefineMethod,
    DefineGetterSetterByValue,
    Imm,
    LdaDyn,
    LdaiDyn,
    LdaStr,
    LdObjByName,
    MovDyn,
    ResultType,
    StaDyn,
    StObjByName,
    VReg
} from "../src/irnodes";
import { checkInstructions, compileAllSnippet, compileMainSnippet } from "./utils/base";

describe("PropertyAccess", function() {
    it('get obj.property', function() {
        let insns = compileMainSnippet(`let obj;
                                obj.property;`);

        let objReg = new VReg();

        let expected = [
            new LdaDyn(objReg),
            new StaDyn(objReg),
            new LdObjByName("property", objReg)
        ];

        insns = insns.slice(2, insns.length - 1); // cut off let obj and return.dyn
        expect(checkInstructions(insns, expected)).to.be.true;
    });

    it('set obj.property', function() {
        let insns = compileMainSnippet(`let obj;
                                obj.property = 0;`);
        let objReg = new VReg();
        let tempObj = new VReg();

        let expected = [
            new LdaDyn(objReg),
            new StaDyn(tempObj),
            new MovDyn(objReg, tempObj),
            new LdaiDyn(new Imm(ResultType.Int, 0)),
            new StObjByName("property", objReg),
        ];

        insns = insns.slice(2, insns.length - 1); // cut off let obj and return.dyn
        expect(checkInstructions(insns, expected)).to.be.true;
    });

    it('SetAccessor', function() {
        let compilerunit = compileAllSnippet(`
            let obj = { 
                set myMethod (arg) { 
                    this.a = arg; 
                } 
            }`);

        let objInstance = new VReg();
        let funcReg = new VReg();
        let propReg = new VReg();

        let expected = [
            new CreateObjectWithBuffer(new Imm(ResultType.Int, 0)),
            new StaDyn(objInstance),
            new LdaDyn(new VReg()),
            new DefineMethod("func_myMethod_1", new VReg()),
            new StaDyn(funcReg),
            new LdaStr("myMethod"),
            new StaDyn(propReg),
            new LdaDyn(new VReg()),
            new DefineGetterSetterByValue(objInstance, propReg, new VReg(), funcReg),
        ];

        compilerunit.forEach(element => {
            if (element.internalName == "func_main_0") {
                let insns = element.getInsns();

                insns = insns.slice(0, insns.length - 3);
                expect(checkInstructions(insns, expected)).to.be.true;
            }

            if (element.internalName == "func_myMethod_1") {
                let parameterLength = element.getParameterLength();
                expect(parameterLength == 1).to.be.true;
            }
        });
    });

    it('GetAccessor', function() {
        let compilerunit = compileAllSnippet(`
            let obj = { 
                get a() { return 'a'; }; 
            }`);

        let objInstance = new VReg();
        let funcReg = new VReg();
        let propReg = new VReg();

        let expected = [
            new CreateObjectWithBuffer(new Imm(ResultType.Int, 0)),
            new StaDyn(objInstance),
            new LdaDyn(new VReg()),
            new DefineMethod("func_a_1", new VReg()),
            new StaDyn(funcReg),
            new LdaStr("a"),
            new StaDyn(propReg),
            new LdaDyn(new VReg()),
            new DefineGetterSetterByValue(objInstance, propReg, funcReg, new VReg()),
        ];

        compilerunit.forEach(element => {
            if (element.internalName == "func_main_0") {
                let insns = element.getInsns();

                insns = insns.slice(0, insns.length - 3);
                expect(checkInstructions(insns, expected)).to.be.true;
            }
        });
    });

    it('GetAccessor&SetAccessor', function() {
        let compilerunit = compileAllSnippet(`let obj = { 
            get a() { return 'a'; }, 
            set a(x) {}
        }`);

        let objInstance = new VReg();
        let getterReg = new VReg();
        let setterReg = new VReg();
        let propReg = new VReg();

        let expected = [
            new CreateObjectWithBuffer(new Imm(ResultType.Int, 0)),
            new StaDyn(objInstance),
            new LdaDyn(new VReg()),
            new DefineMethod("func_a_1", new VReg()),
            new StaDyn(getterReg),
            new LdaDyn(new VReg()),
            new DefineMethod("func_a_2", new VReg()),
            new StaDyn(setterReg),
            new LdaStr("a"),
            new StaDyn(propReg),
            new LdaDyn(new VReg()),
            new DefineGetterSetterByValue(objInstance, propReg, getterReg, setterReg),
        ];

        compilerunit.forEach(element => {
            if (element.internalName == "func_main_0") {
                let insns = element.getInsns();

                insns = insns.slice(0, insns.length - 3);
                expect(checkInstructions(insns, expected)).to.be.true;
            }
        });
    });
});
