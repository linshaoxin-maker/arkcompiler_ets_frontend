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
    DefinefuncDyn,
    EqDyn,
    Imm,
    Jeqz,
    Label,
    LdaDyn,
    LdaiDyn,
    LdaStr,
    ResultType,
    ReturnUndefined,
    StaDyn,
    StGlobalVar,
    ThrowUndefinedIfHole,
    Toboolean,
    VReg
} from "../src/irnodes";
import { checkInstructions, compileMainSnippet, SnippetCompiler } from "./utils/base";

describe("HoistTest", function() {

    // case 1: hoist var declared variable ((declared in global scope)) in global scope
    it('case 1;', function() {
        let insns = compileMainSnippet("var a = 1;");
        let expected = [
            new LdaDyn(new VReg()),
            new StGlobalVar("a"),
            new LdaiDyn(new Imm(ResultType.Int, 1)),
            new StGlobalVar("a"),
            new ReturnUndefined()
        ]

        expect(checkInstructions(insns, expected)).to.be.true;
    });

    // case 2: hoist var declared variable (declared in local scope) in global scope
    it('case 2', function() {
        let insns = compileMainSnippet(`if (true) {
                                  var a = 2;
                                }`);
        let conditionReg = new VReg();
        let endLabel = new Label();

        let expected = [
            new LdaDyn(new VReg()),
            new StGlobalVar("a"),
            new LdaDyn(new VReg()),
            new Toboolean(),
            new EqDyn(conditionReg),
            new Jeqz(endLabel),
            new LdaiDyn(new Imm(ResultType.Int, 2)),
            new StGlobalVar("a"),
            endLabel,
            new ReturnUndefined()
        ]
        expect(checkInstructions(insns, expected)).to.be.true;
    });

    // case 3: hoist function declaration in global scope
    it('case 3', function() {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile(`function a() {};`);

        let insns = snippetCompiler.getGlobalInsns();
        let expected = [
            new DefinefuncDyn("func_a_1", new VReg()),
            new StGlobalVar("a"),
            new ReturnUndefined()
        ]
        expect(checkInstructions(insns, expected)).to.be.true;
    });

    // case 4: In case that two function declared directly in global scope with the same name, hoist the later one.
    it('case 4', function() {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile(`function a() {}; function a() {}`);

        let insns = snippetCompiler.getGlobalInsns();
        let expected = [
            new DefinefuncDyn("func_a_2", new VReg()),
            new StGlobalVar("a"),
            new ReturnUndefined()
        ]

        expect(checkInstructions(insns, expected)).to.be.true;
    });

    // case 5: hoisting of function declaration is of higher priority than var declared variables with a same name in global scope
    it('case 5', function() {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile(`var a = 1; function a() {}`);
        let insns = snippetCompiler.getGlobalInsns();
        let expected = [
            new DefinefuncDyn("func_a_1", new VReg()),
            new StGlobalVar("a"),
            new LdaiDyn(new Imm(ResultType.Int, 1)),
            new StGlobalVar("a"),
            new ReturnUndefined()
        ]

        expect(checkInstructions(insns, expected)).to.be.true;
    });

    // case 6: hoist var declared variable in function scope
    it('case 6', function() {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile(`function a() {var a = 1;}`);
        let funcPg = snippetCompiler.getPandaGenByName("func_a_1");
        let insns = funcPg!.getInsns();

        let builtInUndefinedVreg = new VReg();
        let a = new VReg();
        let expected = [
            new LdaDyn(builtInUndefinedVreg),
            new StaDyn(a),
            new LdaiDyn(new Imm(ResultType.Int, 1)),
            new StaDyn(a),

            new ReturnUndefined()
        ]

        expect(checkInstructions(insns!, expected)).to.be.true;
    });

    // case 7: hoist function declaration in function scope
    it('case 7', function() {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile(`function a() {function b() {}};`);
        let funcPg = snippetCompiler.getPandaGenByName("func_a_1");
        let insns = funcPg!.getInsns();
        let a = new VReg();
        let expected = [
            new DefinefuncDyn("func_b_2", new VReg()),
            new StaDyn(a),

            new ReturnUndefined()
        ]

        expect(checkInstructions(insns!, expected)).to.be.true;
    });

    // case 8: temporary dead zone of let in global scope
    it('case 8', function() {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile(`a = 1;
                                 let a;`);
        let funcPg = snippetCompiler.getPandaGenByName("func_main_0");
        let insns = funcPg!.getInsns();
        let idReg = new VReg();
        let expected = [
            new LdaStr("a"),
            new StaDyn(idReg),
            new ThrowUndefinedIfHole(new VReg(), idReg)
        ]

        expect(checkInstructions(insns.slice(3, 5), expected));
    });

    // case 9: temporary dead zone of let in function scope
    it('case 9', function() {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile(`function b() {
                                 a = 1;
                                 let a;
                                 }`);
        let funcPg = snippetCompiler.getPandaGenByName("func_b_1");
        let insns = funcPg!.getInsns();
        let idReg = new VReg();

        let expected = [
            new LdaStr("a"),
            new StaDyn(idReg),
            new ThrowUndefinedIfHole(new VReg(), idReg)
        ]

        expect(checkInstructions(insns.slice(3, 5), expected));
    });

    // case 10: temporary dead zone of let in local scope
    it('case 10', function() {
        let snippetCompiler = new SnippetCompiler();
        snippetCompiler.compile(`{
                                    a = 1;
                                    let a;
                                 }`);
        let funcPg = snippetCompiler.getPandaGenByName("func_main_0");
        let insns = funcPg!.getInsns();
        let idReg = new VReg();

        let expected = [
            new LdaStr("a"),
            new StaDyn(idReg),
            new ThrowUndefinedIfHole(new VReg(), idReg)
        ]

        expect(checkInstructions(insns.slice(3, 5), expected));
    })
})
