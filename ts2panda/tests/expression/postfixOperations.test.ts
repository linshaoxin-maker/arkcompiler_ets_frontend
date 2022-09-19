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
import { loadAccumulator } from 'src/base/bcGenUtil';
import {
    Dec,
    Inc,
    Returnundefined,
    Sttoglobalrecord,
    Tonumeric,
    Tryldglobalbyname,
    Trystglobalbyname,
    Imm,
    Ldai,
    Sta,
    VReg,
    Lda
} from "../../src/irnodes";
import { checkInstructions, compileMainSnippet } from "../utils/base";

describe("PostfixOperationsTest", function () {
    it("let i = 0; i++", function () {
        let insns = compileMainSnippet("let i = 5; i++");
        let i = new VReg();
        let temp = new VReg();
        let expected = [
            new Ldai(new Imm(5)),
            new Sttoglobalrecord(new Imm(0), 'i'),
            new Tryldglobalbyname(new Imm(1), 'i'),
            new Sta(temp),
            new Lda(temp),
            new Inc(new Imm(2)),
            new Trystglobalbyname(new Imm(3), 'i'),
            new Lda(i),
            new Tonumeric(new Imm(4), ),
            new Returnundefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
    });

    it("let i = 0; i--", function () {
        let insns = compileMainSnippet("let i = 5; i--");
        let i = new VReg();
        let temp = new VReg();
        let expected = [
            new Ldai(new Imm(5)),
            new Sttoglobalrecord(new Imm(0), 'i'),
            new Tryldglobalbyname(new Imm(1), 'i'),
            new Sta(temp),
            new Lda(temp),
            new Dec(new Imm(2)),
            new Trystglobalbyname(new Imm(3), 'i'),
            new Lda(i),
            new Tonumeric(new Imm(4)),
            new Returnundefined()
        ];
        expect(checkInstructions(insns, expected)).to.be.true;
    });
});


