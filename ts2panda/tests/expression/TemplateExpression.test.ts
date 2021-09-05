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
    Add2Dyn,
    CalliThisRangeDyn,
    GetTemplateObject,
    Imm,
    IRNode,
    LdaDyn,
    LdaiDyn,
    LdaStr,
    LdObjByName,
    MovDyn,
    ResultType,
    ReturnUndefined,
    StaDyn,
    StObjByValue,
    TryLdGlobalByName,
    CreateEmptyArray,
    VReg
} from "../../src/irnodes";
import { checkInstructions, compileMainSnippet } from "../utils/base";

function MicroCreateAddInsns(leftVal: number, rightVal: number): IRNode[] {
    let insns = [];
    let lhs = new VReg();

    insns.push(new LdaiDyn(new Imm(ResultType.Int, leftVal)));
    insns.push(new StaDyn(lhs));
    insns.push(new LdaiDyn(new Imm(ResultType.Int, rightVal)));
    insns.push(new Add2Dyn(lhs));

    return insns;
}

function MicroCreateObjAndPropInsns(): IRNode[] {
    let insns = [];
    let obj = new VReg();
    let val = new VReg();

    insns.push(new TryLdGlobalByName("String"));
    insns.push(new StaDyn(obj));
    insns.push(new LdObjByName("raw", obj));
    insns.push(new StaDyn(val));

    return insns;
}

function MicroGetTemplateObject(rawArr: VReg, cookedArr: VReg): IRNode[] {
    let insns = [];
    let objReg = new VReg();
    let indexReg = new VReg();

    insns.push(new CreateEmptyArray());
    insns.push(new StaDyn(objReg));

    insns.push(new LdaiDyn(new Imm(ResultType.Int, 0)));
    insns.push(new StaDyn(indexReg));
    insns.push(new LdaDyn(rawArr));
    insns.push(new StObjByValue(objReg,indexReg));
    insns.push(new LdaiDyn(new Imm(ResultType.Int, 1)));
    insns.push(new StaDyn(indexReg));
    insns.push(new LdaDyn(cookedArr));
    insns.push(new StObjByValue(objReg,indexReg));
    insns.push(new GetTemplateObject(objReg));
    return insns;

}

describe("templateExpressionTest", function() {
    it("`string text line 1`", function() {
        let insns = compileMainSnippet("`string text line 1`;");
        let expected = [
            new LdaStr("string text line 1"),
            new ReturnUndefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
    });

    it("`Fifteen is ${5 + 10}`", function() {
        let insns = compileMainSnippet("`Fifteen is ${5 + 10}`");
        let headVal = new VReg();

        let expected = [
            new LdaStr("Fifteen is "),
            new StaDyn(headVal),
            ...MicroCreateAddInsns(5, 10),
            new Add2Dyn(headVal),
            new ReturnUndefined()
        ]
        expect(checkInstructions(insns, expected)).to.be.true;
    });

    it("String.raw`string text line 1`", function() {
        let insns = compileMainSnippet("String.raw`string text line 1`;");
        let obj = new VReg();
        let prop = new VReg();
        let elemIdxReg = new VReg();
        let rawArr = new VReg();
        let cookedArr = new VReg();
        let rawArr1 = new VReg();
        let cookedArr1 = new VReg();
        let templateObj = new VReg();

        let expected = [

            ...MicroCreateObjAndPropInsns(),
            new CreateEmptyArray(),
            new StaDyn(rawArr),
            new CreateEmptyArray(),
            new StaDyn(cookedArr),

            new LdaiDyn(new Imm(ResultType.Int, 0)),
            new StaDyn(elemIdxReg),
            new LdaStr("string text line 1"),
            new StObjByValue(rawArr, elemIdxReg),

            new LdaStr("string text line 1"),
            new StObjByValue(cookedArr, elemIdxReg),
            new MovDyn(rawArr1,rawArr),
            new MovDyn(cookedArr1,cookedArr),

            ...MicroGetTemplateObject(rawArr1, cookedArr),
            new StaDyn(templateObj),

            // structure call 
            new CalliThisRangeDyn(new Imm(ResultType.Int, 2), [prop, obj, templateObj]),

            new ReturnUndefined()
        ];

        expect(checkInstructions(insns, expected)).to.be.true;
    });

    it("String.raw`string text line 1\\nstring text line 2`", function() {
        let insns = compileMainSnippet("String.raw`string text line 1\\nstring text line 2`;");
        let obj = new VReg();
        let prop = new VReg();
        let elemIdxReg = new VReg();
        let rawArr = new VReg();
        let cookedArr = new VReg();
        let rawArr1 = new VReg();
        let cookedArr1 = new VReg();
        let templateObj = new VReg();

        let expected = [

            ...MicroCreateObjAndPropInsns(),
            new CreateEmptyArray(),
            new StaDyn(rawArr),
            new CreateEmptyArray(),
            new StaDyn(cookedArr),

            new LdaiDyn(new Imm(ResultType.Int, 0)),
            new StaDyn(elemIdxReg),
            new LdaStr("string text line 1\\nstring text line 2"),
            new StObjByValue(rawArr, elemIdxReg),

            new LdaStr("string text line 1\nstring text line 2"),
            new StObjByValue(cookedArr, elemIdxReg),
            new MovDyn(rawArr1,rawArr),
            new MovDyn(cookedArr1,cookedArr),

            ...MicroGetTemplateObject(rawArr1, cookedArr1),
            new StaDyn(templateObj),

            // structure call 
            new CalliThisRangeDyn(new Imm(ResultType.Int, 2), [prop, obj, templateObj]),

            new ReturnUndefined()
        ];

        expect(checkInstructions(insns, expected)).to.be.true;
    });

    it("String.raw`Fifteen is ${5 + 10} !!`", function() {
        let insns = compileMainSnippet("String.raw`Fifteen is ${5 + 10} !!`");
        let obj = new VReg();
        let prop = new VReg();
        let elemIdxReg = new VReg();
        let rawArr = new VReg();
        let cookedArr = new VReg();
        let rawArr1 = new VReg();
        let cookedArr1 = new VReg();
        let addRet = new VReg();
        let templateObj = new VReg();

        let expected = [

            ...MicroCreateObjAndPropInsns(),
            new CreateEmptyArray(),
            new StaDyn(rawArr),
            new CreateEmptyArray(),
            new StaDyn(cookedArr),

            new LdaiDyn(new Imm(ResultType.Int, 0)),
            new StaDyn(elemIdxReg),
            new LdaStr("Fifteen is "),
            new StObjByValue(rawArr, elemIdxReg),
            new LdaStr("Fifteen is "),
            new StObjByValue(cookedArr, elemIdxReg),
            new LdaiDyn(new Imm(ResultType.Int, 1)),
            new StaDyn(elemIdxReg),
            new LdaStr(" !!"),
            new StObjByValue(rawArr, elemIdxReg),
            new LdaStr(" !!"),
            new StObjByValue(cookedArr, elemIdxReg),
            new MovDyn(rawArr1,rawArr),
            new MovDyn(cookedArr1,cookedArr),

            ...MicroGetTemplateObject(rawArr1, cookedArr1),
            new StaDyn(templateObj),

            ...MicroCreateAddInsns(5, 10),
            new StaDyn(addRet),

            // structure call
            new CalliThisRangeDyn(new Imm(ResultType.Int, 3), [prop, obj, rawArr, templateObj]),
            new ReturnUndefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
    });

    it("String.raw`Fifteen is ${5 + 10} !!\\n Is not ${15 + 10} !!!`", function() {
        let insns = compileMainSnippet("String.raw`Fifteen is ${5 + 10} !!\\n Is not ${15 + 10} !!!\\n`");
        let obj = new VReg();
        let val = new VReg();
        let prop = new VReg();
        let elemIdxReg = new VReg();
        let rawArr = new VReg();
        let cookedArr = new VReg();
        let rawArr1 = new VReg();
        let cookedArr1 = new VReg();
        let addRet1 = new VReg();
        let addRet2 = new VReg();
        let templateObj = new VReg();

        let expected = [

            ...MicroCreateObjAndPropInsns(),
            new CreateEmptyArray(),
            new StaDyn(rawArr),
            new CreateEmptyArray(),
            new StaDyn(cookedArr),

            new LdaiDyn(new Imm(ResultType.Int, 0)),
            new StaDyn(elemIdxReg),
            new LdaStr("Fifteen is "),
            new StObjByValue(rawArr, elemIdxReg),
            new LdaStr("Fifteen is "),
            new StObjByValue(cookedArr, elemIdxReg),
            new LdaiDyn(new Imm(ResultType.Int, 1)),
            new StaDyn(elemIdxReg),
            new LdaStr(" !!\\n Is not "),
            new StObjByValue(rawArr, elemIdxReg),
            new LdaStr(" !!\n Is not "),
            new StObjByValue(cookedArr, elemIdxReg),
            new LdaiDyn(new Imm(ResultType.Int, 2)),
            new StaDyn(elemIdxReg),
            new LdaStr(" !!!\\n"),
            new StObjByValue(rawArr, elemIdxReg),
            new LdaStr(" !!!\n"),
            new StObjByValue(cookedArr, elemIdxReg),
            new MovDyn(rawArr1,rawArr),
            new MovDyn(cookedArr1,cookedArr),
            
            ...MicroGetTemplateObject(rawArr1, cookedArr1),
            new StaDyn(templateObj),

            ...MicroCreateAddInsns(5, 10),
            new StaDyn(addRet1),
            ...MicroCreateAddInsns(15, 10),
            new StaDyn(addRet2),

            // structure call
            new CalliThisRangeDyn(new Imm(ResultType.Int, 4), [prop, obj, rawArr, cookedArr, templateObj]),
            new ReturnUndefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
    });
});