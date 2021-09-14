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

import { CacheList } from "./base/vregisterCache";
import { DebugInfo } from "./debuginfo";
import {
    EcmaCallithisrangedyn,
    EcmaCallirangedyn,
    EcmaNewobjdynrange,
    EcmaCreateobjectwithexcludedkeys,
    Format,
    IRNode,
    MovDyn,
    OperandKind,
    OperandType,
    VReg,
    Imm
} from "./irnodes";
import { PandaGen } from "./pandagen";
import {
    isRangeInst,
    getRangeStartVregPos,
    getRangeExplicitVregNums,
} from "./base/util";

const MAX_VREGA = 16;
const MAX_VREGB = 256;
const MAX_VREGC = 65536;

class VRegWithFlag {
    constructor(vreg: VReg) {
        this.flag = false;
        this.vreg = vreg;
    }
    vreg: VReg;
    flag: boolean; // indicate whether it is used as a temporary register for spill
}

class RegAllocator {
    private spills: VReg[] = [];
    private vRegsId: number = 0;
    private usedVreg: VRegWithFlag[] = [];
    private tmpVreg: VRegWithFlag[] = [];

    constructor() {
        this.vRegsId = 0;
    }

    allocIndexForVreg(vreg: VReg) {
        let num = this.getFreeVreg();
        vreg.num = num;
        this.usedVreg[num] = new VRegWithFlag(vreg);
    }

    findTmpVreg(level: number): VReg {
        let iterCnts = Math.min(MAX_VREGB, this.usedVreg.length);
        for (let i = 0; i < iterCnts; ++i) {
            let value = this.usedVreg[i];
            if (value === undefined || value.flag) {
                continue;
            }
            if (level === MAX_VREGA && value.vreg.num >= MAX_VREGA) {
                throw new Error("no available tmp vReg from A");
            }
            value.flag = true;
            this.tmpVreg.push(value);
            return value.vreg;
        }
        throw new Error("no available tmp vReg from B");
    }

    clearVregFlags(): void {
        for (let v of this.tmpVreg) {
            v.flag = false;
        }
        this.tmpVreg = [];
    }
    allocSpill(): VReg {
        if (this.spills.length > 0) {
            return this.spills.pop()!;
        }
        let v = new VReg();
        this.allocIndexForVreg(v);
        return v;
    }
    freeSpill(v: VReg): void {
        this.spills.push(v);
    }

    getFreeVreg(): number {
        if (this.vRegsId >= MAX_VREGC) {
            throw new Error("vreg has been running out");
        }
        return this.vRegsId++;
    }

    /* check whether the operands is valid for the format,
       return 0 if it is valid, otherwise return the total
       number of vreg which does not meet the requirement
    */
    getNumOfInvalidVregs(operands: OperandType[], format: Format): number {
        let num = 0;
        for (let j = 0; j < operands.length; ++j) {
            if (operands[j] instanceof VReg) {
                if ((<VReg>operands[j]).num >= (1 << format[j].bitwidth)) {
                    num++;
                }
            }
        }
        return num;
    }

    markVregNotAvailableAsTmp(vreg: VReg): void {
        let num = vreg.num;
        this.usedVreg[num].flag = true;
        this.tmpVreg.push(this.usedVreg[num]);
    }

    doRealAdjustment(operands: OperandType[], format: Format, index: number, irNodes: IRNode[]): number {
        let head: IRNode[] = [];
        let tail: IRNode[] = [];
        let spills: VReg[] = [];

        // mark all vreg used in the current insn as not valid for tmp register
        for (let i = 0; i < operands.length; ++i) {
            if (operands[i] instanceof VReg) {
                this.markVregNotAvailableAsTmp(<VReg>operands[i]);
            }
        }
        for (let j = 0; j < operands.length; ++j) {
            if (operands[j] instanceof VReg) {
                let vOrigin = <VReg>operands[j];
                if (vOrigin.num >= (1 << format[j].bitwidth)) {
                    let spill = this.allocSpill();
                    spills.push(spill);
                    let vTmp;
                    try {
                        vTmp = this.findTmpVreg(1 << format[j].bitwidth);
                    } catch {
                        throw Error("no available tmp vReg");
                    }
                    head.push(new MovDyn(spill, vTmp));
                    operands[j] = vTmp;
                    if (format[j].kind == OperandKind.SrcVReg) {
                        head.push(new MovDyn(vTmp, vOrigin));
                    } else if (format[j].kind == OperandKind.DstVReg) {
                        tail.push(new MovDyn(vOrigin, vTmp))
                    } else if (format[j].kind == OperandKind.SrcDstVReg) {
                        head.push(new MovDyn(vTmp, vOrigin));
                        tail.push(new MovDyn(vOrigin, vTmp))
                    } else {
                        // here we do nothing
                    }
                    tail.push(new MovDyn(vTmp, spill));
                }
            }
        }

        // for debuginfo
        DebugInfo.copyDebugInfo(irNodes[index], head);
        DebugInfo.copyDebugInfo(irNodes[index], tail);

        irNodes.splice(index, 0, ...head);
        irNodes.splice(index + head.length + 1, 0, ...tail);
        for (let j = spills.length - 1; j >= 0; --j) {
            this.freeSpill(spills[j]);
        }
        this.clearVregFlags();

        return (head.length + tail.length);
    }

    checkDynRangeInstruction(irNodes: IRNode[], index: number): boolean {
        let operands = irNodes[index].operands;
        let rangeRegOffset = getRangeStartVregPos(irNodes[index]);
        let level = 1 << irNodes[index].formats[0][rangeRegOffset].bitwidth;

        /*
          1. "CalliDynRange 4, v255" is a valid insn, there is no need for all 4 registers numbers to be less than 255,
          it is also similar for NewobjDyn
          2. we do not need to mark any register to be invalid for tmp register, since no other register is used in calli.dyn.range
          3. if v.num is bigger than 255, it means all register less than 255 has been already used, they should have been pushed
          into usedVreg
        */
        if ((<VReg>operands[1]).num >= level) {
            // needs to be adjusted.
            return false;
        }

        /* the first operand is an imm */
        let startNum = (<VReg>operands[rangeRegOffset]).num;
        let i = rangeRegOffset + 1;
        let implicitRegNums = (irNodes[index]).operands.length - i;
        for (; i < (irNodes[index]).operands.length; ++i) {
            if ((startNum + 1) != (<VReg>operands[i]).num) {
                throw Error("Warning: VReg sequence of DynRange is not continuous. Please adjust it now.");
            }
            startNum++;
        }

        /* If the parameters are consecutive, no adjustment is required. */
        if (i == (irNodes[index]).operands.length) {
            return true;
        }

        // needs to be adjusted.
        return false;
    }

    adjustDynRangeInstruction(irNodes: IRNode[], index: number): number {
        let head: IRNode[] = [];
        let tail: IRNode[] = [];
        let spills: VReg[] = [];
        let operands = irNodes[index].operands;

        /* exclude operands that are not require consecutive */
        let rangeRegOffset = getRangeStartVregPos(irNodes[index]);
        let regNums = operands.length - getRangeStartVregPos(irNodes[index]);

        let level = 1 << irNodes[index].formats[0][rangeRegOffset].bitwidth;
        let tmp = this.findTmpVreg(level);

        for (let i = 0; i < regNums; i++) {
            let spill = this.allocSpill();
            spills.push(spill);

            /* We need to make sure that the register input in the .range instruction is continuous(small to big). */
            head.push(new MovDyn(spill, this.usedVreg[tmp.num + i].vreg));
            head.push(new MovDyn(this.usedVreg[tmp.num + i].vreg, <VReg>operands[i + rangeRegOffset]));
            operands[i + rangeRegOffset] = this.usedVreg[tmp.num + i].vreg;
            tail.push(new MovDyn(this.usedVreg[tmp.num + i].vreg, spill));
        }

        // for debuginfo
        DebugInfo.copyDebugInfo(irNodes[index], head);
        DebugInfo.copyDebugInfo(irNodes[index], tail);

        irNodes.splice(index, 0, ...head);
        irNodes.splice(index + head.length + 1, 0, ...tail);
        for (let i = spills.length - 1; i >= 0; --i) {
            this.freeSpill(spills[i]);
        }
        this.clearVregFlags();

        return (head.length + tail.length);
    }

    adjustInstructionsIfNeeded(irNodes: IRNode[]): void {
        for (let i = 0; i < irNodes.length; ++i) {
            let operands = irNodes[i].operands;
            let formats = irNodes[i].formats;
            if (isRangeInst(irNodes[i])) {
                if (this.checkDynRangeInstruction(irNodes, i)) {
                    continue;
                }

                i += this.adjustDynRangeInstruction(irNodes, i);
                continue;
            }

            let min = operands.length;
            let minFormat = formats[0];
            for (let j = 0; j < formats.length; ++j) {
                let num = this.getNumOfInvalidVregs(operands, formats[j]);
                if (num < min) {
                    minFormat = formats[j];
                    min = num;
                }
            }
            if (min > 0) {
                i += this.doRealAdjustment(operands, minFormat, i, irNodes);
            }
        }
    }

    getTotalRegsNum(): number {
        return this.vRegsId;
    }

    run(pandaGen: PandaGen): void {
        let irNodes = pandaGen.getInsns();
        let locals = pandaGen.getLocals();
        let temps = pandaGen.getTemps();
        let cache = pandaGen.getVregisterCache();
        let parametersCount = pandaGen.getParametersCount();
        // don't mess up allocation order
        for (let i = 0; i < locals.length; ++i) {
            this.allocIndexForVreg(locals[i]);
        }
        for (let i = 0; i < temps.length; ++i) {
            this.allocIndexForVreg(temps[i]);
        }
        for (let i = CacheList.MIN; i < CacheList.MAX; ++i) {
            let cacheItem = cache.getCache(i);
            if (cacheItem.isNeeded()) {
                this.allocIndexForVreg(cacheItem.getCache());
            }
        }
        this.adjustInstructionsIfNeeded(irNodes);
        for (let i = 0; i < parametersCount; ++i) {
            let v = new VReg();
            this.allocIndexForVreg(v);
            irNodes.splice(0, 0, new MovDyn(locals[i], v));
        }
    }
}

export class RegAlloc {
    run(pandaGen: PandaGen): void {
        let regalloc = new RegAllocator();

        regalloc.run(pandaGen);
        pandaGen.setTotalRegsNum(regalloc.getTotalRegsNum());
    }
}
