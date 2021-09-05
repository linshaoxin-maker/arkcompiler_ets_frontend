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

// singleton to parse commandLine infos
import commandLineArgs from "command-line-args";
import commandLineUsage from "command-line-usage";
import * as ts from "typescript";
import { LOGE } from "./log";
import path = require("path");
import { execute } from "./base/util";

const ts2pandaOptions = [
    { name: 'variant-bytecode', alias: 'r', type: Boolean, defaultValue: true, description: "emit 2nd bytecode to pandafile."},
    { name: 'modules', alias: 'm', type: Boolean, defaultValue: false, description: "compile as module."},
    { name: 'debug-log', alias: 'l', type: Boolean, defaultValue: false, description: "show info debug log."},
    { name: 'dump-assembly', alias: 'a', type: Boolean, defaultValue: false, description: "dump assembly to file."},
    { name: 'debug', alias: 'd', type: Boolean, defaultValue: false, description: "compile with debug info."},
    { name: 'show-statistics', alias: 's', type: String, lazyMultiple: true, defaultValue: "", description: "show compile statistics(ast, histogram, hoisting, all)."},
    { name: 'output', alias: 'o', type: String, defaultValue: "", description: "set output file."},
    { name: 'timeout', alias: 't', type: Number, defaultValue: 0, description: "js to abc timeout threshold(unit: seconds)."},
    { name: 'opt-log-level', type: String, defaultValue: "error", description: "specifie optimizer log level. Possible values: ['debug', 'info', 'error', 'fatal']"},
    { name: 'opt-level', type: Number, defaultValue: 1, description: "Optimization level. Possible values: [0, 1, 2]. Default: 0\n    0: no optimizations\n    \
                                                                    1: basic bytecode optimizations, including valueNumber, lowering, constantResolver, regAccAllocator\n    \
                                                                    2: other bytecode optimizations, unimplemented yet"},
    { name: 'help', alias: 'h', type: Boolean, description: "Show usage guide."},
    { name: 'bc-version', alias: 'v', type: Boolean, defaultValue: false, description: "Print ark bytecode version"},
    { name: 'bc-min-version', type: Boolean, defaultValue: false, description: "Print ark bytecode minimum supported version"}
]

export class CmdOptions {
    private static parsedResult: ts.ParsedCommandLine;
    private static options: commandLineArgs.CommandLineOptions;

    static isEnableDebugLog(): boolean {
        if (!this.options) {
            return false;
        }
        return this.options["debug-log"];
    }

    static isAssemblyMode(): boolean {
        if (!this.options) {
            return false;
        }
        return this.options["dump-assembly"];
    }

    static isDebugMode(): boolean {
        if (!this.options) {
            return false;
        }
        return this.options["debug"];
    }

    static isModules(): boolean {
        if (!this.options) {
            return false;
        }
        return this.options["modules"];
    }

    static isVariantBytecode(): boolean {
        if (!this.options) {
            return true;
        }
        return this.options["variant-bytecode"];
    }

    static getOptLevel(): number {
        return this.options["opt-level"];
    }

    static getOptLogLevel(): string {
        return this.options["opt-log-level"];
    }

    static showASTStatistics(): boolean {
        if (!this.options) {
            return false;
        }
        return this.options["show-statistics"].includes("ast") || this.options["show-statistics"].includes("all");
    }

    static showHistogramStatistics(): boolean {
        if (!this.options) {
            return false;
        }
        return this.options["show-statistics"].includes("all") || this.options["show-statistics"].includes("histogram");
    }

    static showHoistingStatistics(): boolean {
        if (!this.options) {
            return false;
        }
        return this.options["show-statistics"].includes("all") || this.options["show-statistics"].includes("hoisting");
    }

    static getInputFileName(): string {
        let path = this.parsedResult.fileNames[0];
        let inputFile = path.substring(0, path.lastIndexOf('.'));
        return inputFile;
    }

    static getOutputBinName(): string {
        let outputFile = this.options.output;
        if (outputFile == "") {
            outputFile = CmdOptions.getInputFileName() + ".abc";
        }
        return outputFile;
    }

    static getTimeOut(): Number {
        if (!this.options) {
            return 0;
        }
        return this.options["timeout"];
    }

    static showHelp(): void {
        const usage = commandLineUsage([
            {
                header: "Ark JavaScript Compiler",
                content: 'node --expose-gc index.js [options] file.js'
            },
            {
                header: 'Options',
                optionList: ts2pandaOptions
            },
            {
                content: 'Project Ark'
            }
        ])
        LOGE(usage);
    }

    static isBcVersion(): boolean {
        if (!this.options) {
            return false;
        }
        return this.options["bc-version"];
    }

    static getVersion(isBcVersion : boolean = true) : void {
        let js2abc = path.join(path.resolve(__dirname, '../bin'), "js2abc");
        let version_arg = isBcVersion ? "--bc-version" : "--bc-min-version"
        execute(`${js2abc}`, [version_arg]);
    }

    static isBcMinVersion(): boolean {
        if (!this.options) {
            return false;
        }
        return this.options["bc-min-version"];
    }

    static parseUserCmd(args: string[]): ts.ParsedCommandLine | undefined {
        this.options = commandLineArgs(ts2pandaOptions, { partial: true });
        if (this.options.help) {
            this.showHelp();
            return undefined;
        }

        if (this.isBcVersion() || this.isBcMinVersion()) {
            this.getVersion(this.isBcVersion());
            return undefined;
        }

        if (!this.options._unknown) {
            LOGE("options at least one file is needed");
            this.showHelp();
            return undefined;
        }

        this.parsedResult = ts.parseCommandLine(this.options._unknown!);
        return this.parsedResult;
    }
}
