/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

import { Command, ParseOptionsResult } from "commander";
import * as path from "path";
import * as ts from "typescript";
import { LOGE } from "./log";
import { execute } from "./base/util";

export class CmdOptions {
    private static cmd: Command = new Command();
    private static parsedResult: ParseOptionsResult;
    private static unknownOpts: ts.ParsedCommandLine;

    static initOptions(): void {
        this.cmd
            .option('-m, --module', 'compile as module.', false)
            .option('-l, --debug-log', 'show info debug log and generate the json file.', false)
            .option('-a, --dump-assembly', 'dump assembly to file.', false)
            .option('-d, --debug', 'compile with debug info.', false)
            .option('-w, --debug-add-watch <args>', 'watch expression and abc file path in debug mode.', [])
            .option('-k, --keep-persistent-watch <watchArgs>',
                    'keep persistent watch on js file with watched expression.', [])
            .option('-s, --show-statistics', 'show compile statistics(ast, histogram, hoisting, all).', '')
            .option('-o, --output', 'set output file.', '')
            .option('-t, --timeout <time>', 'js to abc timeout threshold(unit: seconds).', 0)
            .option('--opt-log-level', 'specifie optimizer log level. Possible values: [debug, info, error, fatal]', 'error')
            .option('--opt-level', 'Optimization level. Possible values: [0, 1, 2]. Default: 0\n    0: no optimizations\n    \
                1: basic bytecode optimizations, including valueNumber, lowering, constantResolver, regAccAllocator\n    \
                2: other bytecode optimizations, unimplemented yet', '1')
            .option('-h, --help', 'Show usage guide.')
            .option('-v, --bc-version', 'Print ark bytecode version', false)
            .option('--bc-min-version', 'Print ark bytecode minimum supported version', false)
            .option('-i, --included-files', 'The list of dependent files.', false)
            .option('-p, --record-type', 'Record type info. Default: true', false)
            .option('-q, --dts-type-record', 'Record type info for .d.ts files. Default: false', false)
            .option('-g, --debug-type', 'Print type-related log. Default: false', false)
            .option('--output-type', 'set output type.', false)
    }

    static parseUserCmd(args: string[]) {
        this.initOptions();
        this.parsedResult = this.cmd.parseOptions(process.argv);
        if (this.cmd.opts().help) {
            this.showHelp();
            return undefined;
        }

        if (this.isBcVersion() || this.isBcMinVersion()) {
            this.getVersion(this.isBcVersion())
            return undefined
        }

        if (!this.parsedResult.unknown) {
            LOGE("options at least one file is needed");
            this.showHelp();
            return undefined;
        }

        this.unknownOpts = ts.parseCommandLine(this.parsedResult.unknown);

        return this.unknownOpts;
    }

    static showHelp(): void {
        this.cmd.outputHelp()
    }

    static isBcVersion(): boolean {
        return this.cmd.opts().bcVersion;
    }

    static isBcMinVersion(): boolean {
        return this.cmd.opts().bcMinVersion;
    }

    static getVersion(isBcVersion: boolean = true): void {
        let js2abc = path.join(path.resolve(__dirname, '../bin'), "js2abc");
        let version_arg = isBcVersion ? "--bc-version" : "--bc-min-version"
        execute(`${js2abc}`, [version_arg]);
    }

    static getKeepWatchFile(): string[] {
        return this.cmd.opts().keepPersistentWatch;
    }

    static isKeepWatchMode(args: string[]): boolean {
        return args.length == 2 && args[0] == "start";
    }

    static isStopWatchMode(args: string[]): boolean {
        return args.length == 2 && args[0] == "stop";
    }

    static isWatchMode(): boolean {
        return this.cmd.opts().debugAddWatch.length != 0;
    }

    static getAddWatchArgs(): string[] {
        return this.cmd.opts().debugAddWatch;
    }

    static setWatchArgs(watchArgs: string[]) {
        this.cmd.opts().debugAddWatch = watchArgs;
    }

    static getIncludedFiles(): string[] {
        return this.cmd.opts().includedFiles;
    }

    static getInputFileName(): string {
        let path = this.unknownOpts.fileNames[0];
        let inputFile = path.substring(0, path.lastIndexOf('.'));
        return inputFile;
    }

    static getOutputBinName(): string {
        let outputFile = this.cmd.opts().output;
        if (outputFile == "") {
            outputFile = CmdOptions.getInputFileName() + ".abc";
        }
        return outputFile;
    }

    static needRecordType(): boolean {
        return this.cmd.opts().recordType;
    }

    static needRecordDtsType(): boolean {
        return this.cmd.opts().dtsTypeRecord;
    }

    static isAssemblyMode(): boolean {
        return this.cmd.opts().dumpAssembly;
    }

    static isEnableDebugLog(): boolean {
        return this.cmd.opts().debugLog;
    }

    static isDebugMode(): boolean {
        return this.cmd.opts().debug;
    }

    static isModules(): boolean {
        return this.cmd.opts().modules;
    }

    static getOptLevel(): number {
        return this.cmd.opts().optLevel;
    }

    static getOptLogLevel(): string {
        return this.cmd.opts().optLogLevel;
    }

    static showASTStatistics(): boolean {
        return this.cmd.opts().showStatistics.include("ast") || this.cmd.opts().showStatistics.include("all");
    }

    static showHistogramStatistics(): boolean {
        return this.cmd.opts().showStatistics.include("all") || this.cmd.opts().showStatistics.include("histogram");
    }

    static showHoistingStatistics(): boolean {
        return this.cmd.opts().showStatistics.include("all") || this.cmd.opts().showStatistics.include("hoisting");
    }

    static getTimeOut(): Number {
        return this.cmd.opts().timeout;
    }

    static isOutputType(): false {
        return this.cmd.opts().outputType;
    }

    static enableTypeLog(): boolean {
        return this.cmd.opts().debugType;
    }
}
