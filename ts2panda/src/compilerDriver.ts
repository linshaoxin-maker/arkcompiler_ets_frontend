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

import { writeFileSync } from "fs";
import * as ts from "typescript";
import { addVariableToScope } from "./addVariable2Scope";
import { AssemblyDumper } from "./assemblyDumper";
import { hasDefaultKeywordModifier, hasExportKeywordModifier, initiateTs2abcChildProcess, terminateWritePipe } from "./base/util";
import { LiteralBuffer } from "./base/literal";
import { CmdOptions } from "./cmdOptions";
import { Compiler } from "./compiler";
import { CompilerStatistics } from "./compilerStatistics";
import { DebugInfo } from "./debuginfo";
import { hoisting } from "./hoisting";
import { IntrinsicExpander } from "./intrinsicExpander";
import { LOGD } from "./log";
import { setModuleNamespaceImports } from "./ecmaModule";
import { PandaGen } from "./pandagen";
import { Pass } from "./pass";
import { CacheExpander } from "./pass/cacheExpander";
import { Recorder } from "./recorder";
import { RegAlloc } from "./regAllocator";
import {
    FunctionScope,
    GlobalScope,
    ModuleScope,
    Scope,
    VariableScope
} from "./scope";
import { getClassNameForConstructor } from "./statement/classStatement";
import { checkDuplicateDeclaration } from "./syntaxChecker";
import { Ts2Panda } from "./ts2panda";
import { TypeRecorder } from "./typeRecorder";
import { findOuterNodeOfParenthesis } from "./expression/parenthesizedExpression";

export class PendingCompilationUnit {
    constructor(
        readonly decl: ts.FunctionLikeDeclaration,
        readonly scope: Scope,
        readonly internalName: string
    ) { }
}

/**
 * The class which drives the compilation process.
 * It handles all dependencies and run passes.
 */
export class CompilerDriver {
    private outputfileName: string;
    private passes: Pass[];
    private compilationUnits: PandaGen[];
    pendingCompilationUnits: PendingCompilationUnit[];
    private functionId: number = 1; // 0 reserved for main
    private funcIdMap: Map<ts.Node, number> = new Map<ts.Node, number>();
    private statistics: CompilerStatistics;
    private needDumpHeader: boolean = true;
    private ts2abcProcess: any = undefined;
    private recoderName: string;

    constructor(outputFileName: string, ts2abcProc: any, recoderName: string) {
        this.outputfileName = outputFileName;
        // register passes here
        this.passes = [
            new CacheExpander(),
            new IntrinsicExpander(),
            new RegAlloc(),
        ];
        this.compilationUnits = [];
        this.pendingCompilationUnits = [];
        this.statistics = new CompilerStatistics();
        this.ts2abcProcess = ts2abcProc;
        this.recoderName = recoderName;
    }

    getTs2abcProcess(): any {
        if (this.ts2abcProcess === undefined) {
            throw new Error("ts2abc hasn't been initiated");
        }
        return this.ts2abcProcess;
    }

    setTs2abcProcess(ts2abcProc: any) {
        this.ts2abcProcess = ts2abcProc;
    }

    getStatistics() {
        return this.statistics;
    }

    setCustomPasses(passes: Pass[]) {
        this.passes = passes;
    }

    getRecoderName(): string {
        return this.recoderName;
    }

    addCompilationUnit(
        decl: ts.FunctionLikeDeclaration,
        scope: Scope,
        recorder: Recorder
    ): string {
        let internalName = this.getFuncInternalName(decl, recorder);
        this.pendingCompilationUnits.push(
            new PendingCompilationUnit(decl, scope, internalName)
        );
        return internalName;
    }

    getCompilationUnits() {
        return this.compilationUnits;
    }

    kind2String(kind: ts.SyntaxKind) {
        return ts.SyntaxKind[kind];
    }

    getASTStatistics(node: ts.Node, statics: number[]) {
        node.forEachChild((childNode) => {
            statics[<number>childNode.kind] = statics[<number>childNode.kind] + 1;
            this.getASTStatistics(childNode, statics);
        });
    }

    // sort all function in post order
    postOrderAnalysis(scope: GlobalScope): VariableScope[] {
        let spArray: VariableScope[] = [];
        let stack: VariableScope[] = [];

        stack.push(scope);
        while (stack.length > 0) {
            let temp: VariableScope | undefined = stack.pop();
            if (temp == undefined) {
                break;
            }
            spArray.push(temp);

            for (let childVariableScope of temp.getChildVariableScope()) {
                stack.push(childVariableScope);
            }
        }

        return spArray.reverse();
    }

    compileForSyntaxCheck(node: ts.SourceFile): void {
        try {
            let recorder = this.compilePrologue(node, false);
            checkDuplicateDeclaration(recorder);
        } catch (err) {
            terminateWritePipe(this.getTs2abcProcess());
            throw err;
        }
    }

    compile(node: ts.SourceFile): void {
        this.showASTStatistics(node);
        if (!CmdOptions.isMergeAbcFiles()) {
            let ts2abcProc: any = initiateTs2abcChildProcess(this.outputfileName);
            this.setTs2abcProcess(ts2abcProc);
        }

        let recorder: Recorder;
        try {
            recorder = this.compilePrologue(node, true);
        } catch (err) {
            terminateWritePipe(this.getTs2abcProcess());
            throw err;
        }

        if (!CmdOptions.isAssemblyMode()) {
            try {
                this.prePendingCompilationUnits(recorder);

                if (!CmdOptions.isMergeAbcFiles()) {
                    Ts2Panda.dumpCommonFields(this.getTs2abcProcess(), this.outputfileName);
                    Ts2Panda.clearDumpData();
                }
                if (CmdOptions.isOutputType()) {
                    let typeFileName = this.outputfileName.substring(0, this.outputfileName.lastIndexOf(".")).concat(".txt");
                    writeFileSync(typeFileName, Ts2Panda.dumpTypeLiteralArrayBuffer());
                }

                // Ts2Panda.clearDumpData();
            } catch (err) {
                terminateWritePipe(this.getTs2abcProcess());
                throw err;
            }
        } else {
            this.prePendingCompilationUnits(recorder);
        }

        if (!CmdOptions.isMergeAbcFiles()) {
            PandaGen.clearRecoders();
            PandaGen.clearLiteralArrayBuffer();
        }
    }

    private showASTStatistics(node: ts.SourceFile): void {
        if (CmdOptions.showASTStatistics()) {
            let statics: number[] = new Array(ts.SyntaxKind.Count).fill(0);

            this.getASTStatistics(node, statics);
            statics.forEach((element, idx) => {
                if (element > 0) {
                    LOGD(this.kind2String(idx) + " = " + element);
                }
            });
        }
    }

    private prePendingCompilationUnits(recorder: Recorder): void {
        for (let i = 0; i < this.pendingCompilationUnits.length; i++) {
            let unit: PendingCompilationUnit = this.pendingCompilationUnits[i];
            this.compileImpl(unit.decl, unit.scope, unit.internalName, recorder);
        }
    }

    private compileImpl(node: ts.SourceFile | ts.FunctionLikeDeclaration, scope: Scope, internalName: string, recorder: Recorder): void {
        let pandaGen = new PandaGen(internalName, this.getParametersCount(node), scope);
        // for debug info
        DebugInfo.addDebugIns(scope, pandaGen, true);

        let compiler = new Compiler(node, pandaGen, this, recorder);

        // because of para vreg, don't change hosting's position
        hoisting(node, pandaGen, recorder, compiler);
        setModuleNamespaceImports(compiler, scope, pandaGen);
        compiler.compile();

        this.passes.forEach((pass) => pass.run(pandaGen));

        // for debug info
        DebugInfo.addDebugIns(scope, pandaGen, false);
        DebugInfo.setDebugInfo(pandaGen);
        DebugInfo.setSourceFileDebugInfo(pandaGen, node);

        if (CmdOptions.isAssemblyMode()) {
            this.writeBinaryFile(pandaGen);
        } else {
            Ts2Panda.dumpPandaGen(pandaGen, this.getTs2abcProcess(), recorder.recordType);
        }

        if (CmdOptions.showHistogramStatistics()) {
            this.statistics.getInsHistogramStatistics(pandaGen);
        }
    }

    compileUnitTest(node: ts.SourceFile, literalBufferArray?: Array<LiteralBuffer>): void {
        let recorder = this.compilePrologue(node, true);

        for (let i = 0; i < this.pendingCompilationUnits.length; i++) {
            let unit: PendingCompilationUnit = this.pendingCompilationUnits[i];
            this.compileUnitTestImpl(
                unit.decl,
                unit.scope,
                unit.internalName,
                recorder
            );
        }
        if (literalBufferArray) {
            PandaGen.getLiteralArrayBuffer().forEach(val => literalBufferArray.push(val));
        }

        PandaGen.clearLiteralArrayBuffer();
    }

    private compileUnitTestImpl(node: ts.SourceFile | ts.FunctionLikeDeclaration, scope: Scope, internalName: string, recorder: Recorder) {
        let pandaGen = new PandaGen(internalName, this.getParametersCount(node), scope);
        let compiler = new Compiler(node, pandaGen, this, recorder);

        hoisting(node, pandaGen, recorder, compiler);
        compiler.compile();

        this.passes.forEach((pass) => pass.run(pandaGen));

        this.compilationUnits.push(pandaGen);
    }

    private isTypeScriptSourceFile(node: ts.SourceFile) {
        let fileName = node.fileName;
        if (fileName && fileName.endsWith(".ts")) {
            return true;
        } else {
            return false;
        }
    }

    private compilePrologue(node: ts.SourceFile, recordType: boolean) {
        let topLevelScope: GlobalScope | ModuleScope;
        if (CmdOptions.isModules()) {
            topLevelScope = new ModuleScope(node);
        } else {
            topLevelScope = new GlobalScope(node);
        }

        let isTsFile = this.isTypeScriptSourceFile(node);
        let enableTypeRecord = recordType && CmdOptions.needRecordType() && isTsFile;
        if (enableTypeRecord) {
            TypeRecorder.createInstance();
        }
        let recorder = new Recorder(node, topLevelScope, this, enableTypeRecord, isTsFile);
        recorder.record();
        if (topLevelScope instanceof ModuleScope) {
            topLevelScope.module().setModuleEnvironment(topLevelScope);
        }
        addVariableToScope(recorder, enableTypeRecord);

        let postOrderVariableScopes = this.postOrderAnalysis(topLevelScope);

        for (let variableScope of postOrderVariableScopes) {
            this.addCompilationUnit(
                <ts.FunctionLikeDeclaration>variableScope.getBindingNode(),
                variableScope,
                recorder
            );
        }

        return recorder;
    }

    showStatistics(): void {
        if (CmdOptions.showHistogramStatistics()) {
            this.statistics.printHistogram(false);
        }

        if (CmdOptions.showHoistingStatistics()) {
            this.statistics.printHoistStatistics();
        }
    }

    getFuncId(node: ts.SourceFile | ts.FunctionLikeDeclaration | ts.ClassLikeDeclaration): number {
        if (this.funcIdMap.has(node)) {
            return this.funcIdMap.get(node)!;
        }

        if (ts.isSourceFile(node)) {
            this.funcIdMap.set(node, 0);
            return 0;
        }

        let idx = this.functionId++;

        this.funcIdMap.set(node, idx);
        return idx;
    }

    getRecoderFuncName(funcName: string): string {
        let recoderName: string = this.getRecoderName();
        funcName = `${recoderName}.${funcName}`
        return funcName;
    }

    /**
     * Internal name is used to indentify a function in panda file
     * Runtime uses this name to bind code and a Function object
     */
    getFuncInternalName(node: ts.SourceFile | ts.FunctionLikeDeclaration, recorder: Recorder): string {
        let name: string;
        if (ts.isSourceFile(node)) {
            name = "func_main_0";
        } else if (ts.isConstructorDeclaration(node)) {
            let classNode = node.parent;
            name = this.getInternalNameForCtor(classNode, node);
        } else {
            let funcNode = <ts.FunctionLikeDeclaration>node;
            name = (<FunctionScope>recorder.getScopeOfNode(funcNode)).getFuncName();
            if (name == "") {
                if ((ts.isFunctionDeclaration(node) && hasExportKeywordModifier(node) && hasDefaultKeywordModifier(node))
                || ts.isExportAssignment(findOuterNodeOfParenthesis(node))) {
                    return 'default';
                }
                name = `#${this.getFuncId(funcNode)}#`;
            } else if (name == "func_main_0") {
                name = `#${this.getFuncId(funcNode)}#${name}`;
            } else {
                let funcNameMap = recorder.getFuncNameMap();
                if (funcNameMap.has(name)) {
                    let freq = <number>funcNameMap.get(name);
                    if (freq > 1) {
                        name = `#${this.getFuncId(funcNode)}#${name}`;
                    }
                } else {
                    throw new Error("the function name is missing from the name map");
                }

                if (name.lastIndexOf(".") != -1) {
                    name = `#${this.getFuncId(funcNode)}#`;
                }
            }
        }
        return this.getRecoderFuncName(name);
    }

    getInternalNameForCtor(node: ts.ClassLikeDeclaration, ctor: ts.ConstructorDeclaration) {
        let name = getClassNameForConstructor(node);
        name = `#${this.getFuncId(ctor)}#${name}`
        if (name.lastIndexOf(".") != -1) {
            name = `#${this.getFuncId(ctor)}#`
        }
        return name;
    }

    writeBinaryFile(pandaGen: PandaGen) {
        if (this.needDumpHeader) {
            AssemblyDumper.dumpHeader();
            this.needDumpHeader = false;
        }
        new AssemblyDumper(pandaGen).dump();
    }

    private getParametersCount(node: ts.SourceFile | ts.FunctionLikeDeclaration): number {
        // each function and global scope accepts three parameters - funcObj + newTarget + this.
        // the runtime passes these to global scope when calls it
        let parametersCount = 3;
        if (node.kind == ts.SyntaxKind.SourceFile) {
            return parametersCount;
        }
        let decl = <ts.FunctionLikeDeclaration>node;
        parametersCount += decl.parameters.length;
        return parametersCount;
    }
}
