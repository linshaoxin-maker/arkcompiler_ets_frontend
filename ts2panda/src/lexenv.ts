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
    loadAccumulator,
    loadAccumulatorString,
    loadLexicalVar,
    storeAccumulator,
    storeLexicalVar,
    storeModuleVariable,
    throwConstAssignment,
    throwUndefinedIfHole
} from "./base/bcGenUtil";
import { CacheList, getVregisterCache } from "./base/vregisterCache";
import { Compiler } from "./compiler";
import { NodeKind } from "./debuginfo";
import {
    IRNode,
    VReg
} from "./irnodes";
import { PandaGen } from "./pandagen";
import { Scope } from "./scope";
import {
    LocalVariable,
    ModuleVariable,
    Variable
} from "./variable";
import jshelpers from "./jshelpers";

abstract class VariableAccessBase {
    variable: Variable;
    scope: Scope;
    level: number;

    constructor(scope: Scope, level: number, variable: Variable) {
        this.variable = variable;
        this.scope = scope;
        this.level = level;
    }

    isLexVar() {
        return this.variable.isLexVar;
    }

    getEnvSlotOfVar(): number | undefined {
        if (this.isLexVar()) {
            return this.variable.idxLex;
        }

        return undefined;
    }

    abstract expand(pandaGen: PandaGen, compiler: Compiler): Array<IRNode>;
}

export class VariableAccessLoad extends VariableAccessBase {
    constructor(scope: Scope, level: number, variable: Variable) {
        super(scope, level, variable);
    }

    expand(pandaGen: PandaGen): Array<IRNode> {
        if (this.isLexVar()) {
            return this.loadLexEnvVar(pandaGen);
        } else {
            return this.loadLocalVar(pandaGen);
        }
    }

    private loadLocalVar(pandaGen: PandaGen): Array<IRNode> {
        let insns: Array<IRNode> = new Array<IRNode>();
        let v = this.variable;
        let bindVreg = pandaGen.getVregForVariable(v);

        // check TDZ first
        if (!(<LocalVariable>v).isInitialized()) {
            let holeReg = pandaGen.getTemp();
            insns.push(loadAccumulator(getVregisterCache(pandaGen, CacheList.HOLE)));
            insns.push(storeAccumulator(holeReg));
            checkTDZ(pandaGen, holeReg, v.getName(), insns);
            pandaGen.freeTemps(holeReg);
            return insns;
        }
        insns.push(loadAccumulator(bindVreg));

        return insns;
    }

    private loadLexEnvVar(pandaGen: PandaGen): Array<IRNode> {
        let insns: Array<IRNode> = new Array<IRNode>();
        let v = this.variable;

        let slot = v.idxLex;
        insns.push(loadLexicalVar(this.level, slot));

        // check TDZ
        if (v.isLetOrConst()) {
            let tempReg = pandaGen.getTemp();

            insns.push(storeAccumulator(tempReg));
            checkTDZ(pandaGen, tempReg, v.getName(), insns);
            insns.push(loadAccumulator(tempReg));
            pandaGen.freeTemps(tempReg);
        }

        return insns;
    }
}

export class VariableAcessStore extends VariableAccessBase {
    node: ts.Node | NodeKind;
    isDeclaration: boolean;
    constructor(scope: Scope, level: number, variable: Variable, isDeclaration: boolean, node: ts.Node | NodeKind) {
        super(scope, level, variable);
        this.isDeclaration = isDeclaration;
        this.node = node;
    }

    expand(pandaGen: PandaGen): Array<IRNode> {
        if (this.isLexVar()) {
            return this.storeLexEnvVar(pandaGen);
        } else {
            return this.storeLocalVar(pandaGen);
        }
    }

    private storeLocalVar(pandaGen: PandaGen): Array<IRNode> {
        let insns: Array<IRNode> = new Array<IRNode>();
        let v = <LocalVariable>this.variable;
        let bindVreg = pandaGen.getVregForVariable(v);

        if (!this.isDeclaration) {
            // check TDZ first
            if (!v.isInitialized()) {
                let nameReg = pandaGen.getTemp();
                let tempReg = pandaGen.getTemp();
                let holeReg = pandaGen.getTemp();
                insns.push(storeAccumulator(tempReg));
                insns.push(loadAccumulator(getVregisterCache(pandaGen, CacheList.HOLE)));
                insns.push(storeAccumulator(holeReg));
                checkTDZ(pandaGen, holeReg, v.getName(), insns);
                insns.push(loadAccumulator(tempReg));
                pandaGen.freeTemps(nameReg, tempReg, holeReg);
            }

            // check const assignment
            checkConstAssignment(pandaGen, v, insns, this.node);
        }

        insns.push(storeAccumulator(bindVreg));

        if (v.isExportVar() && !(v instanceof ModuleVariable)) {
            insns.push(storeModuleVariable(v.getExportedName()));
        }

        return insns;
    }

    private storeLexEnvVar(pandaGen: PandaGen): Array<IRNode> {
        let insns: Array<IRNode> = new Array<IRNode>();
        let v = <LocalVariable>this.variable;

        // save the value first
        let valueReg: VReg = pandaGen.getTemp();
        insns.push(storeAccumulator(valueReg));

        let slot = v.idxLex;
        if (v.isLetOrConst() || v.isClass()) {
            if (!this.isDeclaration) {
                let holeReg = pandaGen.getTemp();
                /**
                 * check TDZ first
                 * If acc == hole -> throw reference error
                 * else -> excute the next insn
                */
                insns.push(loadLexicalVar(this.level, slot));
                insns.push(storeAccumulator(holeReg));
                checkTDZ(pandaGen, holeReg, v.getName(), insns);

                // const assignment check need to be down after TDZ check
                checkConstAssignment(pandaGen, v, insns, this.node);
                pandaGen.freeTemps(holeReg);
            }
        }

        insns.push(storeLexicalVar(this.level, slot, valueReg));
        insns.push(loadAccumulator(valueReg));
        if (v.isExportVar() && !(v instanceof ModuleVariable)) {
            insns.push(storeModuleVariable(v.getExportedName()));
        }
        pandaGen.freeTemps(valueReg);

        return insns;
    }
}

function checkTDZ(pg: PandaGen, holeReg: VReg, name: string, expansion: IRNode[]) {
    let nameReg = pg.getTemp();
    expansion.push(loadAccumulatorString(name));
    expansion.push(storeAccumulator(nameReg));
    expansion.push(throwUndefinedIfHole(holeReg, nameReg));
    pg.freeTemps(nameReg);
}

function checkConstAssignment(pg: PandaGen, v: Variable, expansion: IRNode[], node: ts.Node | NodeKind) {
    let nameReg = pg.getTemp();
    if (v.isConst()) {
        expansion.push(loadAccumulatorString(v.getName()));
        expansion.push(storeAccumulator(nameReg));
        expansion.push(throwConstAssignment(nameReg));
    }

    if (v.isClass() && node != NodeKind.FirstNodeOfFunction &&
        node != NodeKind.Invalid && node != NodeKind.Normal) {
        let className = v.getName();
        while (node) {
            if (ts.isClassLike(node) && node.name &&
                jshelpers.getTextOfIdentifierOrLiteral(node.name) == className) {
                break;
            }

            node = node.parent;
        }

        // class name binding inside class is immutable
        if (node) {
            expansion.push(loadAccumulatorString(className));
            expansion.push(storeAccumulator(nameReg));
            expansion.push(throwConstAssignment(nameReg));
        }
    }

    pg.freeTemps(nameReg);
}