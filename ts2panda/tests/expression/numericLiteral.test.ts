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

import { expect } from 'chai';
import * as ts from "typescript";
import { compileNumericLiteral } from "../../src/expression/numericLiteral";
import {
    FldaiDyn,
    Imm,
    LdaDyn,
    LdaiDyn,
    ResultType,
    VReg
} from "../../src/irnodes";
import { PandaGen } from "../../src/pandagen";
import { checkInstructions } from "../utils/base";

describe("compileNumericLiteral", function() {
    it("NaN", function() {
        let pandaGen = new PandaGen("ignored", 0, undefined);
        let node: ts.NumericLiteral = ts.createNumericLiteral("NaN");
        compileNumericLiteral(pandaGen, node);
        let insns = pandaGen.getInsns();
        let expected = [new LdaDyn(new VReg())];
        expect(checkInstructions(insns, expected)).to.be.true;
    });

    it("Infinity", function() {
        let pandaGen = new PandaGen("ignored", 0, undefined);
        let node: ts.NumericLiteral = ts.createNumericLiteral("10e10000");
        compileNumericLiteral(pandaGen, node);
        let insns = pandaGen.getInsns();
        let expected = [new LdaDyn(new VReg())];
        expect(checkInstructions(insns, expected)).to.be.true;
    });
    it("int", function() {
        let pandaGen = new PandaGen("ignored", 0, undefined);
        let node: ts.NumericLiteral = ts.createNumericLiteral("1");
        compileNumericLiteral(pandaGen, node);
        let insns = pandaGen.getInsns();
        let expected = [new LdaiDyn(new Imm(ResultType.Int, 1))];
        expect(checkInstructions(insns, expected)).to.be.true;
    });
    it("Integer overflow", function() {
        let pandaGen = new PandaGen("ignored", 0, undefined);
        let node: ts.NumericLiteral = ts.createNumericLiteral("2147483648");
        compileNumericLiteral(pandaGen, node);
        let insns = pandaGen.getInsns();
        let expected = [new FldaiDyn(new Imm(ResultType.Float, 2147483648))];
        expect(checkInstructions(insns, expected)).to.be.true;
    });
    it("double", function() {
        let pandaGen = new PandaGen("ignored", 0, undefined);
        let node: ts.NumericLiteral = ts.createNumericLiteral("1.1");
        compileNumericLiteral(pandaGen, node);
        let insns = pandaGen.getInsns();
        let expected = [new FldaiDyn(new Imm(ResultType.Float, 1.1))];
        expect(checkInstructions(insns, expected)).to.be.true;
    });
})