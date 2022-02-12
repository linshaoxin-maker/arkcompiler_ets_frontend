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

import * as ts from "typescript";
import {
    checkIsGlobalDeclaration,
    initiateTs2abcChildProcess,
    terminateWritePipe,
    isSourceFileFromLibrary
} from "./base/util";
import { CmdOptions } from "./cmdOptions";
import { CompilerDriver } from "./compilerDriver";
import * as diag from "./diagnostic";
import { LOGD, LOGE } from "./log";
import { PandaGen } from "./pandagen";
import { Record } from "./pandasm";
import { Pass } from "./pass";
import { CacheExpander } from "./pass/cacheExpander";
import { ICPass } from "./pass/ICPass";
import { RegAlloc } from "./regAllocator";
import { setGlobalDeclare, setGlobalStrict } from "./strictMode";
import { Ts2Panda } from "./ts2panda";
import { TypeChecker } from "./typeChecker";
import jshelpers = require("./jshelpers");
import path = require("path");

function main(fileNames: string[], options: ts.CompilerOptions) {
    let program: ts.Program = ts.createProgram(fileNames, options);
    let typeChecker = TypeChecker.getInstance();
    typeChecker.setTypeChecker(program.getTypeChecker());

    let outputFileName: string = CmdOptions.getOutputFileName();
    let ts2abcProc: any
    if (CmdOptions.isMergeAbcFiles()) {
        ts2abcProc = initiateTs2abcChildProcess(outputFileName);
    }

    generateDTs(program, outputFileName, options, ts2abcProc);

    let emitResult = program.emit(
        undefined, undefined, undefined, undefined,
        {
            before: [
                (ctx: ts.TransformationContext) => {
                    return (node: ts.SourceFile) => {
                        let recoderName: string = getRecoderName(node.fileName);
                        let outputBinName = getOutputFileName(node, outputFileName);
                        let compilerDriver = new CompilerDriver(outputBinName, ts2abcProc, recoderName);
                        compilerDriver.compileForSyntaxCheck(node);
                        return node;
                    }
                }
            ],
            after: [
                (ctx: ts.TransformationContext) => {
                    return (node: ts.SourceFile) => {
                        return ts2abcTransform(node, outputFileName, options, ts2abcProc);
                    }
                }
            ]
        }
    );

    if (preDiagnostics(program, emitResult)) {
        terminateWritePipe(ts2abcProc);
        return;
    }

    if (!CmdOptions.isAssemblyMode()) {
        if (CmdOptions.isMergeAbcFiles()) {
            Ts2Panda.dumpCommonFields(ts2abcProc, outputFileName);
            PandaGen.clearRecoders();
            PandaGen.clearLiteralArrayBuffer();
        }
    }
}

namespace Compiler {
    export namespace Options {
        export let Default: ts.CompilerOptions = {
            outDir: "../tmp/build",
            allowJs: true,
            noEmitOnError: true,
            noImplicitAny: true,
            target: ts.ScriptTarget.ES2015,
            module: ts.ModuleKind.ES2015,
            strictNullChecks: true,
            skipLibCheck: true,
            alwaysStrict: true,
            importsNotUsedAsValues: ts.ImportsNotUsedAsValues.Preserve
        };
    }
}

function generateDTs(program: ts.Program, outputFileName: string, options: ts.CompilerOptions, ts2abcProc: any): void {
    if (CmdOptions.needRecordDtsType()) {
        for (let sourceFile of program.getSourceFiles()) {
            if (sourceFile.isDeclarationFile && !isSourceFileFromLibrary(program, sourceFile)) {
                setGlobalDeclare(checkIsGlobalDeclaration(sourceFile));
                ts2abcTransform(sourceFile, outputFileName, options, ts2abcProc);
            }
        }
    }
}

function ts2abcTransform(node: ts.SourceFile, outputFileName: string, options: ts.CompilerOptions, ts2abcProc: any): ts.SourceFile {
    let sourceFileName: string = node.fileName;
    let recoderName: string = getRecoderName(sourceFileName);
    if (recoderName == CmdOptions.getEntryPoint()) {
        recoderName = '_GLOBAL';
    }
    // console.error("-----------------sourceFileName:  ", sourceFileName);
    // console.error("-----------------recoderName:  ", recoderName);
    let recoder: Record = new Record(recoderName, sourceFileName);
    let recoders: Array<Record> = PandaGen.getRecoders();
    recoders.push(recoder);
    if (ts.getEmitHelpers(node)) {
        const printer: ts.Printer = ts.createPrinter({ newLine: ts.NewLineKind.LineFeed });
        const text: string = printer.printNode(ts.EmitHint.Unspecified, node, node);
        let newNode = ts.createSourceFile(node.fileName, text, options.target!);
        node = newNode;
    }
    let newOutputFileName: string = getOutputFileName(node, outputFileName);
    let compilerDriver = new CompilerDriver(newOutputFileName, ts2abcProc, recoderName);
    setGlobalStrict(jshelpers.isEffectiveStrictModeSourceFile(node, options));
    if (CmdOptions.isVariantBytecode()) {
        LOGD("variant bytecode dump");
        let passes: Pass[] = [
            new CacheExpander(),
            new ICPass(),
            new RegAlloc()
        ];
        compilerDriver.setCustomPasses(passes);
    }
    compilerDriver.compile(node);
    compilerDriver.showStatistics();
    return node;
}

function preDiagnostics(program: ts.Program, emitResult: ts.EmitResult): boolean {
    let hasDiagnostics: boolean = false;
    let allDiagnostics: ts.Diagnostic[] = ts
        .getPreEmitDiagnostics(program)
        .concat(emitResult.diagnostics);

    allDiagnostics.forEach((diagnostic: ts.Diagnostic) => {
        diag.printDiagnostic(diagnostic);
        hasDiagnostics = true;
    });
    return hasDiagnostics;
}

export function getRecoderName(sourceFileName: string): string {
    let recoderName: string = sourceFileName.substring(0, sourceFileName.lastIndexOf("."));
    let recoderDirInfo: string[] = CmdOptions.getModulesDirMap();
    if (recoderDirInfo.length) {
        let index: number = recoderDirInfo.indexOf(sourceFileName);
        if (index !== -1) {
            return recoderDirInfo[index + 1];
        }
    }

    if (!path.isAbsolute(sourceFileName)) {
        let absoluteSourceFile = path.join(process.cwd(), sourceFileName);
        recoderName = absoluteSourceFile.substring(0, absoluteSourceFile.lastIndexOf("."));
    }

    return recoderName;
}

function getOutputFileName(node: ts.SourceFile, outputFileName: string): string {
    let fileName = node.fileName.substring(0, node.fileName.lastIndexOf('.'));
    let inputFileName = CmdOptions.getInputFileName();
    if (/^win/.test(require('os').platform())) {
        var inputFileTmps = inputFileName.split(path.sep);
        inputFileName = path.posix.join(...inputFileTmps);
    }
    if (fileName != inputFileName) {
        outputFileName = fileName + ".abc";
    }
    return outputFileName;
}

function run(args: string[], options?: ts.CompilerOptions): void {
    let parsed = CmdOptions.parseUserCmd(args);
    if (!parsed) {
        return;
    }

    if (options) {
        if (!((parsed.options.project) || (parsed.options.build))) {
            parsed.options = options;
        }
    }
    try {
        main(parsed.fileNames, parsed.options);
    } catch (err) {
        if (err instanceof diag.DiagnosticError) {
            let diagError: diag.DiagnosticError = <diag.DiagnosticError>err;
            let diagnostic: ts.DiagnosticMessage | undefined = diag.getDiagnostic(diagError.code);
            if (diagnostic != undefined) {
                let diagnosticLog = diag.createDiagnostic(diagError.file, diagError.irnode, diagnostic, ...diagError.args);
                diag.printDiagnostic(diagnosticLog);
            }
        } else if (err instanceof SyntaxError) {
            LOGE(err.name, err.message);
        } else {
            throw err;
        }
    }
}

run(process.argv.slice(2), Compiler.Options.Default);
global.gc();
