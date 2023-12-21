/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import type * as ts from 'typescript';
import type { ProblemInfo } from './ProblemInfo';
import { TypeScriptLinter, consoleLog } from './TypeScriptLinter';
import { FaultID, faultsAttrs } from './Problems';
import { parseCommandLine } from './CommandLineParser';
import { LinterConfig } from './TypeScriptLinterConfig';
import type { LintRunResult } from './LintRunResult';
import logger from '../utils/logger';
import * as fs from 'node:fs';
import * as os from 'node:os';
import * as readline from 'node:readline';
import * as path from 'node:path';
import { compile } from './CompilerWrapper';
import type { CommandLineOptions } from './CommandLineOptions';
import type { LintOptions } from './LintOptions';
import { AutofixInfoSet } from './Autofixer';
import { TSCCompiledProgram, getStrictOptions, transformDiagnostic } from './ts-diagnostics/TSCCompiledProgram';
import { mergeArrayMaps, pathContainsDirectory, TsUtils } from './Utils';
import { ProblemSeverity } from './ProblemSeverity';

const loggerInstance = logger.getLogger();

// Use static init method for Linter configuration, as TypeScript 4.2 doesn't support static blocks.
LinterConfig.initStatic();

function toFixedPercent(part: number): string {
  const percentiles = 100;
  const precision = 2;
  return (part * percentiles).toFixed(precision);
}

function prepareInputFilesList(cmdOptions: CommandLineOptions): string[] {
  let inputFiles = cmdOptions.inputFiles;
  if (cmdOptions.parsedConfigFile) {
    inputFiles = cmdOptions.parsedConfigFile.fileNames;
    if (cmdOptions.inputFiles.length > 0) {

      /*
       * Apply linter only to the project source files that are specified
       * as a command-line arguments. Other source files will be discarded.
       */
      const cmdInputsResolvedPaths = cmdOptions.inputFiles.map((x) => {
        return path.resolve(x);
      });
      const configInputsResolvedPaths = inputFiles.map((x) => {
        return path.resolve(x);
      });
      inputFiles = configInputsResolvedPaths.filter((x) => {
        return cmdInputsResolvedPaths.some((y) => {
          return x === y;
        });
      });
    }
  }

  return inputFiles;
}

function countProblems(linter: TypeScriptLinter): [number, number] {
  let errorNodesTotal = 0;
  let warningNodes = 0;
  for (let i = 0; i < FaultID.LAST_ID; i++) {
    // if Strict mode - count all cases
    if (!linter.strictMode && faultsAttrs[i].migratable) {
      // In relax mode skip migratable
      continue;
    }
    switch (faultsAttrs[i].severity) {
      case ProblemSeverity.ERROR:
        errorNodesTotal += linter.nodeCounters[i];
        break;
      case ProblemSeverity.WARNING:
        warningNodes += linter.nodeCounters[i];
        break;
    }
  }

  return [errorNodesTotal, warningNodes];
}

export function lint(options: LintOptions): LintRunResult {
  const cmdOptions = options.cmdOptions;

  const tscDiagnosticsLinter = createLinter(options);
  const tsProgram = tscDiagnosticsLinter.getOriginalProgram();

  // Prepare list of input files for linter and retrieve AST for those files.
  let inputFiles = prepareInputFilesList(cmdOptions);
  inputFiles = inputFiles.filter((input) => {
    return shouldProcessFile(options, input);
  });

  const srcFiles: ts.SourceFile[] = [];
  for (const inputFile of inputFiles) {
    const srcFile = tsProgram.getSourceFile(inputFile);
    if (srcFile) {
      srcFiles.push(srcFile);
    }
  }

  const tscStrictDiagnostics = getTscDiagnostics(tscDiagnosticsLinter, srcFiles);

  const linter = new TypeScriptLinter(
    tsProgram.getTypeChecker(),
    new AutofixInfoSet(cmdOptions.autofixInfo),
    !!cmdOptions.strictMode,
    cmdOptions.warningsAsErrors,
    tscStrictDiagnostics
  );
  const { errorNodes, problemsInfos } = lintFiles(srcFiles, linter);

  consoleLog('\n\n\nFiles scanned: ', srcFiles.length);
  consoleLog('\nFiles with problems: ', errorNodes);

  const [errorNodesTotal, warningNodes] = countProblems(linter);

  logTotalProblemsInfo(errorNodesTotal, warningNodes, linter);
  logProblemsPercentageByFeatures(linter);

  return {
    errorNodes: errorNodesTotal,
    problemsInfos: mergeArrayMaps(problemsInfos, transformTscDiagnostics(tscStrictDiagnostics))
  };
}

/*
 * We want linter to accept program with no strict options set at all
 * due to them affecting type deduction.
 */
function clearStrictOptions(options: LintOptions): LintOptions {
  if (!options.parserOptions) {
    return options;
  }
  const newOptions = { ...options };
  newOptions.parserOptions.strict = false;
  newOptions.parserOptions.alwaysStrict = false;
  newOptions.parserOptions.noImplicitAny = false;
  newOptions.parserOptions.noImplicitThis = false;
  newOptions.parserOptions.strictBindCallApply = false;
  newOptions.parserOptions.strictFunctionTypes = false;
  newOptions.parserOptions.strictNullChecks = false;
  newOptions.parserOptions.strictPropertyInitialization = false;
  newOptions.parserOptions.useUnknownInCatchVariables = false;
  return newOptions;
}

export function createLinter(options: LintOptions): TSCCompiledProgram {
  if (options.tscDiagnosticsLinter) {
    return options.tscDiagnosticsLinter;
  }

  /*
   * if we are provided with the pre-compiled program, don't tamper with options
   * otherwise, clear all strict related options to avoid differences in type deduction
   */
  const newOptions = options.tsProgram ? options : clearStrictOptions(options);
  const tsProgram = newOptions.tsProgram ?? compile(newOptions, getStrictOptions());
  return new TSCCompiledProgram(tsProgram, newOptions);
}

function lintFiles(srcFiles: ts.SourceFile[], linter: TypeScriptLinter): LintRunResult {
  let problemFiles = 0;
  const problemsInfos: Map<string, ProblemInfo[]> = new Map();

  for (const srcFile of srcFiles) {
    const prevVisitedNodes = linter.totalVisitedNodes;
    const prevErrorLines = linter.totalErrorLines;
    const prevWarningLines = linter.totalWarningLines;
    linter.errorLineNumbersString = '';
    linter.warningLineNumbersString = '';
    const nodeCounters: number[] = [];

    for (let i = 0; i < FaultID.LAST_ID; i++) {
      nodeCounters[i] = linter.nodeCounters[i];
    }

    linter.lint(srcFile);
    // save results and clear problems array
    problemsInfos.set(path.normalize(srcFile.fileName), [...linter.problemsInfos]);
    linter.problemsInfos.length = 0;

    // print results for current file
    const fileVisitedNodes = linter.totalVisitedNodes - prevVisitedNodes;
    const fileErrorLines = linter.totalErrorLines - prevErrorLines;
    const fileWarningLines = linter.totalWarningLines - prevWarningLines;

    problemFiles = countProblemFiles(
      nodeCounters,
      problemFiles,
      srcFile,
      fileVisitedNodes,
      fileErrorLines,
      fileWarningLines,
      linter
    );
  }

  return {
    errorNodes: problemFiles,
    problemsInfos: problemsInfos
  };
}

/**
 * Extracts TSC diagnostics emitted by strict checks.
 * Function might be time-consuming, as it runs second compilation.
 * @param sourceFiles AST of the processed files
 * @param tscDiagnosticsLinter linter initialized with the processed program
 * @returns problems found by TSC, mapped by `ts.SourceFile.fileName` field
 */
function getTscDiagnostics(
  tscDiagnosticsLinter: TSCCompiledProgram,
  sourceFiles: ts.SourceFile[]
): Map<string, ts.Diagnostic[]> {
  const strictDiagnostics = new Map<string, ts.Diagnostic[]>();
  sourceFiles.forEach((file) => {
    const diagnostics = tscDiagnosticsLinter.getStrictDiagnostics(file.fileName);
    if (diagnostics.length !== 0) {
      strictDiagnostics.set(path.normalize(file.fileName), diagnostics);
    }
  });
  return strictDiagnostics;
}

function transformTscDiagnostics(strictDiagnostics: Map<string, ts.Diagnostic[]>): Map<string, ProblemInfo[]> {
  const problemsInfos = new Map<string, ProblemInfo[]>();
  strictDiagnostics.forEach((diagnostics, file) => {
    problemsInfos.set(
      file,
      diagnostics.map((x) => {
        return transformDiagnostic(x);
      })
    );
  });
  return problemsInfos;
}

function countProblemFiles(
  nodeCounters: number[],
  filesNumber: number,
  tsSrcFile: ts.SourceFile,
  fileNodes: number,
  fileErrorLines: number,
  fileWarningLines: number,
  linter: TypeScriptLinter
): number {
  let errorNodes = 0;
  let warningNodes = 0;
  for (let i = 0; i < FaultID.LAST_ID; i++) {
    const nodeCounterDiff = linter.nodeCounters[i] - nodeCounters[i];
    switch (faultsAttrs[i].severity) {
      case ProblemSeverity.ERROR:
        errorNodes += nodeCounterDiff;
        break;
      case ProblemSeverity.WARNING:
        warningNodes += nodeCounterDiff;
        break;
    }
  }
  if (errorNodes > 0) {
    filesNumber++;
    const errorRate = toFixedPercent(errorNodes / fileNodes);
    const warningRate = toFixedPercent(warningNodes / fileNodes);
    consoleLog(tsSrcFile.fileName, ': ', '\n\tError lines: ', linter.errorLineNumbersString);
    consoleLog(tsSrcFile.fileName, ': ', '\n\tWarning lines: ', linter.warningLineNumbersString);
    consoleLog(`\n\tError constructs (%): ${errorRate}\t[ of ${fileNodes} constructs ], \t${fileErrorLines} lines`);
    consoleLog(
      `\n\tWarning constructs (%): ${warningRate}\t[ of ${fileNodes} constructs ], \t${fileWarningLines} lines`
    );
  }
  return filesNumber;
}

function logTotalProblemsInfo(errorNodes: number, warningNodes: number, linter: TypeScriptLinter): void {
  const errorRate = toFixedPercent(errorNodes / linter.totalVisitedNodes);
  const warningRate = toFixedPercent(warningNodes / linter.totalVisitedNodes);
  consoleLog('\nTotal error constructs (%): ', errorRate);
  consoleLog('\nTotal warning constructs (%): ', warningRate);
  consoleLog('\nTotal error lines:', linter.totalErrorLines, ' lines\n');
  consoleLog('\nTotal warning lines:', linter.totalWarningLines, ' lines\n');
}

function logProblemsPercentageByFeatures(linter: TypeScriptLinter): void {
  consoleLog('\nPercent by features: ');
  const paddingPercentage = 7;
  for (let i = 0; i < FaultID.LAST_ID; i++) {
    // if Strict mode - count all cases
    if (!linter.strictMode && faultsAttrs[i].migratable) {
      continue;
    }

    const nodes = linter.nodeCounters[i];
    const lines = linter.lineCounters[i];
    const pecentage = toFixedPercent(nodes / linter.totalVisitedNodes).padEnd(paddingPercentage, ' ');

    consoleLog(LinterConfig.nodeDesc[i].padEnd(55, ' '), pecentage, '[', nodes, ' constructs / ', lines, ' lines]');
  }
}

export function run(): void {
  const commandLineArgs = process.argv.slice(2);
  if (commandLineArgs.length === 0) {
    loggerInstance.info('Command line error: no arguments');
    process.exit(-1);
  }

  const cmdOptions = parseCommandLine(commandLineArgs);

  if (cmdOptions.testMode) {
    TypeScriptLinter.testMode = true;
  }

  TypeScriptLinter.initGlobals();

  if (!cmdOptions.ideMode) {
    const result = lint({ cmdOptions: cmdOptions });
    process.exit(result.errorNodes > 0 ? 1 : 0);
  } else {
    runIDEMode(cmdOptions);
  }
}

function getTempFileName(): string {
  const bigNumber = 10000000;
  return path.join(os.tmpdir(), Math.floor(Math.random() * bigNumber).toString() + '_linter_tmp_file.ts');
}

function runIDEMode(cmdOptions: CommandLineOptions): void {
  TypeScriptLinter.ideMode = true;
  const tmpFileName = getTempFileName();
  // read data from stdin
  const writeStream = fs.createWriteStream(tmpFileName, { flags: 'w' });
  const rl = readline.createInterface({
    input: process.stdin,
    output: writeStream,
    terminal: false
  });

  rl.on('line', (line: string) => {
    fs.appendFileSync(tmpFileName, line + '\n');
  });
  rl.once('close', () => {
    // end of input
    writeStream.close();
    cmdOptions.inputFiles = [tmpFileName];
    if (cmdOptions.parsedConfigFile) {
      cmdOptions.parsedConfigFile.fileNames.push(tmpFileName);
    }
    const result = lint({ cmdOptions: cmdOptions });
    const problems = Array.from(result.problemsInfos.values());
    if (problems.length === 1) {
      const jsonMessage = problems[0].map((x) => {
        return {
          line: x.line,
          column: x.column,
          start: x.start,
          end: x.end,
          type: x.type,
          suggest: x.suggest,
          rule: x.rule,
          severity: x.severity,
          autofixable: x.autofixable,
          autofix: x.autofix
        };
      });
      loggerInstance.info(`{"linter messages":${JSON.stringify(jsonMessage)}}`);
    } else {
      loggerInstance.error('Unexpected error: could not lint file');
    }
    fs.unlinkSync(tmpFileName);
  });
}

function shouldProcessFile(options: LintOptions, fileFsPath: string): boolean {
  if (
    TsUtils.ARKTS_IGNORE_FILES.some((ignore) => {
      return path.basename(fileFsPath) === ignore;
    })
  ) {
    return false;
  }

  if (
    TsUtils.ARKTS_IGNORE_DIRS_NO_OH_MODULES.some((ignore) => {
      return pathContainsDirectory(path.resolve(fileFsPath), ignore);
    })
  ) {
    return false;
  }

  return (
    !pathContainsDirectory(path.resolve(fileFsPath), TsUtils.ARKTS_IGNORE_DIRS_OH_MODULES) ||
    options.isFileFromModuleCb !== undefined && options.isFileFromModuleCb(fileFsPath)
  );
}
