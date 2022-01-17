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
import { initiateTs2abcChildProcess } from "./base/util";
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
import { setGlobalStrict } from "./strictMode";
import { Ts2Panda } from "./ts2panda";
import jshelpers = require("./jshelpers");
import path = require("path");

function main(fileNames: string[], options: ts.CompilerOptions) {
    let program: ts.Program = ts.createProgram(fileNames, options);
    let outputFileName: string = CmdOptions.getOutputFileName();

    let ts2abcProc: any
    if (CmdOptions.isMergeAbcFiles()) {
        ts2abcProc = initiateTs2abcChildProcess(outputFileName);
    }

    let emitResult = program.emit(
        undefined, undefined, undefined, undefined,
        {
            before: [
                (ctx: ts.TransformationContext) => {
                    return (node: ts.SourceFile) => {
                        return ts2abcTransform(node, outputFileName, options, ts2abcProc);
                    }
                }
            ]
        }
    );

    if (preDiagnostics(program, emitResult)) {
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
            module: ts.ModuleKind.CommonJS,
            strictNullChecks: true,
            skipLibCheck: true,
            alwaysStrict: true
        };
    }
}

function ts2abcTransform(node: ts.SourceFile,
    outputFileName: string,
    options: ts.CompilerOptions,
    ts2abcProc: any): ts.SourceFile {
    let sourceFileName: string = node.fileName;
    let recoderName: string = getRecoderName(sourceFileName);
    let recoder: Record = new Record(recoderName, sourceFileName);
    let recoders: Array<Record> = PandaGen.getRecoders();
    recoders.push(recoder);
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

function getRecoderName(sourceFileName: string): string {
    let recoderName: string = sourceFileName.substring(0, sourceFileName.lastIndexOf("."));
    let recoderDirInfo: string[] = CmdOptions.getModulesDirMap();
    if (recoderDirInfo.length) {
        let index: number = recoderDirInfo.indexOf(sourceFileName);
        if (index !== -1) {
            recoderName = recoderDirInfo[index + 1];
        }
    }
    console.error("-----------------sourceFileName:  ", sourceFileName);
    console.error("-----------------recoderName:  ", recoderName);

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
            let diagnostic = diag.getDiagnostic(diagError.code);
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
